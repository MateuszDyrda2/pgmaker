#include "lua_binding.h"

void lua_effect::prepare(int width, int height)
{
    this->width  = width;
    this->height = height;
    L            = luaL_newstate();
    luaL_openlibs(L);
    luabridge::getGlobalNamespace(L)
        .beginClass<image>("image")
        .addFunction("get", &image::get)
        .addFunction("set", &image::set)
        .endClass();
    int scriptLoadStatus = luaL_dofile(L, path.c_str());
    if(scriptLoadStatus != 0) throw std::runtime_error("ASDA");

    p = luabridge::getGlobal(L, "process");
    if(!p.isFunction()) throw std::runtime_error("ASDA");
}
void lua_effect::process(AVFrame* inFrame, AVFrame* outFrame)
{
    image b((float*)(inFrame->data[0]), width, height);
    image g((float*)(inFrame->data[1]), width, height);
    image r((float*)(inFrame->data[2]), width, height);
    image a((float*)(inFrame->data[3]), width, height);

    image out_b((float*)(outFrame->data[0]), width, height);
    image out_g((float*)(outFrame->data[1]), width, height);
    image out_r((float*)(outFrame->data[2]), width, height);
    image out_a((float*)(outFrame->data[3]), width, height);

    p(&b, &g, &r, &a, &out_b, &out_g, &out_r, &out_a, width, height);
}
void lua_effect::cleanup()
{
}
