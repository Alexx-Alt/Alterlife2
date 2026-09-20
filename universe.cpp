#include "universe.h"
#include <QtMath>
#include <algorithm>
#include <QMutexLocker>
#include <QJsonArray>

namespace {
constexpr double Tau = M_PI * 2.0;
static double gaussianish(QRandomGenerator &rng) {
    // Sum-of-uniforms approximation: cheap, deterministic and adequate here.
    double x = 0.0;
    for (int i = 0; i < 6; ++i) x += rng.generateDouble();
    return (x - 3.0) / 1.22474487139;
}
static double clamp01(double x) { return std::clamp(x, 0.0, 1.0); }
static const char *entityTypeName(EntityType t) {
    switch(t){
    case EntityType::Quant:return "quant"; case EntityType::Particle:return "particle";
    case EntityType::Cell:return "cell"; case EntityType::Colony:return "colony"; case EntityType::Mind:return "mind";
    case EntityType::Star:return "star"; case EntityType::Planet:return "planet"; case EntityType::Moon:return "moon";
    case EntityType::Ship:return "ship"; case EntityType::GasCloud:return "gasCloud"; case EntityType::Galaxy:return "galaxy";
    case EntityType::Supercluster:return "supercluster"; } return "unknown";
}
}

Universe::Universe(QObject *parent) : QObject(parent), m_physics(m_scenario)
{
    m_rng.seed(m_scenario.seed);
    reset();
}

void Universe::setScenario(const Scenario &s)
{
    m_scenario = s;
    m_scenario.quantCount = std::max(1, m_scenario.quantCount);
    m_scenario.maxEntities = std::clamp(m_scenario.maxEntities, 200, 100000);
    m_scenario.worldRadius = std::max(120.0, m_scenario.worldRadius);
    m_scenario.planetsPerStar = std::clamp(m_scenario.planetsPerStar, 0, 64);
    m_scenario.maxMoonsPerPlanet = std::clamp(m_scenario.maxMoonsPerPlanet, 0, 16);
    m_scenario.minMoonOrbitSpacing = std::max(0.5, m_scenario.minMoonOrbitSpacing);
    m_scenario.galaxySeedCount = std::clamp(m_scenario.galaxySeedCount, 1, 64);
    m_scenario.gravityConstant = std::max(1e-5, m_scenario.gravityConstant);
    m_scenario.gravitationalSoftening = std::max(1e-4, m_scenario.gravitationalSoftening);
    m_scenario.interactionRadius = std::max(1.0, m_scenario.interactionRadius);
    m_scenario.minOrbitSpacing = std::max(1.0, m_scenario.minOrbitSpacing);
    m_scenario.minPlanetMass = std::max(0.001, m_scenario.minPlanetMass);
    m_scenario.maxPlanetMass = std::max(m_scenario.minPlanetMass, m_scenario.maxPlanetMass);
    m_scenario.stellarMassMin = std::max(0.01, m_scenario.stellarMassMin);
    m_scenario.stellarMassMax = std::max(m_scenario.stellarMassMin, m_scenario.stellarMassMax);
    m_scenario.habitableZoneInner = std::max(0.01, m_scenario.habitableZoneInner);
    m_scenario.habitableZoneOuter = std::max(m_scenario.habitableZoneInner + 0.01, m_scenario.habitableZoneOuter);
    m_scenario.replicationCost = std::clamp(m_scenario.replicationCost, 0.01, 0.99);
    m_scenario.friction = std::clamp(m_scenario.friction, 0.0, 1.0);
    m_physics.setScenario(m_scenario);
    m_rng.seed(m_scenario.seed);
    m_maxEntities = m_scenario.maxEntities;
    m_worldRadius = m_scenario.worldRadius;
    reset();
}

void Universe::recordEvent(const QString &msg)
{
    const QString line = QString("[%1] %2").arg(m_time, 0, 'f', 3).arg(msg);
    m_events.append(line);
    if (m_events.size() > 12000) m_events.removeFirst();
    emit logMessage(msg);
}

void Universe::reset()
{
    m_entities.clear();
    m_entities.reserve(m_maxEntities + 64);
    m_galaxySeeds.clear();
    m_events.clear();
    m_time = 0.0;
    m_nextId = 1;
    m_epoch = 0;
    m_lastSampleTime = 0.0;
    m_lastSampleTotal = 0;
    m_lastStarFormation = -100.0;
    m_lastStructureUpdate = -100.0;
    m_metrics.clear();
    spawnInitialQuants();
    rebuildIndex();
    computeMetrics();
    recordEvent(QString("Инициализирована вселенная: seed=%1, %2 первичных элементов")
                .arg(m_scenario.seed).arg(m_entities.size()));
}

void Universe::spawnInitialQuants()
{
    const int clusters = std::max(1, m_scenario.galaxySeedCount);
    const double clusterRing = m_worldRadius * 0.48;
    for (int g = 0; g < clusters; ++g) {
        const double a = Tau * (g + 0.17 * m_rng.generateDouble()) / clusters;
        const double rr = clusterRing * (0.55 + 0.42 * m_rng.generateDouble());
        m_galaxySeeds.append(QPointF(std::cos(a)*rr, std::sin(a)*rr));
    }

    const int N = std::min(m_scenario.quantCount, m_maxEntities - 8);
    for (int n = 0; n < N; ++n) {
        const int g = n % clusters;
        const QPointF c = m_galaxySeeds[g];
        const double spread = m_worldRadius * (0.10 + 0.10 * m_scenario.densityFluctuation);
        const QPointF offset(gaussianish(m_rng)*spread, gaussianish(m_rng)*spread*0.72);

        Entity e;
        e.id = m_nextId++;
        e.type = EntityType::Quant;
        e.pos = c + offset;
        e.galaxyId = g;
        e.superclusterId = 0;
        e.mass = 0.8 + m_rng.generateDouble()*0.4;
        e.energy = 18.0 + m_rng.generateDouble()*8.0;

        const QPointF rel = e.pos - c;
        const double r = std::hypot(rel.x(), rel.y()) + 1.0;
        const QPointF tangent(-rel.y()/r, rel.x()/r);
        const double v = (0.20 + 0.25*m_rng.generateDouble()) * m_scenario.initialTemperature;
        e.vel = tangent*v + QPointF(gaussianish(m_rng), gaussianish(m_rng))*0.08*m_scenario.initialTemperature;
        m_entities.append(e);
    }
}

