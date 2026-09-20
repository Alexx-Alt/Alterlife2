#include "simulator.h"
#include "physics.h"
#include <QMetaObject>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QRandomGenerator>
#include <cmath>
#include <algorithm>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>


namespace {
QString resolveUserPath(const QString &path)
{
    QFileInfo fi(path);
    if (fi.isAbsolute()) return fi.absoluteFilePath();
    QString base = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    if (base.isEmpty()) base = QDir::homePath();
    QDir dir(base); dir.mkpath("AfterLife");
    return dir.filePath("AfterLife/" + path);
}
}

Simulator::Simulator(QObject *parent):QObject(parent)
{
    m_worker=new SimulationWorker;m_worker->moveToThread(&m_thread);
    connect(&m_thread,&QThread::finished,m_worker,&QObject::deleteLater);
    connect(m_worker,&SimulationWorker::metricsPacketReady,this,&Simulator::onMetricsPacket);
    connect(m_worker,&SimulationWorker::logMessage,this,&Simulator::logMessage);
    connect(m_worker,&SimulationWorker::runningChanged,this,[this](bool r){m_running=r;emit runningChanged();});
    connect(m_worker,&SimulationWorker::speedChanged,this,[this](double s){m_speed=s;emit speedChanged();});
    m_thread.start();QMetaObject::invokeMethod(m_worker,"initialize",Qt::QueuedConnection);
}
Simulator::~Simulator(){m_thread.quit();m_thread.wait();}
#define INVOKE0(name) void Simulator::name(){QMetaObject::invokeMethod(m_worker,#name,Qt::QueuedConnection);}
INVOKE0(start) INVOKE0(pause) INVOKE0(reset) INVOKE0(randomize) INVOKE0(skipToNextEpoch)
#undef INVOKE0
void Simulator::setRunning(bool r){QMetaObject::invokeMethod(m_worker,"setRunning",Qt::QueuedConnection,Q_ARG(bool,r));}
void Simulator::setSpeed(double s){QMetaObject::invokeMethod(m_worker,"setSpeed",Qt::QueuedConnection,Q_ARG(double,s));}
void Simulator::applyScenarioFromQml(const QString &j){QMetaObject::invokeMethod(m_worker,"applyScenarioJson",Qt::QueuedConnection,Q_ARG(QString,j));}
void Simulator::exportCsv(const QString&p){const QString out=resolveUserPath(p);QMetaObject::invokeMethod(m_worker,"exportCsv",Qt::QueuedConnection,Q_ARG(QString,out));}
void Simulator::exportJson(const QString&p){const QString out=resolveUserPath(p);QMetaObject::invokeMethod(m_worker,"exportJson",Qt::QueuedConnection,Q_ARG(QString,out));}
void Simulator::exportReport(const QString&p){const QString out=resolveUserPath(p);QMetaObject::invokeMethod(m_worker,"exportReport",Qt::QueuedConnection,Q_ARG(QString,out));}
void Simulator::fastForward(double s){QMetaObject::invokeMethod(m_worker,"fastForward",Qt::QueuedConnection,Q_ARG(double,s));}
void Simulator::onMetricsPacket(const MetricsPacket&p){m_packet=p;m_history.append(p);if(m_history.size()>720)m_history.remove(0,m_history.size()-720);emit tick();}

QVariantMap Simulator::assessLifePotential(const QString &json) const
{
    Scenario s; auto doc=QJsonDocument::fromJson(json.toUtf8()); if(doc.isObject()) s=Scenario::fromJson(doc.object());
    QRandomGenerator rng(s.seed); Physics physics(s);
    const double starMin=std::max(0.01,std::min(s.stellarMassMin,s.stellarMassMax));
    const double starMax=std::max(starMin,std::max(s.stellarMassMin,s.stellarMassMax));
    const double planetMin=std::max(0.001,std::min(s.minPlanetMass,s.maxPlanetMass));
    const double planetMax=std::max(planetMin,std::max(s.minPlanetMass,s.maxPlanetMass));
    const int trials=std::clamp(std::max(12,std::max(0,s.planetsPerStar)*std::max(1,s.galaxySeedCount)*3),12,256);
    double best=0.0,sum=0.0; int viable=0;
    for(int k=0;k<trials;++k){
        const double starMass=starMin+rng.generateDouble()*std::max(0.01,starMax-starMin);
        const double lum=std::pow(std::max(0.08,starMass),3.5);
        const double au=0.25+rng.generateDouble()*2.9;
        const double pm=(planetMin+rng.generateDouble()*std::max(0.01,planetMax-planetMin))*std::max(0.05,s.planetMass);
        const double albedo=std::clamp(s.meanAlbedo+(rng.generateDouble()-.5)*.18,0.02,.9);
        const double water=std::clamp(s.oceanFraction+(rng.generateDouble()-.5)*.45,0.0,1.0);
        const double atmosphere=std::clamp(s.atmosphereRetention*(.45+.55*std::pow(std::max(.08,pm),.25))+(rng.generateDouble()-.5)*.16,0.0,1.2);
        const double magnetic=std::clamp(s.magneticStrength*(.55+.45*rng.generateDouble())*std::pow(std::max(.1,pm),.22)*std::pow(std::max(.05,s.rotationSpeed),.28),0.0,1.0);
        const double temp=physics.equilibriumTemperature(lum,au,albedo,s.greenhouseFactor*atmosphere);
        const double radiation=std::clamp((lum/std::max(.1,au*au))*(1.0-magnetic*.72)*.16+s.radiationNoise,0.0,1.0);
        const double thermal=std::exp(-std::pow((temp-1.0)/.27,2.0));
        const double wf=std::exp(-std::pow((water-.58)/.38,2.0));
        const double hzScale=std::sqrt(std::max(.02,lum));
        const double hzInner=std::max(.05,s.habitableZoneInner*hzScale),hzOuter=std::max(hzInner+.05,s.habitableZoneOuter*hzScale);
        const double hzMid=.5*(hzInner+hzOuter),hzHalf=std::max(.08,.5*(hzOuter-hzInner));
        const double hz=std::exp(-.65*std::pow((au-hzMid)/hzHalf,4.0));
        const double gravity=std::exp(-std::pow((std::pow(pm,.42)-1.0)/.85,2.0));
        const double resource=.35+.65*rng.generateDouble();
        const double h=std::clamp(thermal*hz*wf*gravity*(.35+.65*std::min(1.0,atmosphere))*(.35+.65*(1-radiation))*(.45+.55*std::clamp(s.chemistryRichness*resource,0.0,1.0)),0.0,1.0);
        best=std::max(best,h);sum+=h;if(h>=.32)viable++;
    }
    const double mean=sum/trials;
    const double score=std::clamp(.55*best+.45*(1.0-std::exp(-mean*3.0)),0.0,1.0);
    const double eventChance=1.0-std::exp(-std::max(0.0,s.abiogenesisRate)*sum*220.0);
    QVariantMap r;r["score"]=score;r["bestWorld"]=best;r["meanHabitability"]=mean;r["viableCandidates"]=viable;r["samples"]=trials;r["abiogenesisChance"]=eventChance;r["seed"]=(qulonglong)s.seed;
    r["verdict"]=score>.66?"Высокий потенциал":score>.36?"Умеренный потенциал":"Низкий потенциал";
    r["explanation"]="Seed-детерминированный скрининг синтетических звёзд и планет по тем же температурным, атмосферным, магнитным, водным и радиационным зависимостям, что использует основной режим. Это оценка потенциала, не гарантия абиогенеза.";
    return r;
}

