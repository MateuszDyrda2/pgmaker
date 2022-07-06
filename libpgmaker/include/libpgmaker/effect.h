/** @file */

#pragma once

#include "pg_types.h"

#include <cstdint>
#include <string>
#include <unordered_map>

namespace libpgmaker {

#define EFFECT(_name)                       \
    virtual effect* instantiate() override  \
    {                                       \
        return new _name;                   \
    }                                       \
    virtual std::string get_name() override \
    {                                       \
        return #_name;                      \
    }

class effect;
class effect_manager
{
  public:
    effect_manager();
    ~effect_manager();
    auto& get_available_effects() { return availableEffects; }
    void add_effect(effect* ef);

  private:
    std::unordered_map<std::string, effect*> availableEffects;
};
/** Effect interface */
class effect
{
  public:
    /** @brief Prepare effect for processing
     * @param width width of the video
     * @param height height of the video
     */
    virtual void prepare(int width, int height) = 0;
    /** @brief Process the video using the effect
     * @param inFrame input frame
     * @param outFrame output frame
     */
    virtual void process(AVFrame* inFrame, AVFrame* outFrame) = 0;
    /** @brief Cleanup after the effect */
    virtual void cleanup()         = 0;
    virtual effect* instantiate()  = 0;
    virtual std::string get_name() = 0;
    virtual ~effect()              = default;
};
/** Video filter in glsl shaders base class */
class glsl_effect : public effect
{
  public:
    /** @brief Prepare effect for processing
     * @param width width of the video
     * @param height height of the video
     */
    void prepare(int width, int height) override;
    /** @brief Process the video using the effect
     * @param inFrame input frame
     * @param outFrame output frame
     */
    void process(AVFrame* inFrame, AVFrame* outFrame) override;
    void cleanup() override;
    virtual const char* get_shader() const = 0;
    /** @brief Cleanup after the effect */
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
    EFFECT(grayscale);

  public:
    const char* get_shader() const override;
};
class pass_through : public effect
{
  public:
    EFFECT(pass_through)
  public:
    void prepare(int width, int height) override;
    void process(AVFrame* inFrame, AVFrame* outFrame) override;
    void cleanup() override;

  private:
    int width, height;
};
} // namespace libpgmaker
