#ifndef ENTITY_H
#define ENTITY_H

#include <QPointF>

// Numeric values below 10 are biological/material stages. Astronomical
// structures start at 10 so old reports remain readable.
enum class EntityType {
    Quant = 0,
    Particle = 1,
    Cell = 2,
    Colony = 3,
    Mind = 4,
    Star = 10,
    Planet = 11,
    Ship = 12,
    GasCloud = 13,
    Galaxy = 14,
    Supercluster = 15,
    Moon = 16
};

struct Entity {
    int id = 0;
    EntityType type = EntityType::Quant;
    bool alive = true;

    QPointF pos;
    QPointF vel;
    double mass = 1.0;
    double radius = 1.0;
    double energy = 20.0;
    double age = 0.0;
    int generation = 0;

    // Decision system. Three environmental sensors feed two action neurons.
    double sensorThermal = 0.0;
    double sensorMagnetic = 0.0;
    double sensorTidal = 0.0;
    double lastAction = 0.0;
    double prevAction = 0.0;
    double w[3][2] = {{1,0}, {0,1}, {0.25,-0.25}};
    int level = 0;
    double fitness = 0.0;
    double responseLatency = 1.0;

    double totalAbsorbed = 0.0;
    double totalSpent = 0.0;
    int childrenCount = 0;
    double starvationTime = 0.0;

    // Hierarchy / astronomy
    int parentId = -1;
    int galaxyId = -1;
    int superclusterId = -1;
    double orbitRadius = 0.0;
    double orbitAngle = 0.0;
    double orbitSpeed = 0.0;
    double eccentricity = 0.0;
    double luminosity = 1.0;
    double temperature = 0.0;
    double habitability = 0.0;
    double albedo = 0.3;
    double waterFraction = 0.0;
    double atmosphere = 0.0;
    double magneticMoment = 0.0;
    double radiationDose = 0.0;
    double resourceRichness = 0.5;
    double biosphere = 0.0;
    double technology = 0.0;
    double carryingCapacity = 0.0;
    int population = 0;

    // Ship heading (also mirrored by lastAction for old renderer paths).
    double direction = 0.0;
};

#endif // ENTITY_H
