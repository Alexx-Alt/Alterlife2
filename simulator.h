#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <QObject>
#include <QThread>
#include <QVariantList>
#include <QVariantMap>
#include "simulationworker.h"

class Simulator : public QObject {
    Q_OBJECT
#define PROP(type,name,read,notify) Q_PROPERTY(type name READ read NOTIFY notify)
    Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged)
    Q_PROPERTY(double speed READ speed WRITE setSpeed NOTIFY speedChanged)
    PROP(double,time,time,tick) PROP(int,epoch,epoch,tick) PROP(int,totalEntities,totalEntities,tick)
    PROP(int,quantCount,quantCount,tick) PROP(int,particleCount,particleCount,tick) PROP(int,cellCount,cellCount,tick)
    PROP(int,colonyCount,colonyCount,tick) PROP(int,mindCount,mindCount,tick) PROP(int,starCount,starCount,tick)
    PROP(int,planetCount,planetCount,tick) PROP(int,habitablePlanetCount,habitablePlanetCount,tick) PROP(int,shipCount,shipCount,tick)
    PROP(int,galaxyCount,galaxyCount,tick) PROP(int,superclusterCount,superclusterCount,tick) PROP(int,moonCount,moonCount,tick)
    PROP(double,entropy,entropy,tick) PROP(double,complexity,complexity,tick) PROP(double,evolutionRate,evolutionRate,tick)
    PROP(double,avgGeneration,avgGeneration,tick) PROP(double,avgLevel,avgLevel,tick) PROP(double,power,power,tick)
    PROP(double,responseLatency,responseLatency,tick) PROP(double,efficiency,efficiency,tick) PROP(double,decisionIndex,decisionIndex,tick)
    Q_PROPERTY(SimulationWorker* worker READ worker CONSTANT)
    Q_PROPERTY(bool replayAvailable READ replayAvailable NOTIFY replayChanged)
#undef PROP
public:
    explicit Simulator(QObject *parent=nullptr); ~Simulator();
    bool running()const{return m_running;} double speed()const{return m_speed;} double time()const{return m_packet.time;} int epoch()const{return m_packet.epoch;}
    int totalEntities()const{return m_packet.totalEntities;} int quantCount()const{return m_packet.quantCount;} int particleCount()const{return m_packet.particleCount;}
    int cellCount()const{return m_packet.cellCount;} int colonyCount()const{return m_packet.colonyCount;} int mindCount()const{return m_packet.mindCount;}
    int starCount()const{return m_packet.starCount;} int planetCount()const{return m_packet.planetCount;} int habitablePlanetCount()const{return m_packet.habitablePlanetCount;}
    int shipCount()const{return m_packet.shipCount;} int galaxyCount()const{return m_packet.galaxyCount;} int superclusterCount()const{return m_packet.superclusterCount;} int moonCount()const{return m_packet.moonCount;}
    double entropy()const{return m_packet.entropy;} double complexity()const{return m_packet.complexity;} double evolutionRate()const{return m_packet.evolutionRate;}
    double avgGeneration()const{return m_packet.avgGeneration;} double avgLevel()const{return m_packet.avgLevel;} double power()const{return m_packet.power;}
    double responseLatency()const{return m_packet.responseLatency;} double efficiency()const{return m_packet.efficiency;} double decisionIndex()const{return m_packet.decisionIndex;}
    SimulationWorker*worker()const{return m_worker;} bool replayAvailable()const{return !m_replayHistory.isEmpty();}

    Q_INVOKABLE QVariantMap assessLifePotential(const QString &scenarioJson) const;
    Q_INVOKABLE QVariantList historySnapshot() const;
    Q_INVOKABLE QVariantList replayHistory() const { return m_replayHistory; }
    Q_INVOKABLE bool loadReportForReplay(const QString &path);
    Q_INVOKABLE bool saveScenario(const QString &path, const QString &scenarioJson);
    Q_INVOKABLE QString loadScenario(const QString &path);
public slots:
    void start();void pause();void setRunning(bool r);void setSpeed(double s);void reset();void applyScenarioFromQml(const QString &json);void randomize();
    void exportCsv(const QString &path);void exportJson(const QString &path);void exportReport(const QString &path);void fastForward(double seconds);void skipToNextEpoch();
signals:
    void runningChanged();void speedChanged();void tick();void logMessage(const QString &msg);void replayChanged();
private slots:void onMetricsPacket(const MetricsPacket&p);
private:
    QThread m_thread;SimulationWorker*m_worker=nullptr;bool m_running=false;double m_speed=1.0;MetricsPacket m_packet;QVector<MetricsPacket>m_history;QVariantList m_replayHistory;
};
#endif
