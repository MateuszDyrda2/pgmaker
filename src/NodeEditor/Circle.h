#pragma once

#include <imgui.h>

class Circle
{
    ImVec2 pos;
    float radius;
    ImU32 col;

  public:
    Circle(ImVec2 _pos, float _radius, ImU32 _col):
        pos(_pos), radius(_radius), col(_col) { }

    bool isInTouch(ImVec2 mousePos)
    {
        if(mousePos.x >= pos.x - radius && mousePos.x <= pos.x + radius && mousePos.y >= pos.y - radius && mousePos.y <= pos.y + radius)
            return true;
        else
            return false;
    }

    void setPos(ImVec2 _pos)
    {
        pos = _pos;
    }

    ImVec2 getPos()
    {
        return pos;
    }

    void draw(ImDrawList* drawList)
    {
        drawList->AddCircleFilled(pos, radius, 0x404040FF);
        drawList->AddCircle(pos, radius, 0x808080FF, 0, 5.f);
    }
};