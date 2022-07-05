#pragma once

extern "C"
{
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}

#include <LuaBridge.h>
#include <libpgmaker/effect.h>
#include <string>

class image
{
  public:
    image(float* ptr, unsigned int width, unsigned int height):
        ptr(ptr), width(width), height(height)
    { }
    float get(unsigned int i, unsigned int j)
    {
        return ptr[i * width + j];
    }
    void set(unsigned int i, unsigned int j, float value)
    {
        ptr[i * width + j] = value;
    }

  private:
    float* ptr;
    unsigned int width, height;
};
class lua_effect : public libpgmaker::effect
{
  public:
    lua_effect(const std::string& path):
        path(path), p{ nullptr } { }
    libpgmaker::effect* instantiate() override { return new lua_effect(path); }
    std::string get_name() override { return path; }
    void prepare(int width, int height) override;
    void process(AVFrame* inFrame, AVFrame* outFrame) override;
    void cleanup() override;

  private:
    std::string path;
    unsigned int width, height;
    lua_State* L;
    luabridge::LuaRef p;
};
