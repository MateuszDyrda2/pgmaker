#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "qtpreview.h"

int main(int argc, char** argv)
{
    QGuiApplication app(argc, argv);
    // const QUrl url(u"qrc:/nauka/main.qml"_qs);

    qmlRegisterType<QtPreview>("Preview", 1, 0, "QtPreview");
    QQmlApplicationEngine engine(QUrl("qrc:/layout/src/qml/main.qml"));

    return app.exec();
}
