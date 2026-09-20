#include "universerenderer.h"
#include <QPainter>
#include <QRadialGradient>
#include <QLinearGradient>
#include <QMutexLocker>
#include <algorithm>
#include <cmath>

UniverseRenderer::UniverseRenderer(QQuickItem *parent):QQuickPaintedItem(parent){setAntialiasing(true);setRenderTarget(QQuickPaintedItem::FramebufferObject);}
void UniverseRenderer::setSimulator(Simulator*s){if(m_sim==s)return;if(m_sim)disconnect(m_sim,nullptr,this,nullptr);m_sim=s;if(m_sim)connect(m_sim,&Simulator::tick,this,&UniverseRenderer::onTick);emit simulatorChanged();}
void UniverseRenderer::setZoom(double z){z=std::clamp(z,0.12,18.0);if(qFuzzyCompare(z,m_zoom))return;m_zoom=z;emit zoomChanged();update();}
void UniverseRenderer::setShowOrbits(bool b){if(m_showOrbits==b)return;m_showOrbits=b;emit showOrbitsChanged();update();}
void UniverseRenderer::setCameraX(double v){if(qFuzzyCompare(v,m_cameraX))return;m_cameraX=v;emit cameraChanged();update();}
void UniverseRenderer::setCameraY(double v){if(qFuzzyCompare(v,m_cameraY))return;m_cameraY=v;emit cameraChanged();update();}
void UniverseRenderer::setBackgroundColor(const QColor&v){if(m_backgroundColor==v)return;m_backgroundColor=v;emit backgroundColorChanged();update();}
void UniverseRenderer::resetView(){m_cameraX=m_cameraY=0;m_zoom=1;emit cameraChanged();emit zoomChanged();update();}
void UniverseRenderer::panBy(double dx,double dy){const double scale=(std::min(width(),height())/(2.0*std::max(1.0,m_local.worldRadius)))*m_zoom;m_cameraX-=dx/std::max(1e-6,scale);m_cameraY-=dy/std::max(1e-6,scale);emit cameraChanged();update();}

void UniverseRenderer::onTick(){if(!m_sim||!m_sim->worker())return;RenderSnapshot&src=m_sim->worker()->snapshot();QMutexLocker lock(&src.mutex);
#define CP(x) m_local.x=src.x
    CP(posX);CP(posY);CP(size);CP(energy);CP(type);CP(direction);CP(habitability);CP(temperature);CP(mass);CP(orbitRadius);CP(id);CP(parentId);CP(galaxyId);CP(population);CP(generation);CP(level);
#undef CP
    m_local.time=src.time;m_local.worldRadius=src.worldRadius;m_local.epoch=src.epoch;m_local.totalCount=src.totalCount;update();}

QPointF UniverseRenderer::screenPos(int i,double scale,const QPointF&center)const{return center+QPointF(m_local.posX[i]-m_cameraX,m_local.posY[i]-m_cameraY)*scale;}
int UniverseRenderer::indexForId(int id)const{for(int i=0;i<m_local.id.size();++i)if(m_local.id[i]==id)return i;return -1;}

void UniverseRenderer::paint(QPainter*p){p->setRenderHint(QPainter::Antialiasing,true);QLinearGradient bg(0,0,width(),height());bg.setColorAt(0,m_backgroundColor.lighter(108));bg.setColorAt(1,m_backgroundColor.darker(112));p->fillRect(boundingRect(),bg);if(!m_sim)return;
    const QPointF center(width()/2,height()/2);const double scale=(std::min(width(),height())/(2.0*std::max(1.0,m_local.worldRadius)))*m_zoom;
    drawBackdrop(p,center,scale);drawCosmicStructures(p,center,scale);if(m_showOrbits)drawOrbits(p,center,scale);drawMatter(p,center,scale);drawStars(p,center,scale);drawPlanets(p,center,scale);drawMoons(p,center,scale);drawShips(p,center,scale);
}

