#ifndef METRICS_H
#define METRICS_H

#include <QObject>
#include <QVector>
#include <QJsonArray>

struct MetricsSample {
    double time = 0.0;
    int epoch = 0;
    int quantCount = 0;
    int particleCount = 0;
    int cellCount = 0;
    int colonyCount = 0;
    int mindCount = 0;
    int starCount = 0;
    int planetCount = 0;
    int habitablePlanetCount = 0;
    int shipCount = 0;
    int galaxyCount = 0;
    int superclusterCount = 0;
    int moonCount = 0;
    double entropy = 0.0;
    double complexity = 0.0;
    double evolutionRate = 0.0;
    double avgGeneration = 0.0;
    double avgLevel = 0.0;
    double power = 0.0;
    double responseLatency = 0.0;
    double efficiency = 0.0;
    double decisionIndex = 0.0;
};

class Metrics : public QObject {
    Q_OBJECT
public:
    explicit Metrics(QObject *parent = nullptr) : QObject(parent) {}
    void record(const MetricsSample &s) {
        m_history.append(s);
        if (m_history.size() > 7200) m_history.remove(0, m_history.size()-7200);
    }
    void clear() { m_history.clear(); }
    const QVector<MetricsSample> &history() const { return m_history; }
    bool exportCsv(const QString &path) const;
    bool exportJson(const QString &path) const;
    QJsonArray toJsonArray() const;

#define LAST_D(name, field) double name() const { return m_history.isEmpty()?0.0:m_history.last().field; }
#define LAST_I(name, field) int name() const { return m_history.isEmpty()?0:m_history.last().field; }
    LAST_D(lastEntropy, entropy)
    LAST_D(lastComplexity, complexity)
    LAST_D(lastEvolutionRate, evolutionRate)
    LAST_D(lastAvgGeneration, avgGeneration)
    LAST_D(lastAvgLevel, avgLevel)
    LAST_D(lastPower, power)
    LAST_D(lastResponseLatency, responseLatency)
    LAST_D(lastEfficiency, efficiency)
    LAST_D(lastDecisionIndex, decisionIndex)
    LAST_I(lastQuantCount, quantCount)
    LAST_I(lastParticleCount, particleCount)
    LAST_I(lastCellCount, cellCount)
    LAST_I(lastColonyCount, colonyCount)
    LAST_I(lastMindCount, mindCount)
    LAST_I(lastStarCount, starCount)
    LAST_I(lastPlanetCount, planetCount)
    LAST_I(lastHabitableCount, habitablePlanetCount)
    LAST_I(lastShipCount, shipCount)
    LAST_I(lastGalaxyCount, galaxyCount)
    LAST_I(lastSuperclusterCount, superclusterCount)
    LAST_I(lastMoonCount, moonCount)
    LAST_I(lastEpoch, epoch)
#undef LAST_D
#undef LAST_I
private:
    QVector<MetricsSample> m_history;
};

#endif // METRICS_H
