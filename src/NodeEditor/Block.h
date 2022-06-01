#pragma once

#include <algorithm>
#include <iostream>
#include <list>
#include <memory>

#include <imgui.h>

class Block
{
  protected:
    ImVec2 pos; 
    float width, height;
    ImU32 col;
    std::string name;
    bool inUse;

  public:
    Block(ImVec2 _pos, float _width, float _height, ImU32 _col, std::string _name):
        pos(_pos), width(_width), height(_height), col(_col), name(_name), inUse(false) { }

    virtual bool interact(ImVec2 mousePos) = 0;

    ImVec2 getPos()
    {
        return pos;
    }

    float getWidth()
    {
        return width;
    }

    float getHeight()
    {
        return height;
    }

    virtual ImVec2 getOutputCirclePos()
    {
      return ImVec2{0.f, 0.f};
    }

    bool isInUse()
    {
        return inUse;
    }

    bool isInTouch(ImVec2 mousePos)
    {
        if(mousePos.x >= pos.x && mousePos.x <= pos.x + width && mousePos.y >= pos.y && mousePos.y <= pos.y + height)
            return true;
        else
            return false;
    }

    virtual void releaseUse()
    {
        inUse = false;
    }

    virtual void setPos(ImVec2 mousePos)
    {
        float xDelta = pos.x - mousePos.x;
        float yDelta = pos.y - mousePos.y;
        pos.x            = mousePos.x - width * 0.5;
        pos.y            = mousePos.y - height * 0.5;
    }

    virtual void draw(ImDrawList* drawList)
    {  
        drawList->AddRectFilled({ pos.x, pos.y }, { pos.x + width, pos.y + height }, 0x404040FF, 5.f);
        drawList->AddRect({ pos.x, pos.y }, { pos.x + width, pos.y + height }, 0x808080FF, 5.f, 0, 5.f);
        drawList->AddText(ImVec2{ pos.x + 5.f, pos.y + 5.f }, 0xFFFFFFFF, name.c_str());
    }
};