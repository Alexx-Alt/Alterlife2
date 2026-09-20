#include "crystalfield.h"
#include <QPainter>
#include <algorithm>
#include <cmath>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>

CrystalField::CrystalField(QQuickItem*p):QQuickPaintedItem(p){setAntialiasing(false);setRenderTarget(QQuickPaintedItem::FramebufferObject);connect(&m_timer,&QTimer::timeout,this,&CrystalField::tick);m_timer.setInterval(16);m_timer.start();reset();}
void CrystalField::addFrontierAround(int x,int y){for(int oy=-1;oy<=1;++oy)for(int ox=-1;ox<=1;++ox){if(!ox&&!oy)continue;int nx=x+ox,ny=y+oy;if(inside(nx,ny)&&!m_cells[idx(nx,ny)].state)m_frontier.insert(idx(nx,ny));}}
void CrystalField::reset(){m_rng.seed(m_seed);m_cells.fill(Cell{},m_grid*m_grid);m_frontier.clear();m_history.clear();m_lastRecordedGeneration=-100;const int c=m_grid/2;m_cells[idx(c,c)].state=1;m_cells[idx(c,c)].trait=float(m_rng.generateDouble()*M_PI*2);m_cells[idx(c,c)].age=0;addFrontierAround(c,c);m_generation=0;m_accumulator=0;recomputeMetrics();update();}
void CrystalField::nucleate(qreal x,qreal y){int gx=std::clamp(int(x/std::max(1.0,width())*m_grid),1,m_grid-2),gy=std::clamp(int(y/std::max(1.0,height())*m_grid),1,m_grid-2);auto&c=m_cells[idx(gx,gy)];c.state=1;c.trait=float(m_rng.generateDouble()*M_PI*2);c.age=quint16(m_generation%65535);m_frontier.remove(idx(gx,gy));addFrontierAround(gx,gy);recomputeMetrics();update();}
void CrystalField::setRunning(bool v){if(m_running==v)return;m_running=v;emit runningChanged();}
#define SET(name,var,lo,hi) void CrystalField::name(double v){v=std::clamp(v,lo,hi);if(qFuzzyCompare(var,v))return;var=v;emit parametersChanged();}
SET(setSpeed,m_speed,.02,500.) SET(setTemperature,m_temperature,0.,3.) SET(setSupersaturation,m_supersaturation,0.,3.) SET(setAnisotropy,m_anisotropy,0.,1.) SET(setMutationRate,m_mutationRate,0.,1.) SET(setBranching,m_branching,0.,1.) SET(setDiffusion,m_diffusion,0.,1.)
#undef SET
void CrystalField::setSeed(uint v){if(m_seed==v)return;m_seed=v;emit parametersChanged();reset();}
void CrystalField::setGridSize(int v){v=std::clamp(v,48,384);if(m_grid==v)return;m_grid=v;emit parametersChanged();reset();}
void CrystalField::setBackgroundColor(const QColor&v){if(m_backgroundColor==v)return;m_backgroundColor=v;emit parametersChanged();update();}
void CrystalField::tick(){if(!m_running)return;m_accumulator+=m_speed*(.4+2.5*m_supersaturation);int steps=std::min(120,int(m_accumulator));if(steps>0){m_accumulator-=steps;growSteps(steps);recomputeMetrics();update();}}
void CrystalField::skipGenerations(int n){growSteps(std::clamp(n,1,25000));recomputeMetrics();update();}
void CrystalField::growSteps(int steps){static const int dx[8]={1,-1,0,0,1,1,-1,-1},dy[8]={0,0,1,-1,1,-1,1,-1};for(int s=0;s<steps&&!m_frontier.isEmpty();++s){QVector<int>front;front.reserve(m_frontier.size());for(int ii:m_frontier)front.append(ii);std::sort(front.begin(),front.end());QVector<double>w;w.reserve(front.size());double total=0;for(int ii:front){int x=ii%m_grid,y=ii/m_grid;double sx=0,sy=0;int nb=0;float parentTrait=0;for(int k=0;k<8;++k){const auto&c=m_cells[idx(x+dx[k],y+dy[k])];if(c.state){sx-=dx[k];sy-=dy[k];parentTrait=c.trait;nb++;}}double angle=std::atan2(sy,sx);double hex=.5+.5*std::cos(6*(angle-parentTrait));double surface=std::exp(-m_temperature*.5*std::max(0,nb-1));double branch=1+m_branching*(nb==1?1.0:-.4);double weight=std::max(1e-6,(.12+.88*m_supersaturation)*surface*branch*((1-m_anisotropy)+m_anisotropy*(.15+.85*hex))*(.55+.45*m_diffusion));w.append(weight);total+=weight;}double r=m_rng.generateDouble()*total;int chosen=0;for(;chosen<w.size()-1;++chosen){r-=w[chosen];if(r<=0)break;}int ii=front[chosen],x=ii%m_grid,y=ii/m_grid;float trait=0;int parents=0;for(int k=0;k<8;++k){const auto&c=m_cells[idx(x+dx[k],y+dy[k])];if(c.state){trait+=c.trait;parents++;}}trait=parents?trait/parents:0;trait+=float((m_rng.generateDouble()-.5)*2*M_PI*m_mutationRate);auto&c=m_cells[ii];c.state=1;c.trait=trait;c.age=quint16(m_generation%65535);m_frontier.remove(ii);addFrontierAround(x,y);m_generation++;}}
void CrystalField::recomputeMetrics(){int occ=0,bins[12]={0},front=0;static const int dx[4]={1,-1,0,0},dy[4]={0,0,1,-1};for(int y=1;y<m_grid-1;++y)for(int x=1;x<m_grid-1;++x){const auto&c=m_cells[idx(x,y)];if(!c.state)continue;occ++;double a=std::fmod(std::abs(c.trait),M_PI*2)/(M_PI*2);bins[std::clamp(int(a*12),0,11)]++;for(int k=0;k<4;++k)if(!m_cells[idx(x+dx[k],y+dy[k])].state){front++;break;}}m_coverage=double(occ)/(m_grid*m_grid);double H=0;for(int b:bins)if(b){double p=double(b)/std::max(1,occ);H-=p*std::log2(p);}m_diversity=H/std::log2(12.0);m_frontierEnergy=double(front)/std::max(1,occ);if(m_generation==0||m_generation-m_lastRecordedGeneration>=20){m_history.append(Sample{m_generation,m_coverage,m_diversity,m_frontierEnergy});if(m_history.size()>5000)m_history.remove(0,m_history.size()-5000);m_lastRecordedGeneration=m_generation;}emit metricsChanged();}
void CrystalField::paint(QPainter*p){p->fillRect(boundingRect(),m_backgroundColor);const double cw=width()/m_grid,ch=height()/m_grid;for(int y=0;y<m_grid;++y)for(int x=0;x<m_grid;++x){const auto&c=m_cells[idx(x,y)];if(!c.state)continue;double trait=.5+.5*std::sin(c.trait);int b=std::clamp(150+int(trait*95),0,255),g=std::clamp(180+int((1-trait)*60),0,255);QColor col(115,g,b,225);p->fillRect(QRectF(x*cw,y*ch,std::max(1.0,cw+.35),std::max(1.0,ch+.35)),col);}}


