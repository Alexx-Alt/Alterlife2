#include "particlefield.h"

#include <QPainter>
#include <QRandomGenerator>
#include <QDateTime>
#include <QImage>
#include <QFont>
#include <algorithm>
#include <cmath>

namespace {
qreal rr(qreal a, qreal b) {
    return a + (b - a) * QRandomGenerator::global()->generateDouble();
}
}

ParticleField::ParticleField(QQuickItem *p)
    : QQuickPaintedItem(p)
{
    setAntialiasing(true);
    setRenderTarget(QQuickPaintedItem::FramebufferObject);
    m_timer.setInterval(16);
    connect(&m_timer, &QTimer::timeout, this, &ParticleField::tick);
    m_timer.start();
}

void ParticleField::geometryChange(const QRectF &n, const QRectF &o)
{
    QQuickPaintedItem::geometryChange(n, o);
    if (n.size() != o.size()) {
        initParticles();
        rebuildTargets();
    }
}

void ParticleField::initParticles()
{
    if (width() <= 0 || height() <= 0)
        return;

    m_particles.resize(m_density);
    for (auto &p : m_particles) {
        p.pos = QPointF(rr(0, width()), rr(0, height()));
        p.vel = QPointF(rr(-.15, .15), rr(-.15, .15));
        p.size = rr(.7, 1.8);
        p.brightness = rr(.35, 1.0);
        p.wobblePhase = rr(0, 6.283);
        p.wobbleAmp = rr(.5, 2.8);
    }
    rebuildTargets();
}

void ParticleField::rebuildTargets()
{
    if (m_shape == Outline)
        rebuildOutlineTargets();
    else if (m_shape == Cube)
        rebuildCubeTargets();
    else if (m_shape == Text)
        rebuildTextTargets();
    else if (m_shape == Square)
        rebuildSquareTargets();
}

void ParticleField::rebuildOutlineTargets()
{
    if (m_particles.isEmpty())
        return;

    const qreal x = m_outlineX;
    const qreal y = m_outlineY;
    const qreal w = m_outlineW;
    const qreal h = m_outlineH;
    const qreal r = std::min(m_cornerRadius, std::min(w, h) / 2);
    const qreal per = 2 * w + 2 * h - 8 * r + 2 * M_PI * r;

    for (int i = 0; i < m_particles.size(); ++i) {
        qreal s = per * i / m_particles.size();
        QPointF q;

        // rounded perimeter approximation
        if (s < w - 2 * r) {
            q = { x + r + s, y };
        } else if ((s -= w - 2 * r) < M_PI * r / 2) {
            q = { x + w - r + std::cos(-M_PI / 2 + s / r) * r,
                 y + r + std::sin(-M_PI / 2 + s / r) * r };
        } else if ((s -= M_PI * r / 2) < h - 2 * r) {
            q = { x + w, y + r + s };
        } else if ((s -= h - 2 * r) < M_PI * r / 2) {
            q = { x + w - r + std::cos(s / r) * r,
                 y + h - r + std::sin(s / r) * r };
        } else if ((s -= M_PI * r / 2) < w - 2 * r) {
            q = { x + w - r - s, y + h };
        } else if ((s -= w - 2 * r) < M_PI * r / 2) {
            q = { x + r + std::cos(M_PI / 2 + s / r) * r,
                 y + h - r + std::sin(M_PI / 2 + s / r) * r };
        } else if ((s -= M_PI * r / 2) < h - 2 * r) {
            q = { x, y + h - r - s };
        } else {
            s -= h - 2 * r;
            q = { x + r + std::cos(M_PI + s / r) * r,
                 y + r + std::sin(M_PI + s / r) * r };
        }

        m_particles[i].target2D = q;
    }
}

