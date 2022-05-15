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

  signals:

  public slots:
    void sync();
    void cleanup();

  private slots:
    void handleWindowChanged(QQuickWindow* win);

  private:
    void releaseResources() override;

  private:
    QtRenderer* renderer;
};

#endif // QTPREVIEW_H
