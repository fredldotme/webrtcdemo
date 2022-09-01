#ifndef EXAMPLE_H
#define EXAMPLE_H

#include <QObject>
#include <QAbstractVideoFilter>
#include <QVideoFilterRunnable>
#include <QSharedPointer>
#include <QElapsedTimer>
#include <QtWebSockets/QtWebSockets>

#include <qwebrtcconfiguration.hpp>
#include <qwebrtcpeerconnectionfactory.hpp>
#include <qwebrtcmediastream.hpp>
#include <qwebrtcmediatrack.hpp>
#include <qwebrtcpeerconnection.hpp>

#include <webrtc/media/base/adaptedvideotracksource.h>

class WebRTCConnector : public QObject {
    Q_OBJECT
    Q_PROPERTY(QObject* cameraFetcher READ cameraFetcher CONSTANT)

public:
    WebRTCConnector(QObject* parent = nullptr);
    QObject* cameraFetcher();
    Q_INVOKABLE void start(QString address, QString room);
    Q_INVOKABLE void connectRtc();

private Q_SLOTS:
    void sendToSignalingServer(QString message);

Q_SIGNALS:
    void videoAdded(QObject* track);

private:
    void negotiate();

    QString m_room;
    QWebRTCConfiguration m_config;
    QWebRTCPeerConnectionFactory m_factory;
    QSharedPointer<QWebRTCPeerConnection> m_peerConnection;
    WebRTCFrameFetcher* m_fetcher;
    QWebSocket m_signalingSocket;
};

#endif
