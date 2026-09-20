#ifndef SCENARIO_H
#define SCENARIO_H

#include <QJsonObject>

// Scenario intentionally uses normalized simulation units. The UI exposes the
// physical meaning of each coefficient; the engine keeps the ratios coherent
// and converts them to stable integration units.
struct Scenario {
    // Initial state / numerical model
    int quantCount = 1200;
    double initialTemperature = 0.85;
    double gravityConstant = 0.42;
    double interactionRadius = 34.0;
    double friction = 0.997;
    double worldRadius = 520.0;
    double gravitationalSoftening = 3.5;
    double darkMatterFactor = 1.8;
    double gasCoolingRate = 0.18;
    double densityFluctuation = 0.55;
    int galaxySeedCount = 4;

    // Planetary environment
    double planetMass = 1.0;
    double rotationSpeed = 1.0;
    double tidalAmplitude = 0.25;
    double tidalFrequency = 0.5;
    double magneticStrength = 0.65;
    double radiationNoise = 0.035;
    double atmosphereRetention = 0.72;
    double oceanFraction = 0.62;
    double greenhouseFactor = 0.22;
    double meanAlbedo = 0.31;

    // Evolution / decision system
    double mutationRate = 0.045;
    double selectionPressure = 0.65;
    double learningRate = 0.0025;
    double memoryFactor = 0.28;
    double abiogenesisRate = 0.0035;
    double chemistryRichness = 0.72;

    // Energetics
    double baseAbsorptionRate = 34.0;
    double metabolismCost = 0.42;
    double movementCost = 0.08;
    double carryingCapacityPerHabitability = 55.0;
    double replicationEnergyThreshold = 62.0;
    double replicationCost = 0.46;

    // Star / system formation
    double starFormationThreshold = 18.0;
    int planetsPerStar = 6;
    double habitableZoneInner = 0.82;
    double habitableZoneOuter = 1.55;
    double planetFormationCost = 5.0;
    double minPlanetMass = 0.35;
    double maxPlanetMass = 6.5;
    double minOrbitSpacing = 24.0;
    double stellarMassMin = 0.55;
    double stellarMassMax = 1.8;
    int maxMoonsPerPlanet = 2;
    double minMoonOrbitSpacing = 3.5;

    // Civilisation / navigation
    double shipEnergyCost = 45.0;
    double shipSpeed = 8.0;
    double colonizationThreshold = 0.38;

    // Runtime
    int maxEntities = 3500;
    unsigned int seed = 42;

    QJsonObject toJson() const;
    static Scenario fromJson(const QJsonObject &o);
};

#endif // SCENARIO_H
