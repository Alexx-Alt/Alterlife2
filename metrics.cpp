#include "metrics.h"
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>

QJsonArray Metrics::toJsonArray() const
{
    QJsonArray arr;
    for(const auto&s:m_history){
        QJsonObject o;
#define P(x) o[#x]=s.x
        P(time);P(epoch);P(quantCount);P(particleCount);P(cellCount);P(colonyCount);P(mindCount);
        P(starCount);P(planetCount);P(habitablePlanetCount);P(shipCount);P(galaxyCount);P(superclusterCount);P(moonCount);
        P(entropy);P(complexity);P(evolutionRate);P(avgGeneration);P(avgLevel);P(power);P(responseLatency);
        P(efficiency);P(decisionIndex);
#undef P
        arr.append(o);
    }
    return arr;
}

bool Metrics::exportCsv(const QString &path) const
{
    QFile f(path); if(!f.open(QIODevice::WriteOnly|QIODevice::Text))return false; QTextStream out(&f);
    out << "time,epoch,quants,particles,cells,colonies,minds,stars,planets,habitable,ships,galaxies,superclusters,moons,entropy,complexity,evolutionRate,avgGeneration,avgLevel,power,responseLatency,efficiency,decisionIndex\n";
    for(const auto&s:m_history){
        out<<s.time<<','<<s.epoch<<','<<s.quantCount<<','<<s.particleCount<<','<<s.cellCount<<','<<s.colonyCount<<','<<s.mindCount<<','
           <<s.starCount<<','<<s.planetCount<<','<<s.habitablePlanetCount<<','<<s.shipCount<<','<<s.galaxyCount<<','<<s.superclusterCount<<','<<s.moonCount<<','
           <<s.entropy<<','<<s.complexity<<','<<s.evolutionRate<<','<<s.avgGeneration<<','<<s.avgLevel<<','<<s.power<<','<<s.responseLatency<<','
           <<s.efficiency<<','<<s.decisionIndex<<'\n';
    }
    return true;
}

bool Metrics::exportJson(const QString &path) const
{
    QJsonObject root;root["format"]="afterlife.metrics.v2";root["samples"]=toJsonArray();
    QFile f(path);if(!f.open(QIODevice::WriteOnly))return false;f.write(QJsonDocument(root).toJson(QJsonDocument::Indented));return true;
}
