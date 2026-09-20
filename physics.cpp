#include "physics.h"
#include <QtMath>
#include <QRandomGenerator>
#include <algorithm>

Physics::Physics(const Scenario &s) : m_s(s) {}

double Physics::thermalField(const QPointF &p, double t, double worldR) const
{
    const double r = std::hypot(p.x(), p.y());
    const double norm = std::clamp(r / std::max(1.0, worldR), 0.0, 2.0);
    const double dayNight = 0.78 + 0.22 * std::sin(t * m_s.rotationSpeed - norm * 4.0);
    return std::clamp(dayNight * std::exp(-0.65 * norm), 0.0, 1.5);
}

double Physics::magneticField(const QPointF &p, double worldR) const
{
    const double r = std::hypot(p.x(), p.y()) + 1.0;
    const double normalized = r / std::max(1.0, worldR);
    return m_s.magneticStrength / std::pow(1.0 + normalized, 3.0);
}

double Physics::tidalField(double t) const
{
    return std::cos(t * m_s.tidalFrequency) * m_s.tidalAmplitude;
}

QPointF Physics::pairGravity(const QPointF &delta, double sourceMass) const
{
    const double eps2 = m_s.gravitationalSoftening * m_s.gravitationalSoftening;
    const double r2 = delta.x()*delta.x() + delta.y()*delta.y() + eps2;
    const double invR = 1.0 / std::sqrt(r2);
    const double a = m_s.gravityConstant * std::max(0.0, sourceMass) / r2;
    return delta * (a * invR);
}

QPointF Physics::gravityForce(const QPointF &p, double worldR) const
{
    // Smooth dark-matter halo. This replaces the former linear inward force.
    const QPointF toCenter = -p;
    const double r = std::hypot(p.x(), p.y()) + 1e-6;
    const double scaleR = std::max(40.0, worldR * 0.42);
    const double enclosed = m_s.darkMatterFactor * (r*r*r) / (r*r*r + scaleR*scaleR*scaleR);
    return pairGravity(toCenter, enclosed * 240.0);
}

QPointF Physics::orbitForce(const QPointF &p, double worldR) const
{
    Q_UNUSED(worldR);
    const double r = std::hypot(p.x(), p.y()) + 1e-6;
    const QPointF tangent(-p.y()/r, p.x()/r);
    return tangent * (0.0009 * m_s.darkMatterFactor);
}

QPointF Physics::radiationNoise() const
{
    auto *rng = QRandomGenerator::global();
    return QPointF((rng->generateDouble()-0.5) * m_s.radiationNoise,
                   (rng->generateDouble()-0.5) * m_s.radiationNoise);
}

double Physics::circularSpeed(double centralMass, double radius) const
{
    return std::sqrt(std::max(0.0, m_s.gravityConstant * centralMass / std::max(1.0, radius)));
}

double Physics::angularSpeed(double centralMass, double radius) const
{
    return circularSpeed(centralMass, radius) / std::max(1.0, radius);
}

double Physics::equilibriumTemperature(double luminosity, double distance,
                                       double albedo, double greenhouse) const
{
    // Earth-like normalized equilibrium relation: T ~ L^(1/4) / sqrt(r),
    // corrected for Bond albedo and greenhouse retention.
    const double absorbed = std::max(0.02, 1.0 - std::clamp(albedo, 0.0, 0.95));
    const double base = std::pow(std::max(0.01, luminosity) * absorbed / 0.69, 0.25)
                        / std::sqrt(std::max(0.08, distance));
    return base * (1.0 + std::clamp(greenhouse, 0.0, 3.0) * 0.22);
}

double Physics::hillRadius(double orbitRadius, double bodyMass, double parentMass) const
{
    return orbitRadius * std::cbrt(std::max(1e-9, bodyMass / (3.0 * std::max(1e-9, parentMass))));
}
