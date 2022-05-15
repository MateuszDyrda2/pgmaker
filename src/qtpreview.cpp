#include "qtpreview.h"

#include <QRunnable>

QtPreview::QtPreview():
    renderer(nullptr)
{
    connect(this, &QQuickItem::windowChanged, this, &QtPreview::handleWindowChanged);
}
void QtPreview::handleWindowChanged(QQuickWindow* win)
{
    if(win)
    {
        connect(win, &QQuickWindow::beforeSynchronizing, this, &QtPreview::sync, Qt::DirectConnection);
        connect(win, &QQuickWindow::sceneGraphInvalidated, this, &QtPreview::cleanup, Qt::DirectConnection);
        win->setColor(Qt::black);
    }
}
void QtPreview::sync()
{
    if(!renderer)
    {
        renderer = new QtRenderer;
        connect(window(), &QQuickWindow::beforeRendering, renderer, &QtRenderer::init, Qt::DirectConnection);
        connect(window(), &QQuickWindow::beforeRenderPassRecording, renderer, &QtRenderer::paint, Qt::DirectConnection);
    }
    renderer->setViewportSize(window()->size() * window()->devicePixelRatio());
    renderer->setWindow(window());
}
void QtPreview::cleanup()
{
    delete renderer;
    renderer = nullptr;
}
class CleanupJob : public QRunnable
{
  public:
    CleanupJob(QtRenderer* renderer):
        renderer(renderer) { }
    void run() override { delete renderer; }

  private:
    QtRenderer* renderer;
};
void QtPreview::releaseResources()
{
    window()->scheduleRenderJob(new CleanupJob(renderer), QQuickWindow::BeforeSynchronizingStage);
    renderer = nullptr;
}
