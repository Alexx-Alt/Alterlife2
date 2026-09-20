#ifndef SIMULATIONWORKER_H
#define SIMULATIONWORKER_H

#include <QObject>
#include <QTimer>
#include <QMetaType>
#include "universe.h"

struct MetricsPacket {
    double time=0.0; int epoch=0; int totalEntities=0;
    int quantCount=0,particleCount=0,cellCount=0,colonyCount=0,mindCount=0;
    int starCount=0,planetCount=0,habitablePlanetCount=0,shipCount=0,galaxyCount=0,superclusterCount=0,moonCount=0;
    double entropy=0.0,complexity=0.0,evolutionRate=0.0,avgGeneration=0.0,avgLevel=0.0;
    double power=0.0,responseLatency=0.0,efficiency=0.0,decisionIndex=0.0;
};
Q_DECLARE_METATYPE(MetricsPacket)

class SimulationWorker : public QObject {
    Q_OBJECT
public:
    explicit SimulationWorker(QObject *parent=nullptr);
    ~SimulationWorker();
    Universe &universe(){return m_universe;}
    RenderSnapshot &snapshot(){return m_snapshot;}
public slots:
    void initialize(); void start(); void pause(); void setRunning(bool r); void setSpeed(double s);
    void reset(); void applyScenarioJson(const QString &json); void randomize();
    void exportCsv(const QString &path); void exportJson(const QString &path); void exportReport(const QString &path);
    void fastForward(double seconds); void skipToNextEpoch(); void processTick();
signals:
    void snapshotReady(); void metricsPacketReady(const MetricsPacket &packet); void runningChanged(bool r);
    void speedChanged(double s); void logMessage(const QString &msg);
private:
    MetricsPacket buildPacket() const;
    void publish();
    Universe m_universe; RenderSnapshot m_snapshot; QTimer *m_timer=nullptr; bool m_running=false; double m_speed=1.0;
};
#endif
