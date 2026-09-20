#ifndef PHYSICS_H
#define PHYSICS_H

#include "scenario.h"
#include <QPointF>

class Physics {
public:
    explicit Physics(const Scenario &s = Scenario{});
    void setScenario(const Scenario &s) { m_s = s; }
    const Scenario &scenario() const { return m_s; }

    double thermalField(const QPointF &p, double t, double worldR) const;
    double magneticField(const QPointF &p, double worldR) const;
    double tidalField(double t) const;
    QPointF gravityForce(const QPointF &p, double worldR) const;
    QPointF orbitForce(const QPointF &p, double worldR) const;
    QPointF radiationNoise() const;

    // Newtonian helpers in normalized units.
    QPointF pairGravity(const QPointF &delta, double sourceMass) const;
    double circularSpeed(double centralMass, double radius) const;
    double angularSpeed(double centralMass, double radius) const;
    double equilibriumTemperature(double luminosity, double distance,
                                  double albedo, double greenhouse) const;
    double hillRadius(double orbitRadius, double bodyMass, double parentMass) const;

private:
    Scenario m_s;
};

#endif // PHYSICS_H