void ParticleField::rebuildCubeTargets()
{
    static const QVector<QVector3D> v = {
        {-1, -1, -1}, {1, -1, -1}, {1, 1, -1}, {-1, 1, -1},
        {-1, -1, 1},  {1, -1, 1},  {1, 1, 1},  {-1, 1, 1}
    };
    static const QVector<QPair<int, int>> e = {
        {0, 1}, {1, 2}, {2, 3}, {3, 0},
        {4, 5}, {5, 6}, {6, 7}, {7, 4},
        {0, 4}, {1, 5}, {2, 6}, {3, 7}
    };

    m_cubeTargets.clear();
    for (int k = 0; k < m_particles.size(); ++k) {
        auto ed = e[k % e.size()];
        qreal t = rr(0, 1);
        m_cubeTargets.append(v[ed.first] * (1 - t) + v[ed.second] * t);
    }
    for (int i = 0; i < m_particles.size(); ++i)
        m_particles[i].target3D = m_cubeTargets[i];
}

void ParticleField::rebuildTextTargets()
{
    QImage img(1200, 260, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::transparent);

    QPainter p(&img);
    QFont f("Sans Serif", 150, QFont::DemiBold);
    f.setLetterSpacing(QFont::AbsoluteSpacing, 8);
    p.setFont(f);
    p.setPen(Qt::white);
    p.drawText(img.rect(), Qt::AlignCenter, m_text);
    p.end();

    QVector<QPointF> pts;
    for (int y = 2; y < img.height(); y += 3)
        for (int x = 2; x < img.width(); x += 3)
            if (qAlpha(img.pixel(x, y)) > 80)
                pts.append(QPointF(x - img.width() / 2, y - img.height() / 2));

    if (pts.isEmpty())
        return;

    const qreal scale = m_shapeSize / 600.0;
    for (int i = 0; i < m_particles.size(); ++i)
        m_particles[i].target2D = pts[(i * 37) % pts.size()] * scale;
}

void ParticleField::rebuildSquareTargets()
{
    for (int i = 0; i < m_particles.size(); ++i) {
        qreal a = rr(0, 4), s = m_shapeSize;
        QPointF q;
        if (a < 1)
            q = { -s / 2 + a * s, -s / 2 };
        else if (a < 2)
            q = { s / 2, -s / 2 + (a - 1) * s };
        else if (a < 3)
            q = { s / 2 - (a - 2) * s, s / 2 };
        else
            q = { -s / 2, s / 2 - (a - 3) * s };

        m_particles[i].target2D = q;
    }
}

void ParticleField::setShape(int v)
{
    if (m_shape == v)
        return;
    m_shape = v;
    rebuildTargets();
    emit shapeChanged();
}

void ParticleField::setDensity(int v)
{
    v = std::clamp(v, 20, 6000);
    if (m_density == v)
        return;
    m_density = v;
    initParticles();
    emit densityChanged();
}

#define SETR(fn, var, sig)                          \
void ParticleField::fn(qreal v) {               \
        if (qFuzzyCompare(var, v)) return;          \
        var = v;                                    \
        sig;                                        \
}

SETR(setShapeX, m_shapeX, emit shapeChanged())
SETR(setShapeY, m_shapeY, emit shapeChanged())

void ParticleField::setShapeSize(qreal v)
{
    if (qFuzzyCompare(m_shapeSize, v))
        return;
    m_shapeSize = v;
    rebuildTargets();
    emit shapeChanged();
}

void ParticleField::setText(const QString &v)
{
    if (m_text == v)
        return;
    m_text = v;
    if (m_shape == Text)
        rebuildTextTargets();
    emit textChanged();
}

void ParticleField::setColor(const QColor &v)
{
    if (m_color == v)
        return;
    m_color = v;
    emit colorChanged();
    update();
}

SETR(setMouseX, m_mouseX, emit interactionChanged())
SETR(setMouseY, m_mouseY, emit interactionChanged())
SETR(setInteractionRadius, m_interactionRadius, emit interactionChanged())
SETR(setRepelStrength, m_repelStrength, emit interactionChanged())

