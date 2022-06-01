#pragma once

#include "BezierCurve.h"
#include "Block.h"
#include "Circle.h"

class OutputBlock : public Block
{
    float rowHeight;
    std::unique_ptr<Circle> input;
    std::shared_ptr<BezierCurve> connector;
    std::shared_ptr<Block> inputBlock;

  public:
    OutputBlock(ImVec2 _pos, float _width, float _rowHeight, ImU32 _col):
        Block(_pos, _width, _rowHeight * 2, _col, "OutputBlock"), rowHeight(_rowHeight)
    {
        input = std::make_unique<Circle>(ImVec2{ _pos.x, _pos.y + _rowHeight * 1.5f }, height * 0.1f, _col);
    }

    void setInputBlock(std::shared_ptr<Block> _inputBlock)
    {
        connector.reset();
        inputBlock = _inputBlock;
        connector  = std::make_shared<BezierCurve>(inputBlock->getOutputCirclePos(), input->getPos(), inputBlock->getWidth(), inputBlock->getHeight(), width, height);
    }

    void setPos(ImVec2 mousePos) override
    {
        Block::setPos(mousePos);
        if(connector)
            connector->setOutputPos(ImVec2{ pos.x, pos.y + rowHeight * 1.5f });
        input->setPos(ImVec2{ pos.x, pos.y + rowHeight * 1.5f });
    }

    std::shared_ptr<BezierCurve> getConnector()
    {
        return connector;
    }

    bool dropAreaInTouch(ImVec2 mousePos)
    {
        return input->isInTouch(mousePos);
    }

    bool interact(ImVec2 mousePos) override
    {
        if(Block::isInTouch(mousePos))
        {
            setPos(mousePos);
            return true;
        }
        return false;
    }

    void draw(ImDrawList* drawList) override
    {
        Block::draw(drawList);
        input->draw(drawList);
        if(connector)
            connector->draw(drawList);
    }
};