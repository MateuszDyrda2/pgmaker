#pragma once

#include "pg_types.h"
#include <cstdint>

namespace libpgmaker {
class effect
{
  public:
    virtual void prepare(int width, int height)               = 0;
    virtual void process(AVFrame* inFrame, AVFrame* outFrame) = 0;
    virtual void cleanup()                                    = 0;
    virtual ~effect()                                         = default;
};
class glsl_effect : public effect
{
  public:
    void prepare(int width, int height) override;
    void process(AVFrame* inFrame, AVFrame* outFrame) override;
    void cleanup() override;
    virtual const char* get_shader() const = 0;
    virtual void set_uniforms() { }
    virtual ~glsl_effect() = default;

    int width, height;
    unsigned int inTexs[4]{};
    unsigned int outTexs[4]{};
    unsigned int programID{ 0 };
};
class grayscale : public glsl_effect
{
  public:
    const char* get_shader() const override;
};
class pass_through : public effect
{
    void prepare(int width, int height) override;
    void process(AVFrame* inFrame, AVFrame* outFrame) override;
    void cleanup() override;

  private:
    int width, height;
};
} // namespace libpgmaker
