import QtQuick 2.7
import Ubuntu.Components 1.3
import QtQuick.Layouts 1.3
import Qt.labs.settings 1.0
import QtMultimedia 5.12

import Example 1.0
import QWebRTC 1.0

MainView {
    id: root
    objectName: 'mainView'
    applicationName: 'webrtcdemo.fredldotme'
    automaticOrientation: true

    width: units.gu(45)
    height: units.gu(75)

    Settings {
        property alias lastAddress : address.text
        property alias lastRoom : room.text
    }

    Page {
        anchors.fill: parent

        header: PageHeader {
            id: header
            title: i18n.tr('WebRTC Demo')
        }

        Camera {
            id: camera
            position: Camera.FrontFace
            onCameraStateChanged: {
                if (state !== Camera.LoadedState) {
                    return;
                }

                var resolutions = supportedViewfinderResolutions();
                for (var resolution in resolutions) {
                    if (resolution.width <= videoOutput.width && resolution.height <= videoOutput.height) {
                        camera.viewfinder.resolution = resolution;
                        return;
                    }
                }
            }
        }

        ColumnLayout {
            id: mainColumn
            spacing: units.gu(2)
            anchors {
                margins: units.gu(2)
                top: header.bottom
                left: parent.left
                right: parent.right
                bottom: parent.bottom
            }

            TextField {
                id: address
                text: "ws://192.168.2.65:8089"
                Layout.alignment: Qt.AlignHCenter
            }

            TextField {
                id: room
                text: "test1"
                Layout.alignment: Qt.AlignHCenter
            }

            Button {
                Layout.alignment: Qt.AlignHCenter
                text: i18n.tr('Connect to server')
                onClicked: {
                    WebRTCConnector.start(address.text, room.text)
                }
            }

            Button {
                Layout.alignment: Qt.AlignHCenter
                text: i18n.tr('Join conversation')
                onClicked: {
                    WebRTCConnector.connectRtc()
                }
            }

            Component {
                id: videoComponent
                QWebRTCQuickVideoItem {
                    id: webRtcVideoItem
                    videoTrack: null
                    anchors.fill: parent
                    rotation: 270

                    Component.onCompleted: {
                        console.log("QWebRTCQuickVideoItem created!")
                    }
                }
            }

            Connections {
                target: WebRTCConnector
                onVideoAdded: {
                    videoComponent.createObject(videos, { videoTrack: track })
                }
            }

            Item {
                id: videos
                Layout.fillHeight: true
                Layout.fillWidth: true
            }

            Component.onCompleted: {
                camera.start()
            }
        }

        VideoOutput {
            id: videoOutput
            source: camera
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            width: 600
            height: 400
            orientation: 270
            filters: [ WebRTCConnector.cameraFetcher ]
        }
    }
}