void Universe::rebuildIndex()
{
    m_idIndex.clear(); m_idIndex.reserve(m_entities.size()*2);
    for(int i=0;i<m_entities.size();++i) m_idIndex.insert(m_entities[i].id,i);
}

Entity *Universe::findEntity(int id)
{
    auto it=m_idIndex.constFind(id); if(it==m_idIndex.constEnd()) return nullptr;
    const int i=it.value(); return (i>=0&&i<m_entities.size()&&m_entities[i].id==id)?&m_entities[i]:nullptr;
}

const Entity *Universe::findEntity(int id) const
{
    auto it=m_idIndex.constFind(id); if(it==m_idIndex.constEnd()) return nullptr;
    const int i=it.value(); return (i>=0&&i<m_entities.size()&&m_entities[i].id==id)?&m_entities[i]:nullptr;
}

double Universe::nearestStarDistance(const QPointF &p, int exceptId) const
{
    double best = 1e9;
    for (const auto &e : m_entities) {
        if (e.type != EntityType::Star || e.id == exceptId) continue;
        best = std::min(best, std::hypot(e.pos.x()-p.x(), e.pos.y()-p.y()));
    }
    return best;
}

double Universe::maxOrbitRadiusForStar(int starId) const
{
    double radius = 0.0;
    for (const auto &e : m_entities) {
        if (e.type != EntityType::Planet || e.parentId != starId) continue;
        double satelliteEnvelope = std::max(2.0, e.radius);
        for (const auto &moon : m_entities)
            if (moon.type == EntityType::Moon && moon.parentId == e.id)
                satelliteEnvelope = std::max(satelliteEnvelope, moon.orbitRadius + moon.radius);
        radius = std::max(radius, e.orbitRadius + satelliteEnvelope);
    }
    return radius;
}

bool Universe::starSystemClearance(const QPointF &candidate, double candidateRadius, int exceptId) const
{
    for (const auto &star : m_entities) {
        if (star.type != EntityType::Star || star.id == exceptId) continue;
        const double d = std::hypot(star.pos.x()-candidate.x(), star.pos.y()-candidate.y());
        const double occupied = std::max(star.radius * 2.0, maxOrbitRadiusForStar(star.id));
        // A safety moat keeps stellar photospheres and all generated orbital lanes disjoint.
        if (d <= occupied + candidateRadius + std::max(12.0, m_scenario.minOrbitSpacing*0.55)) return false;
    }
    return true;
}

double Universe::computeEntropy() const
{
    constexpr int B = 16;
    int hist[B][B] = {{0}};
    int total = 0;
    for (const auto &e : m_entities) {
        if (!e.alive || e.type == EntityType::Galaxy || e.type == EntityType::Supercluster) continue;
        const int bx = std::clamp(int((e.pos.x()/(2*m_worldRadius)+0.5)*B), 0, B-1);
        const int by = std::clamp(int((e.pos.y()/(2*m_worldRadius)+0.5)*B), 0, B-1);
        hist[by][bx]++; total++;
    }
    if (!total) return 0.0;
    double H = 0.0;
    for (auto &row : hist) for (int c : row) if (c) {
        const double p = double(c)/total;
        H -= p*std::log2(p);
    }
    return H/std::log2(double(B*B));
}

void Universe::updatePlanetCapacities()
{
    for (auto &e : m_entities) if (e.type == EntityType::Planet) {
        e.population = 0;
        e.carryingCapacity = std::max(1.0, e.habitability * m_scenario.carryingCapacityPerHabitability
                                     * (0.5 + e.resourceRichness));
    }
    for (const auto &e : m_entities) {
        if (e.type >= EntityType::Cell && e.type <= EntityType::Mind && e.parentId >= 0) {
            if (Entity *p = findEntity(e.parentId); p && p->type == EntityType::Planet) p->population++;
        }
    }
}

void Universe::tryFormStars(double dt)
{
    if (m_time - m_lastStarFormation < 1.25) return;
    m_lastStarFormation = m_time;
    if (m_entities.size() >= m_maxEntities) return;

    const double R = std::max(8.0, m_scenario.interactionRadius);
    const double R2 = R*R;
    const int total=m_entities.size();
    const int startIndex=total?int(std::fmod(m_time*37.0,total)):0;
    for (int attempt=0; attempt<std::min(32,total); ++attempt) {
        const int idx=(startIndex+attempt*97)%std::max(1,total);
        Entity &e = m_entities[idx];
        if (e.type != EntityType::Particle || !e.alive) continue;

        int neighbors = 0;
        double localMass = e.mass;
        QPointF weighted = e.pos * e.mass;
        double velocityDispersion = 0.0;
        QVector<int> members;
        members.reserve(64);
        for (int j = 0; j < m_entities.size(); ++j) {
            const Entity &o = m_entities[j];
            if (j == idx || o.type != EntityType::Particle || !o.alive || o.galaxyId != e.galaxyId) continue;
            const QPointF d = o.pos - e.pos;
            if (d.x()*d.x()+d.y()*d.y() > R2) continue;
            ++neighbors;
            localMass += o.mass;
            weighted += o.pos * o.mass;
            velocityDispersion += std::hypot(o.vel.x()-e.vel.x(), o.vel.y()-e.vel.y());
            members.append(j);
            if (neighbors >= 96) break;
        }
        if (!neighbors) continue;
        velocityDispersion /= neighbors;

        // Jeans-like collapse score: density and mass favor collapse, thermal
        // dispersion opposes it. This is still a reduced 2-D model, but its
        // dependencies have the correct physical direction.
        const double density = localMass / (M_PI*R2);
        const double collapse = density * localMass * m_scenario.gasCoolingRate
                                / (0.08 + velocityDispersion*velocityDispersion
                                   + 0.35*m_scenario.initialTemperature);
        const double threshold = 0.006 * std::max(3.0, m_scenario.starFormationThreshold);
        if (collapse < threshold || neighbors < std::max(5, int(m_scenario.starFormationThreshold*0.45))) continue;

        const QPointF center = weighted / std::max(1e-6, localMass);
        const double provisionalSystem = std::max(16.0, std::min(28.0, R*0.55));
        if (nearestStarDistance(center) < std::max(55.0, R*1.7)
            || !starSystemClearance(center, provisionalSystem)) continue;

        e.type = EntityType::Star;
        e.pos = center;
        e.vel = QPointF();
        e.mass = std::clamp(localMass*0.035, m_scenario.stellarMassMin, m_scenario.stellarMassMax);
        e.radius = 5.5*std::pow(e.mass, 0.8);
        e.luminosity = std::pow(e.mass, 3.5);
        e.energy = 100.0;
        int absorbed = 0;
        for (int j : members) {
            if (j >= 0 && j < m_entities.size() && m_entities[j].type == EntityType::Particle) {
                m_entities[j].alive = false;
                absorbed++;
            }
        }
        emit entityEvolved(e.id, int(e.type));
        recordEvent(QString("★ Звезда #%1 сформировалась: M=%2 M☉, L=%3")
                    .arg(e.id).arg(e.mass,0,'f',2).arg(e.luminosity,0,'f',2));
        formPlanetsForStar(e, std::max(absorbed, neighbors));
        return;
    }
    Q_UNUSED(dt);
}

