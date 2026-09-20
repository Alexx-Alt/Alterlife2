#include "scenario.h"

namespace {
static double d(const QJsonObject &o, const char *k, double fallback) {
    return o.contains(k) ? o.value(k).toDouble(fallback) : fallback;
}
static int i(const QJsonObject &o, const char *k, int fallback) {
    return o.contains(k) ? o.value(k).toInt(fallback) : fallback;
}
}

QJsonObject Scenario::toJson() const
{
    QJsonObject o;
#define PUT(x) o[#x] = x
    PUT(quantCount); PUT(initialTemperature); PUT(gravityConstant); PUT(interactionRadius);
    PUT(friction); PUT(worldRadius); PUT(gravitationalSoftening); PUT(darkMatterFactor);
    PUT(gasCoolingRate); PUT(densityFluctuation); PUT(galaxySeedCount);
    PUT(planetMass); PUT(rotationSpeed); PUT(tidalAmplitude); PUT(tidalFrequency);
    PUT(magneticStrength); PUT(radiationNoise); PUT(atmosphereRetention); PUT(oceanFraction);
    PUT(greenhouseFactor); PUT(meanAlbedo); PUT(mutationRate); PUT(selectionPressure);
    PUT(learningRate); PUT(memoryFactor); PUT(abiogenesisRate); PUT(chemistryRichness);
    PUT(baseAbsorptionRate); PUT(metabolismCost); PUT(movementCost);
    PUT(carryingCapacityPerHabitability); PUT(replicationEnergyThreshold); PUT(replicationCost);
    PUT(starFormationThreshold); PUT(planetsPerStar); PUT(habitableZoneInner);
    PUT(habitableZoneOuter); PUT(planetFormationCost); PUT(minPlanetMass); PUT(maxPlanetMass);
    PUT(minOrbitSpacing); PUT(stellarMassMin); PUT(stellarMassMax); PUT(maxMoonsPerPlanet); PUT(minMoonOrbitSpacing); PUT(shipEnergyCost);
    PUT(shipSpeed); PUT(colonizationThreshold); PUT(maxEntities);
#undef PUT
    o["seed"] = static_cast<qint64>(seed);
    return o;
}

Scenario Scenario::fromJson(const QJsonObject &o)
{
    Scenario s;
#define GETD(x) s.x = d(o, #x, s.x)
#define GETI(x) s.x = i(o, #x, s.x)
    GETI(quantCount); GETD(initialTemperature); GETD(gravityConstant); GETD(interactionRadius);
    GETD(friction); GETD(worldRadius); GETD(gravitationalSoftening); GETD(darkMatterFactor);
    GETD(gasCoolingRate); GETD(densityFluctuation); GETI(galaxySeedCount);
    GETD(planetMass); GETD(rotationSpeed); GETD(tidalAmplitude); GETD(tidalFrequency);
    GETD(magneticStrength); GETD(radiationNoise); GETD(atmosphereRetention); GETD(oceanFraction);
    GETD(greenhouseFactor); GETD(meanAlbedo); GETD(mutationRate); GETD(selectionPressure);
    GETD(learningRate); GETD(memoryFactor); GETD(abiogenesisRate); GETD(chemistryRichness);
    GETD(baseAbsorptionRate); GETD(metabolismCost); GETD(movementCost);
    GETD(carryingCapacityPerHabitability); GETD(replicationEnergyThreshold); GETD(replicationCost);
    GETD(starFormationThreshold); GETI(planetsPerStar); GETD(habitableZoneInner);
    GETD(habitableZoneOuter); GETD(planetFormationCost); GETD(minPlanetMass); GETD(maxPlanetMass);
    GETD(minOrbitSpacing); GETD(stellarMassMin); GETD(stellarMassMax); GETI(maxMoonsPerPlanet); GETD(minMoonOrbitSpacing); GETD(shipEnergyCost);
    GETD(shipSpeed); GETD(colonizationThreshold); GETI(maxEntities);
#undef GETD
#undef GETI
    s.seed = static_cast<unsigned int>(o.value("seed").toInteger(s.seed));
    return s;
}
