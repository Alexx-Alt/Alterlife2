#ifndef CRYSTALFIELD_H
#define CRYSTALFIELD_H
#include <QQuickPaintedItem>
#include <QTimer>
#include <QVector>
#include <QRandomGenerator>
#include <QSet>
#include <QColor>
#include <QString>

class CrystalField:public QQuickPaintedItem{
    Q_OBJECT
    Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged)
    Q_PROPERTY(double speed READ speed WRITE setSpeed NOTIFY parametersChanged)
    Q_PROPERTY(double temperature READ temperature WRITE setTemperature NOTIFY parametersChanged)
    Q_PROPERTY(double supersaturation READ supersaturation WRITE setSupersaturation NOTIFY parametersChanged)
    Q_PROPERTY(double anisotropy READ anisotropy WRITE setAnisotropy NOTIFY parametersChanged)
    Q_PROPERTY(double mutationRate READ mutationRate WRITE setMutationRate NOTIFY parametersChanged)
    Q_PROPERTY(double branching READ branching WRITE setBranching NOTIFY parametersChanged)
    Q_PROPERTY(double diffusion READ diffusion WRITE setDiffusion NOTIFY parametersChanged)
    Q_PROPERTY(uint seed READ seed WRITE setSeed NOTIFY parametersChanged)
    Q_PROPERTY(int gridSize READ gridSize WRITE setGridSize NOTIFY parametersChanged)
    Q_PROPERTY(int generation READ generation NOTIFY metricsChanged)
    Q_PROPERTY(double coverage READ coverage NOTIFY metricsChanged)
    Q_PROPERTY(double diversity READ diversity NOTIFY metricsChanged)
    Q_PROPERTY(double frontierEnergy READ frontierEnergy NOTIFY metricsChanged)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor NOTIFY parametersChanged)
public:
    explicit CrystalField(QQuickItem*parent=nullptr);void paint(QPainter*)override;
    bool running()const{return m_running;}double speed()const{return m_speed;}double temperature()const{return m_temperature;}double supersaturation()const{return m_supersaturation;}double anisotropy()const{return m_anisotropy;}double mutationRate()const{return m_mutationRate;}double branching()const{return m_branching;}double diffusion()const{return m_diffusion;}uint seed()const{return m_seed;}int gridSize()const{return m_grid;}int generation()const{return m_generation;}double coverage()const{return m_coverage;}double diversity()const{return m_diversity;}double frontierEnergy()const{return m_frontierEnergy;}QColor backgroundColor()const{return m_backgroundColor;}
    Q_INVOKABLE void reset();Q_INVOKABLE void nucleate(qreal x,qreal y);Q_INVOKABLE void skipGenerations(int n);Q_INVOKABLE bool exportReport(const QString &path) const;
public slots:void setRunning(bool);void setSpeed(double);void setTemperature(double);void setSupersaturation(double);void setAnisotropy(double);void setMutationRate(double);void setBranching(double);void setDiffusion(double);void setSeed(uint);void setGridSize(int);void setBackgroundColor(const QColor&);
signals:void runningChanged();void parametersChanged();void metricsChanged();
private slots:void tick();
private:
    struct Cell{quint8 state=0;float trait=0;quint16 age=0;};
    struct Sample{int generation=0;double coverage=0,diversity=0,frontierEnergy=0;};
    int idx(int x,int y)const{return y*m_grid+x;}bool inside(int x,int y)const{return x>0&&y>0&&x<m_grid-1&&y<m_grid-1;}void growSteps(int n);void recomputeMetrics();void addFrontierAround(int x,int y);
    int m_grid=176;QVector<Cell>m_cells;QSet<int>m_frontier;QVector<Sample>m_history;QTimer m_timer;QRandomGenerator m_rng;bool m_running=false;double m_speed=1,m_temperature=.42,m_supersaturation=.72,m_anisotropy=.64,m_mutationRate=.06,m_branching=.35,m_diffusion=.58;uint m_seed=1337;int m_generation=0;double m_coverage=0,m_diversity=0,m_frontierEnergy=0;double m_accumulator=0;int m_lastRecordedGeneration=-100;QColor m_backgroundColor=QColor(3,7,14);
};
#endif
