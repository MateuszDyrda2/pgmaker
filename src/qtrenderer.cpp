#include "qtrenderer.h"

#include <glad/glad.h>

#include <cstdio>

using namespace libpgmaker;
QtRenderer::QtRenderer(QObject* parent):
    QObject{ parent }, prev{}
{ }
void QtRenderer::init()
{
    if(!prev)
    {
        if(!gladLoadGL())
        {
            fprintf(stderr, "Failed to load glad\n");
            return;
        }
        prev = new preview(resolution{ .width  = unsigned(m_window->size().width()),
                                       .height = unsigned(m_window->size().height()) });
    }
}
void QtRenderer::paint()
{
    if(prev)
    {
        m_window->beginExternalCommands();
        ////
        m_window->endExternalCommands();
    }
}
void QtRenderer::setViewportSize(const QSize& size)
{
    if(prev)
    {
        prev->resize(resolution{ .width  = unsigned(size.width()),
                                 .height = unsigned(size.height()) });
    }
}
void QtRenderer::setWindow(QQuickWindow* window)
{
    m_window = window;
}