void Universe::formPlanetsForStar(Entity &star, int absorbedMass)
{
    int count = std::min(m_scenario.planetsPerStar,
                         int(std::floor(absorbedMass/std::max(1.0,m_scenario.planetFormationCost))));
    if (count <= 0) return;

    const double nearest = nearestStarDistance(star.pos, star.id);
    const double systemLimit = std::min(125.0, nearest < 1e8 ? nearest*0.30 : 125.0);
    const double minR = 34.0 + star.radius*1.6;
    if (systemLimit < minR + m_scenario.minOrbitSpacing) return;

    double previousR = minR - m_scenario.minOrbitSpacing;
    for (int i = 0; i < count && m_entities.size() < m_maxEntities; ++i) {
        const double target = minR + (systemLimit-minR) * (i+1.0)/(count+0.55);
        double r = std::max(target, previousR + std::max(8.0, m_scenario.minOrbitSpacing));
        if (r >= systemLimit) break;

        Entity p;
        p.id = m_nextId++;
        p.type = EntityType::Planet;
        p.parentId = star.id;
        p.galaxyId = star.galaxyId;
        p.superclusterId = star.superclusterId;
        p.orbitRadius = r;
        p.orbitAngle = m_rng.generateDouble()*Tau;
        p.eccentricity = 0.0; // collision-safe baseline; GUI explains why.
        p.mass = (m_scenario.minPlanetMass + m_rng.generateDouble()
                 * std::max(0.0, m_scenario.maxPlanetMass-m_scenario.minPlanetMass))
                 * std::max(0.05, m_scenario.planetMass);
        p.radius = 2.8*std::pow(std::max(0.08,p.mass), 0.28);
        p.orbitSpeed = m_physics.angularSpeed(star.mass*260.0, r);
        p.pos = star.pos + QPointF(std::cos(p.orbitAngle)*r, std::sin(p.orbitAngle)*r);
        p.albedo = clamp01(m_scenario.meanAlbedo + gaussianish(m_rng)*0.08);
        p.waterFraction = clamp01(m_scenario.oceanFraction + gaussianish(m_rng)*0.22);
        p.atmosphere = clamp01(m_scenario.atmosphereRetention * (0.45 + 0.55*std::pow(p.mass,0.25))
                               + gaussianish(m_rng)*0.08);
        p.magneticMoment = clamp01(m_scenario.magneticStrength * (0.55+0.45*m_rng.generateDouble())
                                   * std::pow(std::max(0.1,p.mass),0.22)
                                   * std::pow(std::max(0.05,m_scenario.rotationSpeed),0.28));
        p.resourceRichness = clamp01(0.32 + 0.58*m_rng.generateDouble());
        const double au = r / 62.0;
        p.temperature = m_physics.equilibriumTemperature(star.luminosity, au, p.albedo,
                                                         m_scenario.greenhouseFactor*p.atmosphere);
        p.radiationDose = clamp01((star.luminosity/std::max(0.1,au*au))
                                  * (1.0-p.magneticMoment*0.72) * 0.16
                                  + m_scenario.radiationNoise);
        const double thermal = std::exp(-std::pow((p.temperature-1.0)/0.27,2.0));
        const double hzScale = std::sqrt(std::max(0.02, star.luminosity));
        const double hzInner = std::max(0.05, m_scenario.habitableZoneInner*hzScale);
        const double hzOuter = std::max(hzInner+0.05, m_scenario.habitableZoneOuter*hzScale);
        const double hzMid = 0.5*(hzInner+hzOuter);
        const double hzHalf = std::max(0.08, 0.5*(hzOuter-hzInner));
        const double hz = std::exp(-0.65*std::pow((au-hzMid)/hzHalf, 4.0));
        const double water = std::exp(-std::pow((p.waterFraction-0.58)/0.38,2.0));
        const double gravity = std::exp(-std::pow((std::pow(p.mass,0.42)-1.0)/0.85,2.0));
        const double shielding = clamp01(1.0 - p.radiationDose);
        const double chemistry = clamp01(m_scenario.chemistryRichness*(0.5+0.5*p.resourceRichness));
        p.habitability = clamp01(thermal*hz*water*gravity*(0.35+0.65*p.atmosphere)
                                 *(0.35+0.65*shielding)*(0.45+0.55*chemistry));
        p.carryingCapacity = p.habitability*m_scenario.carryingCapacityPerHabitability;
        p.energy = p.habitability*100.0;
        m_entities.append(p);
        previousR = r;
        emit entityEvolved(p.id, int(p.type));
        recordEvent(QString("● Планета #%1: a=%2, T=%3, H=%4")
                    .arg(p.id).arg(p.orbitRadius,0,'f',1).arg(p.temperature,0,'f',2)
                    .arg(p.habitability,0,'f',2));
        formMoonsForPlanet(star, p);
    }
}

