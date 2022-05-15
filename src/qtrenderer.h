#ifndef QTRENDERER_H
#define QTRENDERER_H

#include <QObject>
//#include <QOpenGLFunctions>
#include <QQuickWindow>

#include <libpgmaker/preview.h>

class QtRenderer : public QObject//, protected QOpenGLFunctions
{
    Q_OBJECT
  public:
    explicit QtRenderer(QObject* parent = nullptr);

    void setViewportSize(const QSize& size);
    void setWindow(QQuickWindow* window);

  public slots:
    void init();
    void paint();

  private:
    libpgmaker::preview* prev;
    QSize viewportSize;
    QQuickWindow* m_window;
};

#endif // QTRENDERER_H