bool CrystalField::exportReport(const QString &path) const
{
    QJsonObject root;
    root["format"] = "afterlife.crystal-report.v1";
    QJsonObject params;
    params["seed"] = static_cast<qint64>(m_seed);
    params["temperature"] = m_temperature;
    params["supersaturation"] = m_supersaturation;
    params["anisotropy"] = m_anisotropy;
    params["mutationRate"] = m_mutationRate;
    params["branching"] = m_branching;
    params["diffusion"] = m_diffusion;
    params["grid"] = m_grid;
    root["parameters"] = params;

    QJsonObject summary;
    summary["generation"] = m_generation;
    summary["coverage"] = m_coverage;
    summary["diversity"] = m_diversity;
    summary["frontierEnergy"] = m_frontierEnergy;
    summary["text"] = QString("Кристалл: поколение %1, покрытие %2%, разнообразие %3, энергия фронта %4.")
        .arg(m_generation).arg(m_coverage*100.0,0,'f',2).arg(m_diversity,0,'f',3).arg(m_frontierEnergy,0,'f',3);
    root["summary"] = summary;

    QJsonArray history;
    for (const auto &s : m_history) {
        QJsonObject o; o["generation"]=s.generation; o["coverage"]=s.coverage;
        o["diversity"]=s.diversity; o["frontierEnergy"]=s.frontierEnergy; history.append(o);
    }
    root["history"] = history;

    // Sparse final state: only occupied lattice cells are stored.
    QJsonArray state;
    for (int y=0;y<m_grid;++y) for (int x=0;x<m_grid;++x) {
        const auto &c=m_cells[idx(x,y)]; if(!c.state) continue;
        QJsonObject o; o["x"]=x; o["y"]=y; o["trait"]=c.trait; o["bornAt"]=int(c.age); state.append(o);
    }
    root["finalState"] = state;
    QFileInfo fi(path); QString resolved=fi.isAbsolute()?fi.absoluteFilePath():QString();
    if(resolved.isEmpty()){QString base=QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);if(base.isEmpty())base=QDir::homePath();QDir dir(base);dir.mkpath("AfterLife");resolved=dir.filePath("AfterLife/"+path);}
    QFile f(resolved); if(!f.open(QIODevice::WriteOnly)) return false;
    f.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    return true;
}
