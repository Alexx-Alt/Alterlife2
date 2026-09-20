#ifndef UNIVERSE_H
#define UNIVERSE_H

#include "entity.h"
#include "scenario.h"
#include "metrics.h"
#include "physics.h"

#include <QObject>
#include <QVector>
#include <QMutex>
#include <QRandomGenerator>
#include <QJsonObject>
#include <QStringList>
#include <QHash>

struct RenderSnapshot {
    QVector<float> posX, posY, size, energy, type, direction;
    QVector<float> habitability, temperature, mass, orbitRadius;
    QVector<int> id, parentId, galaxyId, population, generation, level;
    double time = 0.0;
    double worldRadius = 520.0;
    int epoch = 0;
    int totalCount = 0;
    QMutex mutex;

    void clear() {
        posX.clear(); posY.clear(); size.clear(); energy.clear(); type.clear(); direction.clear();
        habitability.clear(); temperature.clear(); mass.clear(); orbitRadius.clear();
        id.clear(); parentId.clear(); galaxyId.clear(); population.clear(); generation.clear(); level.clear();
    }
};

class Universe : public QObject {
    Q_OBJECT
public:
    explicit Universe(QObject *parent = nullptr);

    void setScenario(const Scenario &s);
    Scenario scenario() const { return m_scenario; }
    void reset();
    void step(double dt);
    void fastForward(double simulatedSeconds, int maxSteps = 2500);

    const QVector<Entity> &entities() const { return m_entities; }
    double time() const { return m_time; }
    double worldRadius() const { return m_worldRadius; }
    int epoch() const { return m_epoch; }
    Metrics &metrics() { return m_metrics; }
    const Metrics &metrics() const { return m_metrics; }
    const QStringList &events() const { return m_events; }

    void publishSnapshot(RenderSnapshot &snap) const;
    QJsonObject reportJson() const;

signals:
    void entityEvolved(int id, int newType);
    void epochChanged(int newEpoch);
    void logMessage(const QString &msg);

private:
    void spawnInitialQuants();
    void updatePlanetCapacities();
    void updateCosmicStructures();
    void tryFormStars(double dt);
    void formPlanetsForStar(Entity &star, int absorbedMass);
    void formMoonsForPlanet(const Entity &star, const Entity &planet);
    void trySpawnLife(double dt);
    void tryBuildShip(Entity &colony);
    void replicate(Entity &parent);
    void tryEvolve();
    void updateEpoch();
    void computeMetrics();
    double computeEntropy() const;
    void enforceLimit();
    Entity *findEntity(int id);
    const Entity *findEntity(int id) const;
    double nearestStarDistance(const QPointF &p, int exceptId = -1) const;
    double maxOrbitRadiusForStar(int starId) const;
    bool starSystemClearance(const QPointF &candidate, double candidateRadius, int exceptId = -1) const;
    void recordEvent(const QString &msg);
    void rebuildIndex();

    Scenario m_scenario;
    Physics m_physics;
    QVector<Entity> m_entities;
    QHash<int,int> m_idIndex;
    QVector<QPointF> m_galaxySeeds;
    Metrics m_metrics;
    QRandomGenerator m_rng;
    QStringList m_events;

    double m_time = 0.0;
    double m_worldRadius = 520.0;
    int m_nextId = 1;
    int m_epoch = 0;
    int m_maxEntities = 3500;
    double m_lastSampleTime = 0.0;
    int m_lastSampleTotal = 0;
    double m_lastStarFormation = -100.0;
    double m_lastStructureUpdate = -100.0;
};

#endif // UNIVERSE_H
