#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "activevideos.h"
#include "qtpreview.h"
#include "qttimeline.h"

int main(int argc, char** argv)
{
    QGuiApplication app(argc, argv);
    // const QUrl url(u"qrc:/nauka/main.qml"_qs);

    qmlRegisterType<QtPreview>("Preview", 1, 0, "QtPreview");
    QQmlApplicationEngine engine(QUrl("qrc:/layout/src/qml/main.qml"));
    auto rootObj = engine.rootObjects().first();
    auto fd      = rootObj->findChild<QObject*>("openFileDialog");
    auto grid    = rootObj->findChild<QObject*>("grid");
    auto preview = grid->findChild<QObject*>("preview");

    ActiveVideos av;
    QObject::connect(fd, SIGNAL(qmlSignal(QString)),
                     &av, SLOT(video_chosen(QString)));
    QtTimeline tl;
    QObject::connect(&tl, SIGNAL(frame_sent(libpgmaker::frame*)),
                     preview, SLOT(update_frame(libpgmaker::frame*)));
    QObject::connect(&av, &ActiveVideos::new_clip, &tl, &QtTimeline::new_clip);
    QObject::connect(rootObj, SIGNAL(playPressed()), &tl, SLOT(play()));

    return app.exec();
}