void UniverseRenderer::drawBackdrop(QPainter*p,const QPointF&center,double scale){Q_UNUSED(center);Q_UNUSED(scale);p->setPen(QPen(QColor(90,130,180,12),1));const int gap=80;const double ox=std::fmod(-m_cameraX*scale,double(gap));const double oy=std::fmod(-m_cameraY*scale,double(gap));for(double x=ox;x<width();x+=gap)p->drawLine(QPointF(x,0),QPointF(x,height()));for(double y=oy;y<height();y+=gap)p->drawLine(QPointF(0,y),QPointF(width(),y));}

void UniverseRenderer::drawCosmicStructures(QPainter*p,const QPointF&center,double scale){for(int i=0;i<m_local.totalCount;++i){const int t=int(m_local.type[i]);if(t!=14&&t!=15)continue;const QPointF pos=screenPos(i,scale,center);const double r=(t==15?m_local.worldRadius*0.55:70.0+std::sqrt(std::max(1.0f,m_local.mass[i]))*35.0)*scale;QRadialGradient g(pos,r);if(t==15){g.setColorAt(0,QColor(100,120,255,8));g.setColorAt(1,QColor(60,80,180,0));}else{g.setColorAt(0,QColor(80,150,255,20));g.setColorAt(0.45,QColor(110,80,220,10));g.setColorAt(1,QColor(20,40,100,0));}p->setPen(Qt::NoPen);p->setBrush(g);p->drawEllipse(pos,r,r);}}

void UniverseRenderer::drawOrbits(QPainter*p,const QPointF&center,double scale){p->setBrush(Qt::NoBrush);for(int i=0;i<m_local.totalCount;++i){const int t=int(m_local.type[i]);if(t!=11&&t!=16)continue;const int parent=indexForId(m_local.parentId[i]);if(parent<0)continue;const QPointF c=screenPos(parent,scale,center);const double r=m_local.orbitRadius[i]*scale;p->setPen(QPen(t==16?QColor(150,175,205,28):QColor(100,145,195,40),std::max(0.45,(t==16?0.55:0.8)*m_zoom),Qt::DashLine));p->drawEllipse(c,r,r);}}

void UniverseRenderer::drawMatter(QPainter*p,const QPointF&center,double scale){Q_UNUSED(scale);p->setPen(Qt::NoPen);for(int i=0;i<m_local.totalCount;++i){const int t=int(m_local.type[i]);if(t>=10)continue;const QPointF pos=screenPos(i,(std::min(width(),height())/(2.0*std::max(1.0,m_local.worldRadius)))*m_zoom,center);QColor c;double r=1.5;bool glow=false;
        switch(t){case 0:c=QColor(116,142,182,150);r=1.2;break;case 1:c=QColor(100,190,255,180);r=1.8;break;case 2:c=QColor(106,245,170);r=3;glow=true;break;case 3:c=QColor(255,197,104);r=4.2;glow=true;break;case 4:c=QColor(240,103,205);r=5.4;glow=true;break;default:continue;}
        if(glow){QRadialGradient g(pos,r*3);g.setColorAt(0,QColor(c.red(),c.green(),c.blue(),120));g.setColorAt(1,QColor(c.red(),c.green(),c.blue(),0));p->setBrush(g);p->drawEllipse(pos,r*3,r*3);}p->setBrush(c);p->drawEllipse(pos,r,r);}}

void UniverseRenderer::drawStars(QPainter*p,const QPointF&center,double scale){for(int i=0;i<m_local.totalCount;++i){if(int(m_local.type[i])!=10)continue;const QPointF pos=screenPos(i,scale,center);const double core=std::clamp(5.0+double(m_local.mass[i])*2.8,5.0,12.0);QRadialGradient halo(pos,core*4.5);halo.setColorAt(0,QColor(255,244,195,180));halo.setColorAt(.25,QColor(255,180,70,70));halo.setColorAt(1,QColor(255,120,20,0));p->setBrush(halo);p->setPen(Qt::NoPen);p->drawEllipse(pos,core*4.5,core*4.5);QRadialGradient body(pos,core);body.setColorAt(0,QColor(255,255,245));body.setColorAt(.55,QColor(255,224,145));body.setColorAt(1,QColor(255,156,65));p->setBrush(body);p->drawEllipse(pos,core,core);}}