void Universe::formMoonsForPlanet(const Entity &star, const Entity &planet)
{
    if (m_scenario.maxMoonsPerPlanet <= 0 || m_entities.size() >= m_maxEntities) return;
    // Effective masses are expressed in the simulator's normalized unit system.
    // The stability fraction remains conservative while keeping moons resolvable in 2-D.
    const double hill = m_physics.hillRadius(planet.orbitRadius,
                                             std::max(0.01, planet.mass),
                                             std::max(0.1, star.mass*5.0));
    const double inner = std::max(planet.radius*1.55, 3.2);
    const double outer = std::min(18.0, std::max(inner, hill*0.42));
    if (outer <= inner + m_scenario.minMoonOrbitSpacing) return;

    int wanted = std::min(m_scenario.maxMoonsPerPlanet,
                          std::max(0, int(std::floor((outer-inner)/m_scenario.minMoonOrbitSpacing))));
    // Massive planets are more likely to retain multiple stable satellites.
    wanted = std::min(wanted, std::max(0, int(std::round(std::sqrt(std::max(0.0,planet.mass))))));
    double previous = inner - m_scenario.minMoonOrbitSpacing;
    for (int i=0; i<wanted && m_entities.size()<m_maxEntities; ++i) {
        const double target = inner + (outer-inner)*(i+1.0)/(wanted+1.0);
        const double r = std::max(target, previous + m_scenario.minMoonOrbitSpacing);
        if (r >= outer) break;
        Entity moon;
        moon.id = m_nextId++; moon.type = EntityType::Moon; moon.parentId = planet.id;
        moon.galaxyId = planet.galaxyId; moon.superclusterId = planet.superclusterId;
        moon.mass = std::clamp(planet.mass*(0.004 + 0.025*m_rng.generateDouble()), 0.002, 0.18);
        moon.radius = std::max(1.0, planet.radius*std::pow(moon.mass/std::max(0.01,planet.mass),0.30));
        moon.orbitRadius = r; moon.orbitAngle = m_rng.generateDouble()*Tau;
        moon.orbitSpeed = m_physics.angularSpeed(std::max(1.0,planet.mass*42.0),r);
        moon.pos = planet.pos + QPointF(std::cos(moon.orbitAngle)*r,std::sin(moon.orbitAngle)*r);
        moon.albedo = clamp01(planet.albedo + gaussianish(m_rng)*0.12);
        moon.temperature = planet.temperature*(0.96 + 0.08*m_rng.generateDouble());
        moon.energy = 12.0;
        m_entities.append(moon); previous = r;
        recordEvent(QString("◦ Спутник #%1 сформирован у планеты #%2, r=%3")
                    .arg(moon.id).arg(planet.id).arg(r,0,'f',1));
    }
}

void Universe::trySpawnLife(double dt)
{
    const int n = m_entities.size();
    for (int i = 0; i < n && m_entities.size() < m_maxEntities; ++i) {
        Entity &p = m_entities[i];
        if (p.type != EntityType::Planet || p.habitability < 0.16) continue;
        bool hasLife = false;
        for (const auto &e : m_entities) {
            if (e.parentId == p.id && e.type >= EntityType::Cell && e.type <= EntityType::Mind) { hasLife = true; break; }
        }
        if (hasLife) { p.biosphere = clamp01(p.biosphere + 0.00008*dt*std::max(1,p.population)); continue; }

        const double chemistry = m_scenario.chemistryRichness * (0.35+0.65*p.resourceRichness);
        const double hazard = std::max(0.0, m_scenario.abiogenesisRate * p.habitability*p.habitability
                                       * chemistry * dt);
        if (m_rng.generateDouble() > 1.0-std::exp(-hazard)) continue;

        Entity cell;
        cell.id = m_nextId++;
        cell.type = EntityType::Cell;
        cell.parentId = p.id;
        cell.galaxyId = p.galaxyId;
        cell.superclusterId = p.superclusterId;
        cell.pos = p.pos + QPointF(gaussianish(m_rng)*4.0, gaussianish(m_rng)*4.0);
        cell.energy = 46.0;
        cell.level = 0;
        cell.fitness = p.habitability;
        m_entities.append(cell);
        p.biosphere = 0.02;
        emit entityEvolved(cell.id, int(cell.type));
        recordEvent(QString("✿ Абиогенез на планете #%1").arg(p.id));
    }
}

void Universe::tryBuildShip(Entity &colony)
{
    if (m_entities.size() >= m_maxEntities || colony.energy < m_scenario.shipEnergyCost*1.6) return;
    Entity *best = nullptr;
    double bestScore = -1e9;
    for (auto &p : m_entities) {
        if (p.type != EntityType::Planet || p.id == colony.parentId || p.habitability < m_scenario.colonizationThreshold) continue;
        const double d = std::hypot(p.pos.x()-colony.pos.x(), p.pos.y()-colony.pos.y());
        const double score = p.habitability*2.2 + p.resourceRichness - d/(m_worldRadius*1.5);
        if (score > bestScore) { bestScore = score; best = &p; }
    }
    if (!best) return;
    Entity ship;
    ship.id = m_nextId++;
    ship.type = EntityType::Ship;
    ship.pos = colony.pos;
    ship.parentId = colony.parentId;
    ship.galaxyId = colony.galaxyId;
    ship.energy = m_scenario.shipEnergyCost;
    const QPointF d = best->pos-colony.pos;
    const double len = std::hypot(d.x(),d.y())+1e-9;
    ship.vel = d/len*m_scenario.shipSpeed;
    ship.direction = std::atan2(d.y(),d.x());
    ship.lastAction = ship.direction;
    colony.energy -= m_scenario.shipEnergyCost;
    m_entities.append(ship);
    recordEvent(QString("🚀 Экспедиция #%1 стартовала от колонии #%2 к планете #%3")
                .arg(ship.id).arg(colony.id).arg(best->id));
}

