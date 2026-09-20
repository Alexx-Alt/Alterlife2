#ifndef LOCALASSISTANT_H
#define LOCALASSISTANT_H
#include <QObject>
#include <QNetworkAccessManager>
class Simulator;

class LocalAssistant:public QObject{
    Q_OBJECT
    Q_PROPERTY(QString endpoint READ endpoint WRITE setEndpoint NOTIFY endpointChanged)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
public:
    explicit LocalAssistant(QObject*parent=nullptr);void setSimulator(Simulator*s){m_sim=s;}
    QString endpoint()const{return m_endpoint;}void setEndpoint(const QString&v){if(m_endpoint==v)return;m_endpoint=v;emit endpointChanged();}
    QString status()const{return m_status;}bool busy()const{return m_busy;}
    Q_INVOKABLE void ask(const QString&prompt);
signals:void endpointChanged();void statusChanged();void busyChanged();void responseReady(const QString&text);void commandExecuted(const QString&text);
private:
    QString applyControlIntent(const QString&prompt);QString fallback(const QString&prompt)const;void setStatus(const QString&s);void setBusy(bool b);
    Simulator*m_sim=nullptr;QNetworkAccessManager m_net;QString m_endpoint="http://127.0.0.1:8080/v1/chat/completions";QString m_status="локальная модель не проверена";bool m_busy=false;
};
#endif
