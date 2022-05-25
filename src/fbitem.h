#ifndef FBITEM_H
#define FBITEM_H

#include <QOpenGLFunctions>
#include <QOpenGLFunctions_3_3_Core>
#include <QQuickFramebufferObject>

#include <libpgmaker/clip.h>
//#include <libpgmaker/preview.h>

class FbItem;
class FbItemRenderer : public QQuickFramebufferObject::Renderer
{
  public:
    FbItemRenderer(const FbItem* item);
    ~FbItemRenderer();
    QOpenGLFramebufferObject* createFramebufferObject(const QSize& size) override;
    void render() override;
    void synchronize(QQuickFramebufferObject* item) override;

  private:
    const FbItem* item;
    libpgmaker::frame* currentFrame;
    QSize size;
    // libpgmaker::preview* prev;

    QOpenGLFunctions_3_3_Core* ogl;
    unsigned int vao, texture, shaderProgram;

  private:
    void initialize_shaders();
    void initialize_vao();
    void initialize_texture();
};

class FbItem : public QQuickFramebufferObject
{
    Q_OBJECT
    QML_ELEMENT
  public:
    FbItem();

    QQuickFramebufferObject::Renderer* createRenderer() const override;
    libpgmaker::frame* get_current_frame();
  public slots:
    void frame_updated(libpgmaker::frame* f);

  private:
    libpgmaker::frame* currentFrame;
};

#endif // FBITEM_H
