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
        glViewport(0, 0, viewportSize.width(), viewportSize.height());
        glClearColor(1.0f, 0.f, 0.f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        prev->draw();
        m_window->endExternalCommands();
    }
}
void QtRenderer::setViewportSize(const QSize& size)
{
    viewportSize = size;
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
void QtRenderer::update_frame(libpgmaker::frame* f)
{
    if(prev)
    {
        prev->update(f);
    }
}