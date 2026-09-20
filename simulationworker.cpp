#include "simulationworker.h"
#include <QJsonDocument>
#include <QRandomGenerator>
#include <QFile>
#include <algorithm>

SimulationWorker::SimulationWorker(QObject *parent):QObject(parent)
{
    connect(&m_universe,&Universe::epochChanged,this,[this](int e){
        static const char *names[]={"ПЕРВИЧНАЯ СРЕДА","ЧАСТИЦЫ","ЗВЁЗДНЫЕ СИСТЕМЫ","ГАЛАКТИКИ","ЖИЗНЬ","КОЛОНИИ","РЕФЛЕКСИВНЫЙ ВЫБОР"};
        emit logMessage(QString("ЭПОХА %1 — %2").arg(e).arg(e>=0&&e<=6?names[e]:"?"));
    });
    connect(&m_universe,&Universe::logMessage,this,&SimulationWorker::logMessage);
}
SimulationWorker::~SimulationWorker(){if(m_timer){m_timer->stop();delete m_timer;}}
void SimulationWorker::initialize(){if(m_timer)return;m_timer=new QTimer(this);m_timer->setInterval(16);m_timer->setTimerType(Qt::PreciseTimer);connect(m_timer,&QTimer::timeout,this,&SimulationWorker::processTick);publish();}
void SimulationWorker::start(){setRunning(true);} void SimulationWorker::pause(){setRunning(false);}
void SimulationWorker::setRunning(bool r){if(m_running==r)return;m_running=r;if(!m_timer)initialize();if(r)m_timer->start();else m_timer->stop();emit runningChanged(r);}
void SimulationWorker::setSpeed(double s){s=std::clamp(s,0.02,1000.0);if(qFuzzyCompare(m_speed,s))return;m_speed=s;emit speedChanged(s);}

MetricsPacket SimulationWorker::buildPacket() const
{
    MetricsPacket p; const auto&m=m_universe.metrics();
    p.time=m_universe.time();p.epoch=m_universe.epoch();p.totalEntities=m_universe.entities().size();
    p.quantCount=m.lastQuantCount();p.particleCount=m.lastParticleCount();p.cellCount=m.lastCellCount();p.colonyCount=m.lastColonyCount();p.mindCount=m.lastMindCount();
    p.starCount=m.lastStarCount();p.planetCount=m.lastPlanetCount();p.habitablePlanetCount=m.lastHabitableCount();p.shipCount=m.lastShipCount();
    p.galaxyCount=m.lastGalaxyCount();p.superclusterCount=m.lastSuperclusterCount();p.moonCount=m.lastMoonCount();p.entropy=m.lastEntropy();p.complexity=m.lastComplexity();
    p.evolutionRate=m.lastEvolutionRate();p.avgGeneration=m.lastAvgGeneration();p.avgLevel=m.lastAvgLevel();p.power=m.lastPower();
    p.responseLatency=m.lastResponseLatency();p.efficiency=m.lastEfficiency();p.decisionIndex=m.lastDecisionIndex();return p;
}
void SimulationWorker::publish(){m_universe.publishSnapshot(m_snapshot);emit snapshotReady();emit metricsPacketReady(buildPacket());}
void SimulationWorker::reset(){m_universe.reset();publish();emit logMessage("Симуляция сброшена");}
void SimulationWorker::applyScenarioJson(const QString &json){auto doc=QJsonDocument::fromJson(json.toUtf8());if(!doc.isObject()){emit logMessage("Ошибка: некорректный JSON сценария");return;}m_universe.setScenario(Scenario::fromJson(doc.object()));publish();emit logMessage("Новый сценарий применён");}
void SimulationWorker::randomize(){Scenario s=m_universe.scenario();s.seed=QRandomGenerator::global()->generate();m_universe.setScenario(s);publish();emit logMessage(QString("Случайный seed: %1").arg(s.seed));}
void SimulationWorker::exportCsv(const QString &path){emit logMessage(m_universe.metrics().exportCsv(path)?"CSV метрик сохранён: "+path:"Не удалось сохранить CSV");}
void SimulationWorker::exportJson(const QString &path){emit logMessage(m_universe.metrics().exportJson(path)?"JSON метрик сохранён: "+path:"Не удалось сохранить JSON");}
void SimulationWorker::exportReport(const QString &path){QFile f(path);if(!f.open(QIODevice::WriteOnly)){emit logMessage("Не удалось сохранить полный отчёт");return;}f.write(QJsonDocument(m_universe.reportJson()).toJson(QJsonDocument::Indented));emit logMessage("Полный отчёт сохранён: "+path);}
void SimulationWorker::fastForward(double seconds){const bool was=m_running;if(was)setRunning(false);emit logMessage(QString("Ускоренное моделирование: +%1 условных секунд").arg(seconds,0,'f',0));m_universe.fastForward(seconds);publish();if(was)setRunning(true);}
void SimulationWorker::skipToNextEpoch(){const bool was=m_running;if(was)setRunning(false);const int before=m_universe.epoch();for(int i=0;i<7000&&m_universe.epoch()==before;++i)m_universe.step(0.35);publish();emit logMessage(m_universe.epoch()>before?"Переход к следующей достигнутой эпохе завершён":"Следующая эпоха не возникла при текущих условиях");if(was)setRunning(true);}
void SimulationWorker::processTick(){
    // Real-time mode keeps the integration step small; high speed increases the
    // amount of simulated time, not the force magnitudes.
    double remaining=0.016*m_speed; int guard=0;
    while(remaining>1e-9&&guard++<128){const double dt=std::min(m_speed>50.0?0.25:0.08,remaining);m_universe.step(dt);remaining-=dt;}
    publish();
}
