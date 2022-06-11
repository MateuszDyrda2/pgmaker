#pragma once

#include "FilterBlock.h"
#include "InputBlock.h"
#include "OutputBlock.h"
#include "events.h"

#define LEFT_MOUSE_BUTTON 0

class BlockEditor
{
    float x;
    float y;
    std::shared_ptr<Block> draggedBlock;

    std::list<std::shared_ptr<InputBlock>> inputBlocks;
    std::list<std::shared_ptr<OutputBlock>> outputBlocks;
    std::list<std::shared_ptr<FilterBlock>> filterBlocks;

    float lastY;

  public:
    BlockEditor(float _x, float _y):
        x(_x), y(_y), draggedBlock(nullptr)
    {
        lastY = _y;
        event_handler::subscribe(
            "TimelineClipAppended",
            std::function<void(size_t, size_t)>([this](size_t channelIdx, size_t clipHandle) {
                this->add_block_pair(channelIdx, clipHandle);
            }));
    }
    void add_block_pair(size_t channelIdx, size_t clipHandle)
    {
        lastY += 100;
        inputBlocks.push_back(std::make_shared<InputBlock>(ImVec2(x, lastY), 100, 25, 0xFF00FF7F));
        outputBlocks.push_back(std::make_shared<OutputBlock>(ImVec2(x + 800, lastY), 100, 25, 0xFF00FF7F));
        auto& ib = inputBlocks.back();
        auto& ob = outputBlocks.back();
        ob->setInputBlock(ib);
        ib->addConnector(ob->getConnector());
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
        for(auto block = inputBlocks.rbegin(); block != inputBlocks.rend(); ++block)
        {
            block->get()->draw(drawList);
        }
        for(auto block = outputBlocks.rbegin(); block != outputBlocks.rend(); ++block)
        {
            block->get()->draw(drawList);
        }
        for(auto block = filterBlocks.rbegin(); block != filterBlocks.rend(); ++block)
        {
            block->get()->draw(drawList);
        }
    }
};