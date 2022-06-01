#pragma once

#include "BezierCurve.h"
#include "Block.h"
#include "Circle.h"

class InputBlock : public Block
{
    float rowHeight;
    std::unique_ptr<BezierCurve> drag;
    std::unique_ptr<Circle> output;
    std::list<std::shared_ptr<BezierCurve>> connectors;

  public:
    InputBlock(ImVec2 _pos, float _width, float _rowHeight, ImU32 _col):
        Block(_pos, _width, _rowHeight * 2, _col, "InputBlock"), rowHeight(_rowHeight)
    {
        drag   = std::make_unique<BezierCurve>(ImVec2{ _pos.x + _width, _pos.y + _rowHeight * 1.5f }, ImVec2{ _pos.x + _width, _pos.y + _rowHeight * 1.5f }, _width, _rowHeight * 2, 0.f, 0.f);
        output = std::make_unique<Circle>(ImVec2{ _pos.x + _width, _pos.y + _rowHeight * 1.5f }, height * 0.1f, _col);
    }

    void releaseUse() override
    {
        Block::releaseUse();
        drag->setPos({ pos.x + width, pos.y + rowHeight * 1.5f }, ImVec2{ pos.x + width, pos.y + rowHeight * 1.5f });
    }

    void setPos(ImVec2 mousePos) override
    {
        Block::setPos(mousePos);
        output->setPos(ImVec2{ pos.x + width, pos.y + rowHeight * 1.5f });
        drag->setPos({ pos.x + width, pos.y + rowHeight * 1.5f }, ImVec2{ pos.x + width, pos.y + rowHeight * 1.5f });
        for(auto connector : connectors)
            connector->setInputPos(ImVec2{ pos.x + width, pos.y + rowHeight * 1.5f });
    }

    ImVec2 getOutputCirclePos() override
    {
      return output->getPos();
    }

    void addConnector(std::shared_ptr<BezierCurve> connector)
    {
        connectors.push_back(connector);
    }

    bool interact(ImVec2 mousePos) override
    {
        if(output->isInTouch(mousePos))
        {
            inUse = true;
        }
        if(inUse)
        {
            drag->setOutputPos(mousePos);
            return true;
        }
        else if(Block::isInTouch(mousePos))
        {
            setPos(mousePos);
            return true;
        }
        return false;
    }

    void draw(ImDrawList* drawList) override
    {
        Block::draw(drawList);
        output->draw(drawList);
        drag->draw(drawList);
    }
};