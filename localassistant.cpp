#include "localassistant.h"
#include "simulator.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QRegularExpression>

LocalAssistant::LocalAssistant(QObject *p)
    : QObject(p)
{
}

void LocalAssistant::setStatus(const QString &s)
{
    if (m_status == s)
        return;
    m_status = s;
    emit statusChanged();
}

void LocalAssistant::setBusy(bool b)
{
    if (m_busy == b)
        return;
    m_busy = b;
    emit busyChanged();
}

QString LocalAssistant::applyControlIntent(const QString &p)
{
    if (!m_sim)
        return {};

    const QString s = p.toLower();
    QStringList done;

    if (s.contains("пауза") || s.contains("останов") || s.contains("pause")) {
        m_sim->pause();
        done << "симуляция поставлена на паузу";
    }
    if (s.contains("запусти") || s.contains("продолж") || s.contains("start simulation")) {
        m_sim->start();
        done << "симуляция запущена";
    }
    if (s.contains("сброс") || s.contains("reset")) {
        m_sim->reset();
        done << "состояние сброшено";
    }
    if (s.contains("следующ") && s.contains("эпох")) {
        m_sim->skipToNextEpoch();
        done << "запущен переход к следующей достижимой эпохе";
    }

    QRegularExpression re(
        "(?:x|×|скорост(?:ь|ью)?|speed)\\s*([0-9]+(?:[\\.,][0-9]+)?)",
        QRegularExpression::CaseInsensitiveOption);
    auto m = re.match(p);
    if (m.hasMatch()) {
        double v = m.captured(1).replace(',', '.').toDouble();
        m_sim->setSpeed(v);
        done << QString("скорость установлена ×%1").arg(v);
    }

    QRegularExpression ff(
        "(?:пропусти|ускорь на|fast.?forward)\\s*([0-9]+(?:[\\.,][0-9]+)?)\\s*(?:с|сек|seconds?)?",
        QRegularExpression::CaseInsensitiveOption);
    auto f = ff.match(p);
    if (f.hasMatch() && !s.contains("эпох")) {
        double sec = f.captured(1).replace(',', '.').toDouble();
        m_sim->fastForward(sec);
        done << QString("добавлено %1 условных секунд").arg(sec);
    }

    if (s.contains("экспорт") && s.contains("отч")) {
        m_sim->exportReport("afterlife-report.json");
        done << "полный отчёт отправлен в afterlife-report.json";
    }

    return done.join("; ");
}

QString LocalAssistant::fallback(const QString &p) const
{
    Q_UNUSED(p);

    if (!m_sim)
        return "Ассистент пока не связан с симулятором.";

    return QString(
               "Локальная LLM не ответила, поэтому доступен встроенный экспертный режим. "
               "Сейчас эпоха %1, t=%2, планет %3, пригодных %4, жизнь: %5 клеток / %6 колоний / "
               "%7 разумных агентов. Энтропия %8, эффективность %9, индекс выбора %10. "
               "Для свободного диалога запустите локальный OpenAI-compatible сервер "
               "(например llama.cpp) на указанном endpoint — данные останутся на машине.")
        .arg(m_sim->epoch())
        .arg(m_sim->time(), 0, 'f', 1)
        .arg(m_sim->planetCount())
        .arg(m_sim->habitablePlanetCount())
        .arg(m_sim->cellCount())
        .arg(m_sim->colonyCount())
        .arg(m_sim->mindCount())
        .arg(m_sim->entropy(), 0, 'f', 3)
        .arg(m_sim->efficiency(), 0, 'f', 2)
        .arg(m_sim->decisionIndex(), 0, 'f', 2);
}

void LocalAssistant::ask(const QString &prompt)
{
    if (prompt.trimmed().isEmpty())
        return;

    const QString control = applyControlIntent(prompt);
    if (!control.isEmpty())
        emit commandExecuted(control);

    setBusy(true);

    QNetworkRequest req((QUrl(m_endpoint)));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject root;
    root["model"] = "local-model";
    root["temperature"] = 0.55;
    root["max_tokens"] = 450;

    QJsonArray msgs;

    QJsonObject sys;
    sys["role"] = "system";
    sys["content"] = QString(
                         "Ты локальный ассистент приложения AfterLife. Отвечай содержательно на общий вопрос "
                         "пользователя, а если он касается модели — учитывай текущие данные: epoch=%1,time=%2,"
                         "stars=%3,planets=%4,life=%5,entropy=%6,efficiency=%7. Не притворяйся, что физическая "
                         "модель является точной моделью реальной Вселенной: это нормализованная вычислительная "
                         "аппроксимация.")
                         .arg(m_sim ? m_sim->epoch() : 0)
                         .arg(m_sim ? m_sim->time() : 0)
                         .arg(m_sim ? m_sim->starCount() : 0)
                         .arg(m_sim ? m_sim->planetCount() : 0)
                         .arg(m_sim ? (m_sim->cellCount() + m_sim->colonyCount() + m_sim->mindCount()) : 0)
                         .arg(m_sim ? m_sim->entropy() : 0)
                         .arg(m_sim ? m_sim->efficiency() : 0);
    msgs.append(sys);

    QJsonObject user;
    user["role"] = "user";
    user["content"] = prompt;
    msgs.append(user);

    root["messages"] = msgs;

    auto *reply = m_net.post(req, QJsonDocument(root).toJson(QJsonDocument::Compact));

    connect(reply, &QNetworkReply::finished, this, [this, reply, prompt, control]() {
        setBusy(false);

        if (reply->error() != QNetworkReply::NoError) {
            setStatus("встроенный режим · локальная LLM недоступна");
            QString r = fallback(prompt);
            if (!control.isEmpty())
                r = QString("Выполнено: %1.\n\n%2").arg(control, r);
            emit responseReady(r);
            reply->deleteLater();
            return;
        }

        auto doc = QJsonDocument::fromJson(reply->readAll());
        QString out;

        if (doc.isObject()) {
            auto choices = doc.object().value("choices").toArray();
            if (!choices.isEmpty())
                out = choices.first().toObject().value("message").toObject().value("content").toString();
        }

        if (out.isEmpty())
            out = fallback(prompt);

        setStatus("локальная LLM подключена");
        emit responseReady(out);
        reply->deleteLater();
    });
}