void Universe::replicate(Entity &parent)
{
    if (m_entities.size() >= m_maxEntities) return;
    Entity child = parent;
    child.id = m_nextId++;
    child.generation = parent.generation+1;
    const double childShare = std::clamp(m_scenario.replicationCost,0.05,0.95);
    child.energy = parent.energy*childShare;
    parent.energy *= (1.0-childShare);
    child.age = 0.0; child.childrenCount = 0; child.starvationTime = 0.0;
    for (int a=0;a<3;++a) for (int b=0;b<2;++b)
        child.w[a][b] += gaussianish(m_rng)*m_scenario.mutationRate;
    child.responseLatency = std::max(0.03, parent.responseLatency*(1.0+gaussianish(m_rng)*m_scenario.mutationRate*0.25));
    const double a = m_rng.generateDouble()*Tau;
    child.pos += QPointF(std::cos(a)*3.5,std::sin(a)*3.5);
    m_entities.append(child);
    parent.childrenCount++;
}

void Universe::tryEvolve()
{
    for (auto &e : m_entities) {
        if (!e.alive) continue;
        if (e.type == EntityType::Quant && e.mass > 1.55 && e.age > 2.0) {
            e.type = EntityType::Particle; e.energy += 8.0; emit entityEvolved(e.id,int(e.type));
        } else if (e.type == EntityType::Cell && e.generation >= 2 && e.age > 7.0 && e.fitness > 0.28) {
            e.type = EntityType::Colony; e.level=1; e.responseLatency*=0.68;
            emit entityEvolved(e.id,int(e.type));
            recordEvent(QString("◆ Колониальная организация #%1 на планете #%2").arg(e.id).arg(e.parentId));
        } else if (e.type == EntityType::Colony && e.generation >= 5 && e.age > 20.0 && e.technology > 0.12) {
            e.type = EntityType::Mind; e.level=2; e.responseLatency*=0.55;
            emit entityEvolved(e.id,int(e.type));
            recordEvent(QString("🧠 Устойчивый слой выбора #%1 перешёл к рефлексивному поведению").arg(e.id));
        }
    }
}

void Universe::updateCosmicStructures()
{
    if (m_time - m_lastStructureUpdate < 4.0) return;
    m_lastStructureUpdate = m_time;

    const int groups = std::max(1,m_scenario.galaxySeedCount);
    for (int g=0; g<groups; ++g) {
        QPointF c; double mass=0; int stars=0; Entity *marker=nullptr;
        for (auto &e : m_entities) {
            if (e.type==EntityType::Galaxy && e.galaxyId==g) marker=&e;
            if (e.galaxyId!=g) continue;
            if (e.type==EntityType::Star) { c += e.pos*e.mass; mass+=e.mass; stars++; }
            else if (e.type==EntityType::Particle || e.type==EntityType::Quant || e.type==EntityType::GasCloud) {
                // Diffuse matter contributes to the barycentre with reduced visual/structural weight.
                const double w=std::max(0.01,e.mass)*0.035; c+=e.pos*w; mass+=w;
            }
        }
        if (stars<2) continue;
        c /= std::max(1e-6,mass);
        if (!marker && m_entities.size()<m_maxEntities) {
            Entity gal; gal.id=m_nextId++; gal.type=EntityType::Galaxy; gal.galaxyId=g; gal.superclusterId=0;
            gal.pos=c; gal.mass=mass; gal.radius=70.0+18.0*std::sqrt(stars); gal.energy=std::min(100.0,20.0*stars);
            m_entities.append(gal);
            recordEvent(QString("◎ Галактика G%1 стабилизировалась вокруг %2 звёздных систем").arg(g+1).arg(stars));
        } else if (marker) {
            marker->pos=c; marker->mass=mass; marker->radius=70.0+18.0*std::sqrt(stars);
        }
    }

    QVector<Entity*> galaxies;
    for (auto &e:m_entities) if(e.type==EntityType::Galaxy) galaxies.append(&e);
    if (galaxies.size()>=2) {
        Entity *super=nullptr; QPointF c; double m=0;
        for (auto *g:galaxies){c+=g->pos*std::max(1.0,g->mass);m+=std::max(1.0,g->mass);}
        c/=std::max(1.0,m);
        for(auto &e:m_entities) if(e.type==EntityType::Supercluster){super=&e;break;}
        if(!super && m_entities.size()<m_maxEntities){
            Entity s; s.id=m_nextId++; s.type=EntityType::Supercluster; s.pos=c; s.mass=m; s.radius=m_worldRadius*0.72; s.energy=100;
            m_entities.append(s); recordEvent(QString("◉ Сверхгалактическая структура объединила %1 галактик").arg(galaxies.size()));
        } else if(super){super->pos=c;super->mass=m;}
    }
}

void Universe::updateEpoch()
{
    int particle=0,cell=0,colony=0,mind=0,star=0,galaxy=0;
    for(const auto&e:m_entities){
        switch(e.type){
        case EntityType::Particle: particle++; break; case EntityType::Cell: cell++; break;
        case EntityType::Colony: colony++; break; case EntityType::Mind: mind++; break;
        case EntityType::Star: star++; break; case EntityType::Galaxy: galaxy++; break; default: break;
        }
    }
    int next=0;
    if(mind>0) next=6; else if(colony>0) next=5; else if(cell>0) next=4;
    else if(galaxy>0) next=3; else if(star>0) next=2; else if(particle>0) next=1;
    if(next>m_epoch){m_epoch=next;emit epochChanged(m_epoch);}
}

