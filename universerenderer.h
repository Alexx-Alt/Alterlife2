#ifndef UNIVERSERENDERER_H
#define UNIVERSERENDERER_H

#include <QQuickPaintedItem>
#include <QVariantMap>
#include <QColor>
#include "simulator.h"

class UniverseRenderer : public QQuickPaintedItem {
    Q_OBJECT
    Q_PROPERTY(Simulator* simulator READ simulator WRITE setSimulator NOTIFY simulatorChanged)
    Q_PROPERTY(double zoom READ zoom WRITE setZoom NOTIFY zoomChanged)
    Q_PROPERTY(bool showOrbits READ showOrbits WRITE setShowOrbits NOTIFY showOrbitsChanged)
    Q_PROPERTY(double cameraX READ cameraX WRITE setCameraX NOTIFY cameraChanged)
    Q_PROPERTY(double cameraY READ cameraY WRITE setCameraY NOTIFY cameraChanged)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor NOTIFY backgroundColorChanged)
public:
    explicit UniverseRenderer(QQuickItem *parent=nullptr);
    void paint(QPainter *painter) override;
    Simulator*simulator()const{return m_sim;} void setSimulator(Simulator*s);
    double zoom()const{return m_zoom;} void setZoom(double z);
    bool showOrbits()const{return m_showOrbits;} void setShowOrbits(bool b);
    double cameraX()const{return m_cameraX;} double cameraY()const{return m_cameraY;} QColor backgroundColor()const{return m_backgroundColor;}
    void setCameraX(double v);void setCameraY(double v);void setBackgroundColor(const QColor&v);
    Q_INVOKABLE void panBy(double screenDx,double screenDy);
    Q_INVOKABLE void resetView();
    Q_INVOKABLE QVariantMap pickPlanet(double x,double y) const;
    Q_INVOKABLE QVariantMap planetDetail(int id) const;
signals:
    void simulatorChanged();void zoomChanged();void showOrbitsChanged();void cameraChanged();void backgroundColorChanged();
private slots:void onTick();
private:
    QPointF screenPos(int i,double scale,const QPointF&center)const;
    int indexForId(int id)const;
    void drawBackdrop(QPainter*p,const QPointF&center,double scale);
    void drawCosmicStructures(QPainter*p,const QPointF&center,double scale);
    void drawOrbits(QPainter*p,const QPointF&center,double scale);
    void drawMatter(QPainter*p,const QPointF&center,double scale);
    void drawStars(QPainter*p,const QPointF&center,double scale);
    void drawPlanets(QPainter*p,const QPointF&center,double scale);
    void drawMoons(QPainter*p,const QPointF&center,double scale);
    void drawShips(QPainter*p,const QPointF&center,double scale);
    Simulator*m_sim=nullptr;double m_zoom=1.0;bool m_showOrbits=true;double m_cameraX=0,m_cameraY=0;QColor m_backgroundColor=QColor(3,6,12);RenderSnapshot m_local;
};
#endif
