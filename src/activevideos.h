#ifndef ACRIVEVIDEOS_H
#define ACRIVEVIDEOS_H

#include <libpgmaker/video_reader.h>

#include <QObject>

class ActiveVideos : public QObject
{
    Q_OBJECT
  public:
    explicit ActiveVideos(QObject* parent = nullptr);

  signals:
    void new_clip(std::shared_ptr<libpgmaker::video> vid);

  public slots:
    void video_chosen(const QString& path);

  private:
    libpgmaker::video_reader reader;
};

#endif // ACRIVEVIDEOS_H