void UniverseRenderer::drawPlanets(QPainter*p,const QPointF&center,double scale){for(int i=0;i<m_local.totalCount;++i){if(int(m_local.type[i])!=11)continue;const QPointF pos=screenPos(i,scale,center);const double h=m_local.habitability[i],temp=m_local.temperature[i];const double r=std::clamp(3.4+double(m_local.mass[i])*.65,3.4,8.5);QColor base;if(h>.55)base=QColor(88,200,160);else if(temp>1.22)base=QColor(214,128,78);else if(temp<.72)base=QColor(126,172,215);else base=QColor(135,151,178);QRadialGradient g(pos-QPointF(r*.25,r*.25),r*1.3);g.setColorAt(0,base.lighter(155));g.setColorAt(.62,base);g.setColorAt(1,base.darker(180));p->setPen(QPen(QColor(220,235,255,80),.7));p->setBrush(g);p->drawEllipse(pos,r,r);if(h>.32){p->setBrush(Qt::NoBrush);p->setPen(QPen(QColor(100,255,190,80),1));p->drawEllipse(pos,r+3,r+3);}}}

void UniverseRenderer::drawMoons(QPainter*p,const QPointF&center,double scale){for(int i=0;i<m_local.totalCount;++i){if(int(m_local.type[i])!=16)continue;const QPointF pos=screenPos(i,scale,center);const double r=std::clamp(1.2+double(m_local.mass[i])*4.0,1.2,2.8);p->setPen(Qt::NoPen);p->setBrush(QColor(185,198,215,210));p->drawEllipse(pos,r,r);}}

void UniverseRenderer::drawShips(QPainter*p,const QPointF&center,double scale){for(int i=0;i<m_local.totalCount;++i){if(int(m_local.type[i])!=12)continue;const QPointF pos=screenPos(i,scale,center);const double d=m_local.direction[i];const QPointF f(std::cos(d),std::sin(d)),n(-f.y(),f.x());QPolygonF poly;poly<<pos+f*7<<pos-f*4+n*3<<pos-f*2<<pos-f*4-n*3;p->setPen(Qt::NoPen);p->setBrush(QColor(205,225,255));p->drawPolygon(poly);p->setPen(QPen(QColor(100,170,255,90),1));p->drawLine(pos-f*3,pos-f*9);}}

QVariantMap UniverseRenderer::planetDetail(int id)const{QVariantMap r;const int i=indexForId(id);if(i<0||int(m_local.type[i])!=11)return r;r["id"]=id;r["habitability"]=m_local.habitability[i];r["temperature"]=m_local.temperature[i];r["mass"]=m_local.mass[i];r["orbitRadius"]=m_local.orbitRadius[i];r["population"]=m_local.population[i];r["galaxyId"]=m_local.galaxyId[i];int cells=0,colonies=0,minds=0,ships=0,moons=0,maxGen=0;for(int j=0;j<m_local.totalCount;++j){if(m_local.parentId[j]==id){int t=int(m_local.type[j]);if(t==2)cells++;else if(t==3)colonies++;else if(t==4)minds++;else if(t==16)moons++;maxGen=std::max(maxGen,m_local.generation[j]);}if(int(m_local.type[j])==12){const double d=std::hypot(double(m_local.posX[j]-m_local.posX[i]),double(m_local.posY[j]-m_local.posY[i]));if(d<90)ships++;}}r["cells"]=cells;r["colonies"]=colonies;r["minds"]=minds;r["ships"]=ships;r["moons"]=moons;r["maxGeneration"]=maxGen;r["biosphere"]=std::clamp((cells*.02+colonies*.08+minds*.18),0.0,1.0);return r;}
QVariantMap UniverseRenderer::pickPlanet(double x,double y)const{const QPointF center(width()/2,height()/2);const double scale=(std::min(width(),height())/(2.0*std::max(1.0,m_local.worldRadius)))*m_zoom;int best=-1;double bd=18.0;for(int i=0;i<m_local.totalCount;++i){if(int(m_local.type[i])!=11)continue;const QPointF p=screenPos(i,scale,center);const double d=std::hypot(p.x()-x,p.y()-y);if(d<bd){bd=d;best=i;}}return best>=0?planetDetail(m_local.id[best]):QVariantMap();}
