#include "qtrenderer.h"

QtRenderer::QtRenderer(QObject* parent):
    QObject{ parent }
{ }
void QtRenderer::init()
{
}
void QtRenderer::paint()
{
    m_window->beginExternalCommands();
    ////
    m_window->endExternalCommands();
}

void QtRenderer::setViewportSize(const QSize& size)
{
}
void QtRenderer::setWindow(QQuickWindow* window)
{
    m_window = window;
}
