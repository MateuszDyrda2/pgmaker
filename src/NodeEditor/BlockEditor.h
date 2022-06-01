#pragma once

#include "FilterBlock.h"
#include "InputBlock.h"
#include "OutputBlock.h"

#define LEFT_MOUSE_BUTTON 0

class BlockEditor
{
    float x;
    float y;
    std::shared_ptr<Block> draggedBlock;

    std::list<std::shared_ptr<InputBlock>> inputBlocks;
    std::list<std::shared_ptr<OutputBlock>> outputBlocks;
    std::list<std::shared_ptr<FilterBlock>> filterBlocks;

  public:
    BlockEditor(float _x, float _y, int numberOfInputAndOutputBlocks):
        x(_x), y(_y), draggedBlock(nullptr)
    {
        for(int i = 0; i < numberOfInputAndOutputBlocks; i++)
        {
            inputBlocks.push_back(std::make_shared<InputBlock>(ImVec2{ x, y + i * 100 }, 100, 25, 0xFF00FF7F));
            outputBlocks.push_back(std::make_shared<OutputBlock>(ImVec2{ x + 800, y + i * 100 }, 100, 25, 0xFF00FF7F));
            filterBlocks.push_back(std::make_shared<FilterBlock>(ImVec2{ x + 400, y + i * 100 }, 100, 25, 0xFF00FF7F));
        }
    }

    void interact()
    {
        auto& io = ImGui::GetIO();
        if(io.MouseDown[LEFT_MOUSE_BUTTON])
        {
            for(auto block : inputBlocks)
            {
                if(block == draggedBlock || draggedBlock.get() == nullptr)
                {
                    if(block->interact(io.MousePos))
                    {
                        draggedBlock = block;
                    }
                }
            }
            for(auto const& block : outputBlocks)
            {
                if(block == draggedBlock || draggedBlock.get() == nullptr)
                {
                    if(block->interact(io.MousePos))
                    {
                        draggedBlock = block;
                    }
                }
            }
            for(auto const& block : filterBlocks)
            {
                if(block == draggedBlock || draggedBlock.get() == nullptr)
                {
                    if(block->interact(io.MousePos))
                    {
                        draggedBlock = block;
                    }
                }
            }
        }
        else if(io.MouseReleased[LEFT_MOUSE_BUTTON])
        {
            for(auto const& block : outputBlocks)
            {
                if(draggedBlock.get() != nullptr)
                {
                    if(block->dropAreaInTouch(io.MousePos) && draggedBlock->isInUse())
                    {
                        block->setInputBlock(draggedBlock);

                        if(auto t = dynamic_cast<InputBlock*>(draggedBlock.get()))
                        {
                            t->addConnector(block->getConnector());
                        }
                        else if(auto t = dynamic_cast<FilterBlock*>(draggedBlock.get()))
                        {
                            t->addConnector(block->getConnector());
                        }
                    }
                }
                block->releaseUse();
            }
            for(auto const& block : filterBlocks)
            {
                if(draggedBlock.get() != nullptr)
                {
                    if(block->dropAreaInTouch(io.MousePos) && draggedBlock->isInUse())
                    {
                        block->setInputBlock(draggedBlock);

                        if(auto t = dynamic_cast<InputBlock*>(draggedBlock.get()))
                        {
                            t->addConnector(block->getConnector());
                        }
                    }
                }
                block->releaseUse();
            }
            for(auto const& block : inputBlocks)
            {
                block->releaseUse();
            }
            draggedBlock = nullptr;
        }
    }

    void draw(ImDrawList* drawList)
    {
        for(std::list<std::shared_ptr<InputBlock>>::reverse_iterator block = inputBlocks.rbegin(); block != inputBlocks.rend(); block++)
        {
            block->get()->draw(drawList);
        }
        for(std::list<std::shared_ptr<OutputBlock>>::reverse_iterator block = outputBlocks.rbegin(); block != outputBlocks.rend(); block++)
        {
            block->get()->draw(drawList);
        }
        for(std::list<std::shared_ptr<FilterBlock>>::reverse_iterator block = filterBlocks.rbegin(); block != filterBlocks.rend(); block++)
        {
            block->get()->draw(drawList);
        }
    }
};