void Universe::enforceLimit()
{
    if(m_entities.size()<m_maxEntities) return;
    for(auto &e:m_entities){
        if(e.type==EntityType::Quant || e.type==EntityType::Particle){e.alive=false;break;}
    }
}

void Universe::step(double dt)
{
    if (!(dt>0.0)) return;
    dt = std::min(dt, 0.75); // integrator stability; fast-forward uses repeated adaptive steps.
    m_time += dt;

    // Exact circular Kepler propagation keeps generated orbital lanes disjoint.
    for(auto &e:m_entities) if(e.type==EntityType::Planet){
        if(Entity *star=findEntity(e.parentId); star && star->type==EntityType::Star){
            e.orbitAngle = std::fmod(e.orbitAngle + e.orbitSpeed*dt, Tau);
            e.pos = star->pos + QPointF(std::cos(e.orbitAngle)*e.orbitRadius,
                                        std::sin(e.orbitAngle)*e.orbitRadius);
        }
    }
    for(auto &e:m_entities) if(e.type==EntityType::Moon){
        if(Entity *planet=findEntity(e.parentId); planet && planet->type==EntityType::Planet){
            e.orbitAngle = std::fmod(e.orbitAngle + e.orbitSpeed*dt, Tau);
            e.pos = planet->pos + QPointF(std::cos(e.orbitAngle)*e.orbitRadius,
                                          std::sin(e.orbitAngle)*e.orbitRadius);
        }
    }
    updatePlanetCapacities();

    const int initialN=m_entities.size();
    for(int idx=0; idx<initialN && idx<m_entities.size(); ++idx){
        Entity &e=m_entities[idx];
        if(!e.alive || e.type==EntityType::Star || e.type==EntityType::Planet || e.type==EntityType::Moon
           || e.type==EntityType::Galaxy || e.type==EntityType::Supercluster) continue;
        e.age += dt;

        if(e.type==EntityType::Ship){
            Entity *best=nullptr; double bestScore=-1e9;
            for(auto &p:m_entities) if(p.type==EntityType::Planet && p.habitability>=m_scenario.colonizationThreshold){
                const double d=std::hypot(p.pos.x()-e.pos.x(),p.pos.y()-e.pos.y());
                const double score=2.2*p.habitability+p.resourceRichness-d/(m_worldRadius*1.7);
                if(score>bestScore){bestScore=score;best=&p;}
            }
            if(!best){e.alive=false;continue;}
            const QPointF dir=best->pos-e.pos; const double dist=std::hypot(dir.x(),dir.y())+1e-9;
            if(dist<std::max(8.0,best->radius*2.2)){
                e.type=EntityType::Colony; e.parentId=best->id; e.pos=best->pos+QPointF(5,0);
                e.vel=QPointF(); e.energy=54; e.level=1; e.technology=0.18; e.responseLatency=0.5;
                recordEvent(QString("⛵ Экспедиция #%1 основала колонию на планете #%2").arg(e.id).arg(best->id));
                continue;
            }
            const double desired=std::max(0.5,m_scenario.shipSpeed);
            e.vel=dir/dist*desired; e.pos+=e.vel*dt; e.direction=std::atan2(dir.y(),dir.x()); e.lastAction=e.direction;
            e.energy-=0.035*desired*dt;
            if(e.energy<=0)e.alive=false;
            continue;
        }

        // Free matter follows a smooth halo and local stochastic radiation.
        QPointF force=m_physics.gravityForce(e.pos,m_worldRadius)+m_physics.orbitForce(e.pos,m_worldRadius);
        if(e.type==EntityType::Quant || e.type==EntityType::Particle) force += m_physics.radiationNoise();

        Entity *planet=nullptr;
        if(e.type>=EntityType::Cell && e.type<=EntityType::Mind && e.parentId>=0) planet=findEntity(e.parentId);
        if(planet && planet->type==EntityType::Planet){
            QPointF toP=planet->pos-e.pos; double d=std::hypot(toP.x(),toP.y())+1e-9;
            force += toP/d * std::clamp((d-9.0)*0.018,-0.15,0.35);
            if(d>110){e.alive=false;continue;}
            e.sensorThermal=clamp01(1.0-std::abs(planet->temperature-1.0));
            e.sensorMagnetic=planet->magneticMoment;
            e.sensorTidal=clamp01(0.5 + 0.5*m_scenario.tidalAmplitude
                                      * std::sin(m_time*m_scenario.tidalFrequency + e.id*0.17));
        } else {
            e.sensorThermal=m_physics.thermalField(e.pos,m_time,m_worldRadius);
            e.sensorMagnetic=m_physics.magneticField(e.pos,m_worldRadius);
            e.sensorTidal=0.5+0.5*m_physics.tidalField(m_time);
        }

        if(e.type==EntityType::Quant){
            e.vel += QPointF(gaussianish(m_rng),gaussianish(m_rng))*0.05*m_scenario.initialTemperature*dt;
            const QPointF gc=(e.galaxyId>=0&&e.galaxyId<m_galaxySeeds.size())?m_galaxySeeds[e.galaxyId]:QPointF();
            const double gr=std::hypot(e.pos.x()-gc.x(),e.pos.y()-gc.y());
            const double spread=m_worldRadius*(0.10+0.10*m_scenario.densityFluctuation);
            const double localDensity=std::exp(-(gr*gr)/(2.0*spread*spread+1.0));
            const double expectedNeighbors=(double(m_scenario.quantCount)/std::max(1,m_scenario.galaxySeedCount))*localDensity
                                           * std::pow(m_scenario.interactionRadius/std::max(20.0,spread),2.0);
            e.mass += expectedNeighbors*0.0016*m_scenario.gasCoolingRate*dt*10.0;
        } else if(e.type>=EntityType::Cell && e.type<=EntityType::Mind){
            const double in[3]={e.sensorThermal,e.sensorMagnetic,e.sensorTidal}; double out[2]={0,0};
            for(int a=0;a<3;++a)for(int b=0;b<2;++b)out[b]+=in[a]*e.w[a][b];
            const double targetFitness=planet?planet->habitability:0.2;
            const double reward=targetFitness - 0.16*std::abs(e.sensorTidal-0.5) - 0.12*(1.0-e.sensorMagnetic);
            e.fitness = 0.992*e.fitness+0.008*reward;
            if(e.level>=1){
                for(int a=0;a<3;++a)for(int b=0;b<2;++b)
                    e.w[a][b]+=m_scenario.learningRate*reward*in[a]*std::tanh(out[b]);
            }
            if(e.level>=2){out[0]=(1-m_scenario.memoryFactor)*out[0]+m_scenario.memoryFactor*std::cos(e.prevAction);
                            out[1]=(1-m_scenario.memoryFactor)*out[1]+m_scenario.memoryFactor*std::sin(e.prevAction);}
            e.prevAction=e.lastAction; e.lastAction=std::atan2(out[1],out[0]);
            const double accel=(e.type==EntityType::Cell?0.24:e.type==EntityType::Colony?0.34:0.42);
            e.vel += QPointF(std::cos(e.lastAction),std::sin(e.lastAction))*accel*dt;
            e.responseLatency = std::max(0.02, e.responseLatency*(1.0-0.0002*e.level*dt));
            if(e.type==EntityType::Colony || e.type==EntityType::Mind){
                e.technology = clamp01(e.technology + m_scenario.learningRate*(0.08+std::max(0.0,e.fitness))*dt);
                if(planet && planet->population>planet->carryingCapacity*0.82 && e.energy>m_scenario.shipEnergyCost*1.7
                   && m_rng.generateDouble()<0.0015*dt*(1.0+e.technology*4.0)) tryBuildShip(e);
            }
        }

        e.vel += force*dt;
        e.vel *= std::pow(std::clamp(m_scenario.friction,0.0,1.0),dt*60.0);
        e.pos += e.vel*dt;

        double absorb=0;
        if(planet && e.type>=EntityType::Cell && e.type<=EntityType::Mind){
            const double crowd=double(planet->population)/std::max(1.0,planet->carryingCapacity);
            const double resource=std::clamp(1.0-crowd*0.62,-0.25,1.0);
            absorb=m_scenario.baseAbsorptionRate*planet->habitability*(0.45+0.55*planet->resourceRichness)*resource*dt;
        } else if(e.type==EntityType::Quant || e.type==EntityType::Particle) {
            absorb=0.06*e.sensorThermal*dt;
        }
        e.energy += absorb; e.totalAbsorbed += absorb;
        const double metabolism=(e.type>=EntityType::Cell?m_scenario.metabolismCost:0.006)*dt;
        const double motion=std::hypot(e.vel.x(),e.vel.y())*m_scenario.movementCost*dt;
        e.energy -= metabolism+motion; e.totalSpent += metabolism+motion;
        if(e.energy<=0){e.starvationTime+=dt;if(e.starvationTime>8.0)e.alive=false;}else e.starvationTime=0;

        if(e.type>=EntityType::Cell && e.type<=EntityType::Mind && e.energy>m_scenario.replicationEnergyThreshold){
            bool room=!planet || planet->population<planet->carryingCapacity*1.12;
            const double pRepl=std::clamp((e.energy-m_scenario.replicationEnergyThreshold)/100.0,0.0,0.08)
                               *(0.4+0.6*std::max(0.0,e.fitness))*m_scenario.selectionPressure*dt;
            if(room && m_rng.generateDouble()<pRepl) replicate(e);
        }
    }

    m_entities.erase(std::remove_if(m_entities.begin(),m_entities.end(),[](const Entity&e){return !e.alive;}),m_entities.end());
    rebuildIndex();
    tryFormStars(dt);
    trySpawnLife(dt);
    updateCosmicStructures();
    tryEvolve();
    updateEpoch();
    enforceLimit();
    m_entities.erase(std::remove_if(m_entities.begin(),m_entities.end(),[](const Entity&e){return !e.alive;}),m_entities.end());
    rebuildIndex();
    computeMetrics();
}

