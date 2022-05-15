#include <QGuiApplication>
#include <QQmlApplicationEngine>

int main(int argc, char** argv)
{
    QGuiApplication app(argc, argv);
    //const QUrl url(u"qrc:/nauka/main.qml"_qs);
    QQmlApplicationEngine engine(QUrl("qrc:/layout/src/qml/main.qml"));

    return app.exec();
}
