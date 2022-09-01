#include <QtQml>
#include <QtQml/QQmlContext>

#include "plugin.h"
#include "example.h"

#include <qwebrtcquickvideoitem.hpp>

void ExamplePlugin::registerTypes(const char *uri) {
    //@uri Example
    qmlRegisterSingletonType<WebRTCConnector>(uri, 1, 0, "WebRTCConnector", [](QQmlEngine*, QJSEngine*) -> QObject* { return new WebRTCConnector; });
    qmlRegisterSingletonType<WebRTCFrameFetcher>(uri, 1, 0, "WebRTCFrameFetcher", [](QQmlEngine*, QJSEngine*) -> QObject* { return new WebRTCFrameFetcher; });

    // QWebRTC types
    qmlRegisterType<QWebRTCQuickVideoItem>("QWebRTC", 1, 0, "QWebRTCQuickVideoItem");
}