QVariantList Simulator::historySnapshot() const
{
    QVariantList out;out.reserve(m_history.size());for(const auto&p:m_history){QVariantMap m;m["time"]=p.time;m["entropy"]=p.entropy;m["complexity"]=p.complexity;m["decisionIndex"]=p.decisionIndex;m["entities"]=p.totalEntities;m["life"]=p.cellCount+p.colonyCount+p.mindCount;out.append(m);}return out;
}


bool Simulator::loadReportForReplay(const QString &path)
{
    const QString resolved=resolveUserPath(path); QFile f(resolved); if(!f.open(QIODevice::ReadOnly)){emit logMessage("Не удалось открыть отчёт: "+resolved);return false;}
    const auto doc=QJsonDocument::fromJson(f.readAll()); if(!doc.isObject()){emit logMessage("Некорректный JSON отчёта");return false;}
    const auto root=doc.object(); QJsonArray arr=root.value("metrics").toArray(); if(arr.isEmpty())arr=root.value("samples").toArray();
    if(arr.isEmpty()){emit logMessage("В отчёте нет истории метрик");return false;}
    QVariantList history; history.reserve(arr.size());
    for(const auto &v:arr){const auto o=v.toObject();QVariantMap m;m["time"]=o.value("time").toDouble();m["entropy"]=o.value("entropy").toDouble();m["complexity"]=o.value("complexity").toDouble();m["decisionIndex"]=o.value("decisionIndex").toDouble();const int entities=o.value("quantCount").toInt()+o.value("particleCount").toInt()+o.value("cellCount").toInt()+o.value("colonyCount").toInt()+o.value("mindCount").toInt()+o.value("starCount").toInt()+o.value("planetCount").toInt()+o.value("shipCount").toInt()+o.value("galaxyCount").toInt()+o.value("superclusterCount").toInt()+o.value("moonCount").toInt();m["entities"]=entities;m["life"]=o.value("cellCount").toInt()+o.value("colonyCount").toInt()+o.value("mindCount").toInt();history.append(m);}
    m_replayHistory=history;emit replayChanged();emit logMessage(QString("Отчёт загружен для графического воспроизведения: %1 точек").arg(history.size()));return true;
}


bool Simulator::saveScenario(const QString &path, const QString &scenarioJson)
{
    const auto doc = QJsonDocument::fromJson(scenarioJson.toUtf8());
    if (!doc.isObject()) { emit logMessage("Не удалось сохранить сценарий: некорректные параметры"); return false; }
    QJsonObject root;
    root["format"] = "afterlife.scenario.v2";
    root["scenario"] = doc.object();
    const QString resolved=resolveUserPath(path); QFile f(resolved);
    if (!f.open(QIODevice::WriteOnly)) { emit logMessage("Не удалось открыть файл сценария: "+resolved); return false; }
    f.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    emit logMessage("Сценарий сохранён: "+resolved);
    return true;
}

QString Simulator::loadScenario(const QString &path)
{
    const QString resolved=resolveUserPath(path); QFile f(resolved);
    if (!f.open(QIODevice::ReadOnly)) { emit logMessage("Не удалось открыть сценарий: "+resolved); return {}; }
    const auto doc = QJsonDocument::fromJson(f.readAll());
    if (!doc.isObject()) { emit logMessage("Некорректный JSON сценария"); return {}; }
    QJsonObject scenario = doc.object().value("scenario").toObject();
    if (scenario.isEmpty()) scenario = doc.object();
    const QString compact = QString::fromUtf8(QJsonDocument(scenario).toJson(QJsonDocument::Compact));
    applyScenarioFromQml(compact);
    emit logMessage("Сценарий загружен: "+resolved);
    return compact;
}