void Universe::fastForward(double simulatedSeconds, int maxSteps)
{
    simulatedSeconds=std::max(0.0,simulatedSeconds);
    if(simulatedSeconds<=0)return;
    const double adaptive=std::clamp(simulatedSeconds/std::max(1,maxSteps),0.05,0.65);
    const int steps=std::max(1,int(std::ceil(simulatedSeconds/adaptive)));
    for(int i=0;i<steps;++i) step(std::min(adaptive,simulatedSeconds-i*adaptive));
}

void Universe::computeMetrics()
{
    MetricsSample s; s.time=m_time;s.epoch=m_epoch;
    int living=0; double totalMass=0,totalGen=0,totalLevel=0,totalEnergy=0,totalAbs=0,totalSpent=0,totalLatency=0;
    for(const auto&e:m_entities){
        switch(e.type){
        case EntityType::Quant:s.quantCount++;break;case EntityType::Particle:s.particleCount++;break;
        case EntityType::Cell:s.cellCount++;living++;break;case EntityType::Colony:s.colonyCount++;living++;break;
        case EntityType::Mind:s.mindCount++;living++;break;case EntityType::Star:s.starCount++;break;
        case EntityType::Planet:s.planetCount++;if(e.habitability>=0.32)s.habitablePlanetCount++;break;
        case EntityType::Ship:s.shipCount++;break;case EntityType::Galaxy:s.galaxyCount++;break;
        case EntityType::Supercluster:s.superclusterCount++;break;case EntityType::Moon:s.moonCount++;break;default:break;}
        totalMass+=e.mass;totalGen+=e.generation;totalLevel+=e.level;totalEnergy+=std::max(0.0,e.energy);
        totalAbs+=e.totalAbsorbed;totalSpent+=e.totalSpent;if(e.type>=EntityType::Cell&&e.type<=EntityType::Mind)totalLatency+=e.responseLatency;
    }
    const int n = std::max(1, int(m_entities.size()));
    s.entropy=computeEntropy(); s.complexity=(totalMass/n)*(1.0+0.5*s.colonyCount+1.2*s.mindCount)/(1.0+0.02*n);
    s.avgGeneration=totalGen/n; s.avgLevel=totalLevel/n;
    s.power=totalEnergy/std::max(1.0,double(n));
    s.responseLatency=living?totalLatency/living:0.0;
    s.efficiency=totalAbs>1e-9?clamp01((totalAbs-totalSpent*0.42)/totalAbs):0.0;
    s.decisionIndex=clamp01((s.avgLevel*0.28 + s.avgGeneration*0.015 + (1.0-s.responseLatency)*0.22));
    const double dts=m_time-m_lastSampleTime;
    if(dts>0.75){s.evolutionRate=(m_entities.size()-m_lastSampleTotal)/dts;m_lastSampleTime=m_time;m_lastSampleTotal=m_entities.size();}
    else s.evolutionRate=m_metrics.lastEvolutionRate();
    m_metrics.record(s);
}

