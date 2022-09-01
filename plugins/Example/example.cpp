#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMetaObject>
#include <QThread>

#include <qwebrtcicecandidate.hpp>
#include <qwebrtcutilities.hpp>

#include "example.h"

WebRTCConnector::WebRTCConnector(QObject* parent) : QObject(parent),
    m_fetcher(new WebRTCFrameFetcher()),
    m_signalingSocket(QStringLiteral("http://localhost/"))
{
    // Only if we can reach the signaling server should we start the negotiation to keep a well known flow.
    // Connection to the signaling server happens in ::start()
    connect(&m_signalingSocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error),
            [=](QAbstractSocket::SocketError error){
        qWarning() << "WebSocket error:" << error;
    });

    connect(&m_signalingSocket, &QWebSocket::connected, this, [=]() {
        qDebug() << "Connected to signaling server, creating PeerConnection";

        QWebRTCIceServer ice1;
        ice1.urls.append(QStringLiteral("stun:139.59.214.236:3478").toUtf8());
        ice1.tlsCertNoCheck = false;

        m_config.iceServers.append(ice1);
        m_peerConnection = m_factory.createPeerConnection(m_config);

        // Signaling
        connect(m_peerConnection.data(), &QWebRTCPeerConnection::signalingChange,
                this, [=](){
            qDebug() << "Signaling change:" << (int)m_peerConnection->signalingState();
        });

        connect(m_peerConnection.data(), &QWebRTCPeerConnection::newIceCandidate,
                this, [=](const QSharedPointer<QWebRTCIceCandidate>& candidate) {
            //QSharedPointer<QWebRTCSessionDescription> description =
            //        QWebRTCPeerConnection::createSessionDescription(QWebRTCSessionDescription::SDPType, const QByteArray&)
            //m_peerConnection->addIceCandidate(candidate);
            QJsonObject obj = QWebRTCUtilities::iceCandidateToJSON(candidate);
            sendToSignalingServer(QJsonDocument(obj).toJson());
        });

        connect(m_peerConnection.data(), &QWebRTCPeerConnection::iceCandidateRemoved,
                this, [=](const QSharedPointer<QWebRTCIceCandidate>& candidate) {
            //m_peerConnection->removeIceCandidate(candidate);
        });

        // Addition of new streams
        connect(m_peerConnection.data(), &QWebRTCPeerConnection::videoTrackAdded,
                this, [=](const QSharedPointer<QWebRTCMediaTrack>& track) {
            QObject* obj = qobject_cast<QObject*>(track.data());
            qDebug() << "new video track:" << obj;
            Q_EMIT videoAdded(obj);
        });
    });

    connect(&m_signalingSocket, &QWebSocket::textMessageReceived, this, [=](const QString& message) {
        qDebug() << "WebSocket message in state" << (int)m_peerConnection->signalingState() << ":" << message;
        QJsonObject obj = QJsonDocument::fromJson(message.toUtf8()).object();

        if (obj.contains("type") && obj.contains("sdp")) {
            if (obj.value("type") == QString("offer")) {
                QSharedPointer<QWebRTCSessionDescription> description = QWebRTCUtilities::sessionFromJSON(obj);
                m_peerConnection->setRemoteDescription(description, [=](bool success) {
                    qDebug() << "offered setRemoteDescription?" << success;

                    QVariantMap constraints;
                    m_peerConnection->createAnswerForConstraints(constraints,
                                                                 [=](const QSharedPointer<QWebRTCSessionDescription>& description) {
                        m_peerConnection->setLocalDescription(description, [=](bool success) {
                            if (!success)
                                return;

                            QJsonObject answer = QWebRTCUtilities::sessionToJSON(description);
                            sendToSignalingServer(QJsonDocument(answer).toJson());
                        });
                    });
                });
            } else if (obj.value("type") == QString("answer")) {
                QSharedPointer<QWebRTCSessionDescription> description = QWebRTCUtilities::sessionFromJSON(obj);
                m_peerConnection->setRemoteDescription(description, [=](bool success) {
                    qDebug() << "answered setRemoteDescription?" << success;

                    // Let's get ready to rumble on this endpoint too
                    //if (success) {
                    //    connectRtc();
                    //}
                });
            }
        } else if (obj.contains("sdpMid") && obj.contains("candidate") && obj.contains("sdpMLineIndex")) {
            qDebug() << "Adding ICE candidate";
            QSharedPointer<QWebRTCIceCandidate> candidate = QWebRTCUtilities::iceCandidateFromJSON(obj);
            m_peerConnection->addIceCandidate(candidate);
        }
    });
}

QObject* WebRTCConnector::cameraFetcher()
{
    return m_fetcher;
}

void WebRTCConnector::start(QString address, QString room)
{
    m_room = room;

    const QString wsAddr = QStringLiteral("%1").arg(address);
    QList<QSslError> expectedSslErrors;
    expectedSslErrors.append(QSslError::CertificateRevoked);
    m_signalingSocket.ignoreSslErrors(expectedSslErrors);
    m_signalingSocket.open(QUrl(wsAddr));
}

void WebRTCConnector::connectRtc()
{
    QSharedPointer<QWebRTCMediaStream> stream = m_factory.createMediaStream("stream");
    QSharedPointer<QWebRTCMediaTrack> audioTrack = m_factory.createAudioTrack(QVariantMap(), "microphone");
    QSharedPointer<QWebRTCMediaTrack> videoTrack = m_factory.createVideoTrack(m_fetcher, "camera");

    if (audioTrack)
        stream->addTrack(audioTrack);
    if (videoTrack)
        stream->addTrack(videoTrack);

    m_peerConnection->addStream(stream);

    negotiate();
}

void WebRTCConnector::negotiate()
{
    QVariantMap constraints;
    constraints.insert("receiveVideo", 1);
    constraints.insert("receiveAudio", 1);
    m_peerConnection->createOfferForConstraints(constraints,
                                                [=](const QSharedPointer<QWebRTCSessionDescription>& description) {
        qDebug() << "createOfferForConstraints";
        m_peerConnection->setLocalDescription(description, [=](bool success) {
            qDebug() << "setLocalDescription" << success;
            if (!success) {
                return;
            }

            QJsonObject offer = QWebRTCUtilities::sessionToJSON(description);
            sendToSignalingServer(QJsonDocument(offer).toJson());
        });
    });
}

void WebRTCConnector::sendToSignalingServer(QString message)
{
    if (QThread::currentThread() != this->thread()) {
        QMetaObject::invokeMethod(this, "sendToSignalingServer",
                                  Qt::QueuedConnection, Q_ARG(QString, message));
        return;
    }
    m_signalingSocket.sendTextMessage(message);
    m_signalingSocket.flush();
}
