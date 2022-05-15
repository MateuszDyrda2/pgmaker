#ifndef QTTIMELINE_H
#define QTTIMELINE_H

#include <libpgmaker/timeline.h>

#include <QObject>
#include <QTimer>

class QtTimeline : public QObject
{
    Q_OBJECT
  public:
    explicit QtTimeline(QObject* parent = nullptr);

  signals:
    void frame_sent(libpgmaker::frame* frame);

  public slots:
    void play();
    void new_clip(std::shared_ptr<libpgmaker::video> v);

  private:
    libpgmaker::timeline tl;
    QTimer* timer;
    bool paused;

  private:
    void update();
};

#endif // QTTIMELINE_H