void Universe::publishSnapshot(RenderSnapshot &snap) const
{
    QMutexLocker lock(&snap.mutex); snap.clear(); const int N=m_entities.size();
#define RES(v) snap.v.reserve(N)
    RES(posX);RES(posY);RES(size);RES(energy);RES(type);RES(direction);RES(habitability);RES(temperature);RES(mass);RES(orbitRadius);
    RES(id);RES(parentId);RES(galaxyId);RES(population);RES(generation);RES(level);
#undef RES
    for(const auto&e:m_entities){
        snap.posX.append(float(e.pos.x()));snap.posY.append(float(e.pos.y()));snap.type.append(float(int(e.type)));
        float sz=float(std::max(1.0,e.radius));
        if(e.type==EntityType::Quant)sz=1.5f;else if(e.type==EntityType::Particle)sz=2.4f;else if(e.type==EntityType::Cell)sz=3.2f;
        else if(e.type==EntityType::Colony)sz=5.0f;else if(e.type==EntityType::Mind)sz=6.5f;else if(e.type==EntityType::Ship)sz=4.0f;
        snap.size.append(sz);snap.energy.append(float(clamp01(e.energy/100.0)));snap.direction.append(float(e.direction));
        snap.habitability.append(float(e.habitability));snap.temperature.append(float(e.temperature));snap.mass.append(float(e.mass));
        snap.orbitRadius.append(float(e.orbitRadius));snap.id.append(e.id);snap.parentId.append(e.parentId);snap.galaxyId.append(e.galaxyId);
        snap.population.append(e.population);snap.generation.append(e.generation);snap.level.append(e.level);
    }
    snap.time=m_time;snap.worldRadius=m_worldRadius;snap.epoch=m_epoch;snap.totalCount=N;
}

QJsonObject Universe::reportJson() const
{
    QJsonObject root; root["format"]="afterlife.simulation-report.v2";root["scenario"]=m_scenario.toJson();
    root["simulationTime"]=m_time;root["epoch"]=m_epoch;root["entityCount"]=m_entities.size();
    QJsonObject summary; summary["text"]=QString("Прогон завершён на t=%1, эпоха=%2. Звёзд=%3, планет=%4, спутников=%5, пригодных миров=%6, клеток=%7, колоний=%8, разумных агентов=%9. Энтропия=%10, эффективность=%11, индекс выбора=%12.").arg(m_time,0,'f',2).arg(m_epoch).arg(m_metrics.lastStarCount()).arg(m_metrics.lastPlanetCount()).arg(m_metrics.lastMoonCount()).arg(m_metrics.lastHabitableCount()).arg(m_metrics.lastCellCount()).arg(m_metrics.lastColonyCount()).arg(m_metrics.lastMindCount()).arg(m_metrics.lastEntropy(),0,'f',3).arg(m_metrics.lastEfficiency(),0,'f',3).arg(m_metrics.lastDecisionIndex(),0,'f',3); summary["modelNote"]="Нормализованная интерактивная модель: физические зависимости мотивированы известными законами, но масштаб, размерность и численная интеграция оптимизированы для реального времени."; root["summary"]=summary;
    QJsonArray events;for(const auto&e:m_events)events.append(e);root["events"]=events;
    root["metrics"]=m_metrics.toJsonArray();
    QJsonArray entities;
    for(const auto&e:m_entities){
        QJsonObject o;o["id"]=e.id;o["type"]=int(e.type);o["typeName"]=entityTypeName(e.type);o["parentId"]=e.parentId;o["galaxyId"]=e.galaxyId;o["superclusterId"]=e.superclusterId;
        o["x"]=e.pos.x();o["y"]=e.pos.y();o["vx"]=e.vel.x();o["vy"]=e.vel.y();o["mass"]=e.mass;o["radius"]=e.radius;o["energy"]=e.energy;o["age"]=e.age;o["generation"]=e.generation;o["level"]=e.level;
        o["orbitRadius"]=e.orbitRadius;o["orbitAngle"]=e.orbitAngle;o["orbitSpeed"]=e.orbitSpeed;o["eccentricity"]=e.eccentricity;o["luminosity"]=e.luminosity;
        o["habitability"]=e.habitability;o["temperature"]=e.temperature;o["albedo"]=e.albedo;o["waterFraction"]=e.waterFraction;o["atmosphere"]=e.atmosphere;o["magneticMoment"]=e.magneticMoment;o["radiationDose"]=e.radiationDose;o["resourceRichness"]=e.resourceRichness;
        o["population"]=e.population;o["carryingCapacity"]=e.carryingCapacity;o["biosphere"]=e.biosphere;o["technology"]=e.technology;o["fitness"]=e.fitness;o["responseLatency"]=e.responseLatency;o["lastAction"]=e.lastAction;
        QJsonArray weights;for(int a=0;a<3;++a){QJsonArray row;for(int b=0;b<2;++b)row.append(e.w[a][b]);weights.append(row);}o["decisionWeights"]=weights;
        entities.append(o);
    }
    root["finalState"]=entities;
    return root;
}
