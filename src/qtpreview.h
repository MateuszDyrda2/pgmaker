#ifndef QTPREVIEW_H
#define QTPREVIEW_H

#include "qtrenderer.h"

#include <QQuickItem>

class QtPreview : public QQuickItem
{
    Q_OBJECT
    QML_ELEMENT
  public:
    QtPreview();
    QtRenderer* get_renderer() const { return renderer; }

  signals:
    void frame_sent(libpgmaker::frame* frame);

  public slots:
    void sync();
    void cleanup();
    void update_frame(libpgmaker::frame* frame);

  private slots:
    void handleWindowChanged(QQuickWindow* win);

  private:
    void releaseResources() override;

  private:
    QtRenderer* renderer;
};

#endif // QTPREVIEW_H