void ParticleField::setOutlineX(qreal v)
{
    m_outlineX = v;
    if (m_shape == Outline) rebuildOutlineTargets();
    emit shapeChanged();
}
void ParticleField::setOutlineY(qreal v)
{
    m_outlineY = v;
    if (m_shape == Outline) rebuildOutlineTargets();
    emit shapeChanged();
}
void ParticleField::setOutlineW(qreal v)
{
    m_outlineW = v;
    if (m_shape == Outline) rebuildOutlineTargets();
    emit shapeChanged();
}
void ParticleField::setOutlineH(qreal v)
{
    m_outlineH = v;
    if (m_shape == Outline) rebuildOutlineTargets();
    emit shapeChanged();
}
void ParticleField::setCornerRadius(qreal v)
{
    m_cornerRadius = v;
    if (m_shape == Outline) rebuildOutlineTargets();
    emit shapeChanged();
}

#undef SETR

void ParticleField::tick()
{
    if (width() <= 0 || height() <= 0)
        return;
    if (m_particles.isEmpty())
        initParticles();

    const qreal cx = m_shapeX >= 0 ? m_shapeX : width() / 2;
    const qreal cy = m_shapeY >= 0 ? m_shapeY : height() / 2;
    const double tm = QDateTime::currentMSecsSinceEpoch() / 1000.0;

    m_cubeAngleX += .006;
    m_cubeAngleY += .009;

    for (auto &p : m_particles) {
        QPointF target;
        bool hasTarget = false;

        if (m_shape == Cube) {
            qreal ax = m_cubeAngleX, ay = m_cubeAngleY;
            qreal cx1 = std::cos(ax), sx = std::sin(ax);
            qreal cy1 = std::cos(ay), sy = std::sin(ay);
            qreal x = p.target3D.x() * cy1 + p.target3D.z() * sy;
            qreal z = -p.target3D.x() * sy + p.target3D.z() * cy1;
            qreal y = p.target3D.y();
            qreal yy = y * cx1 - z * sx;
            qreal zz = y * sx + z * cx1;
            qreal pr = 1 / (1 + zz * .25);
            target = QPointF(cx, cy) + QPointF(x, yy) * m_shapeSize * pr;
            hasTarget = true;
        } else if (m_shape == Outline) {
            target = p.target2D;
            hasTarget = true;
        } else if (m_shape == Text || m_shape == Square) {
            target = QPointF(cx, cy) + p.target2D;
            hasTarget = true;
        } else if (m_shape == Galaxy) {
            QPointF d = QPointF(cx, cy) - p.pos;
            qreal dist = std::hypot(d.x(), d.y()) + 1e-6;
            QPointF n = d / dist, t(-n.y(), n.x());
            p.vel += n * (.055 * m_shapeSize / (dist + 35)) + t * .035;
            p.vel *= .997;
        } else {
            p.vel += (QPointF(width() / 2, height() / 2) - p.pos) * .00008;
            p.vel += QPointF(std::sin(tm * .6 + p.wobblePhase),
                             std::cos(tm * .55 + p.wobblePhase)) * .002 * p.wobbleAmp;
            p.vel *= .997;
        }

        if (hasTarget) {
            target += QPointF(std::sin(tm * 1.7 + p.wobblePhase),
                              std::cos(tm * 1.4 + p.wobblePhase)) * p.wobbleAmp * .45;
            p.vel += (target - p.pos) * .055;
            p.vel *= .86;
        }

        QPointF md = p.pos - QPointF(m_mouseX, m_mouseY);
        qreal mlen = std::hypot(md.x(), md.y()) + 1e-6;
        if (mlen < m_interactionRadius) {
            qreal k = (1 - mlen / m_interactionRadius) * m_repelStrength;
            p.vel += md / mlen * k;
        }

        p.pos += p.vel;

        if (m_shape == FreeForm || m_shape == Galaxy) {
            if (p.pos.x() < 0) p.pos.setX(width());
            if (p.pos.x() > width()) p.pos.setX(0);
            if (p.pos.y() < 0) p.pos.setY(height());
            if (p.pos.y() > height()) p.pos.setY(0);
        }
    }

    update();
}

void ParticleField::paint(QPainter *p)
{
    p->setPen(Qt::NoPen);
    for (const auto &x : m_particles) {
        QColor c = m_color;
        c.setAlpha(int(70 + x.brightness * 175));
        p->setBrush(c);
        p->drawEllipse(x.pos, x.size, x.size);
    }
}