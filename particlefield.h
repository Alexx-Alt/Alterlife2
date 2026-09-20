#ifndef PARTICLEFIELD_H
#define PARTICLEFIELD_H

#include <QQuickPaintedItem>
#include <QTimer>
#include <QVector>
#include <QPointF>
#include <QVector3D>
#include <QColor>

class ParticleField : public QQuickPaintedItem {
    Q_OBJECT
    Q_PROPERTY(int shape READ shape WRITE setShape NOTIFY shapeChanged)
    Q_PROPERTY(int density READ density WRITE setDensity NOTIFY densityChanged)
    Q_PROPERTY(qreal shapeX READ shapeX WRITE setShapeX NOTIFY shapeChanged)
    Q_PROPERTY(qreal shapeY READ shapeY WRITE setShapeY NOTIFY shapeChanged)
    Q_PROPERTY(qreal shapeSize READ shapeSize WRITE setShapeSize NOTIFY shapeChanged)
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY(qreal mouseX READ mouseX WRITE setMouseX NOTIFY interactionChanged)
    Q_PROPERTY(qreal mouseY READ mouseY WRITE setMouseY NOTIFY interactionChanged)
    Q_PROPERTY(qreal interactionRadius READ interactionRadius WRITE setInteractionRadius NOTIFY interactionChanged)
    Q_PROPERTY(qreal repelStrength READ repelStrength WRITE setRepelStrength NOTIFY interactionChanged)
    Q_PROPERTY(qreal outlineX READ outlineX WRITE setOutlineX NOTIFY shapeChanged)
    Q_PROPERTY(qreal outlineY READ outlineY WRITE setOutlineY NOTIFY shapeChanged)
    Q_PROPERTY(qreal outlineW READ outlineW WRITE setOutlineW NOTIFY shapeChanged)
    Q_PROPERTY(qreal outlineH READ outlineH WRITE setOutlineH NOTIFY shapeChanged)
    Q_PROPERTY(qreal cornerRadius READ cornerRadius WRITE setCornerRadius NOTIFY shapeChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
public:
    enum Shape{FreeForm=0,Outline=1,Cube=2,Galaxy=3,Text=4,Square=5};Q_ENUM(Shape)
    explicit ParticleField(QQuickItem *parent=nullptr);void paint(QPainter*painter)override;
    int shape()const{return m_shape;}int density()const{return m_density;}qreal shapeX()const{return m_shapeX;}qreal shapeY()const{return m_shapeY;}qreal shapeSize()const{return m_shapeSize;}
    QString text()const{return m_text;}qreal mouseX()const{return m_mouseX;}qreal mouseY()const{return m_mouseY;}qreal interactionRadius()const{return m_interactionRadius;}qreal repelStrength()const{return m_repelStrength;}
    QColor color()const{return m_color;}qreal outlineX()const{return m_outlineX;}qreal outlineY()const{return m_outlineY;}qreal outlineW()const{return m_outlineW;}qreal outlineH()const{return m_outlineH;}qreal cornerRadius()const{return m_cornerRadius;}
public slots:
    void setShape(int);void setDensity(int);void setShapeX(qreal);void setShapeY(qreal);void setShapeSize(qreal);void setText(const QString&);
    void setMouseX(qreal);void setMouseY(qreal);void setInteractionRadius(qreal);void setRepelStrength(qreal);void setColor(const QColor&);
    void setOutlineX(qreal);void setOutlineY(qreal);void setOutlineW(qreal);void setOutlineH(qreal);void setCornerRadius(qreal);
signals:void shapeChanged();void densityChanged();void textChanged();void interactionChanged();void colorChanged();
protected:void geometryChange(const QRectF&n,const QRectF&o)override;
private slots:void tick();
private:
    struct Particle{QPointF pos,vel,target2D;QVector3D target3D;qreal size=1,brightness=1,wobblePhase=0,wobbleAmp=1;};
    void initParticles();void rebuildTargets();void rebuildOutlineTargets();void rebuildCubeTargets();void rebuildTextTargets();void rebuildSquareTargets();
    QVector<Particle>m_particles;QTimer m_timer;int m_shape=FreeForm,m_density=500;qreal m_shapeX=-1,m_shapeY=-1,m_shapeSize=140;
    QString m_text="AFTERLIFE";QColor m_color=QColor(192,218,255);qreal m_mouseX=-9999,m_mouseY=-9999,m_interactionRadius=110,m_repelStrength=0.75;
    qreal m_outlineX=0,m_outlineY=0,m_outlineW=180,m_outlineH=44,m_cornerRadius=22;QVector<QVector3D>m_cubeTargets;qreal m_cubeAngleX=0,m_cubeAngleY=0;
};
#endif
