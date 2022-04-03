#include <QGuiApplication>
#include <QQmlApplicationEngine>

int main(int argc, char** argv)
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine(QUrl("qrc:/qml/MainWindow"));

    return app.exec();
}
