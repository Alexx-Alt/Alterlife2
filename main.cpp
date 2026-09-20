#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QtQml>
#include "simulator.h"
#include "universerenderer.h"
#include "particlefield.h"
#include "crystalfield.h"
#include "localassistant.h"

int main(int argc,char*argv[]){
    QQuickStyle::setStyle("Basic");
    QGuiApplication app(argc,argv);
    qRegisterMetaType<MetricsPacket>("MetricsPacket");
    qmlRegisterType<UniverseRenderer>("AfterLife",1,0,"UniverseRenderer");
    qmlRegisterType<ParticleField>("AfterLife",1,0,"ParticleField");
    qmlRegisterType<CrystalField>("AfterLife",1,0,"CrystalField");
    Simulator sim;LocalAssistant assistant;assistant.setSimulator(&sim);
    QQmlApplicationEngine engine;engine.rootContext()->setContextProperty("sim",&sim);engine.rootContext()->setContextProperty("assistant",&assistant);
    QObject::connect(&engine,&QQmlApplicationEngine::objectCreationFailed,&app,[](){QCoreApplication::exit(-1);},Qt::QueuedConnection);
    engine.loadFromModule("AfterLife","Main");return QGuiApplication::exec();
}
