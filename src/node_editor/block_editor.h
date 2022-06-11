#pragma once

#include <deque>
#include <list>
#include <string>
#include <vector>

#include <imgui.h>

static ImVec2 pos2reg(ImVec2 wp, ImVec2 p);
static ImVec2 pos2gl(ImVec2 wp, ImVec2 p);
struct connection;
struct block
{
    ImVec2 pos;
    ImU32 color;
    void draw(ImDrawList* drawList, ImVec2 windowPos);
};
struct effect_block : public block
{
    ImVec2 lastDelta{ 0.f, 0.f };
    bool isBeingDragged = false;
    connection* attachedTo{ nullptr };
    const char* currentItem = NULL;

    bool interact();
};
struct connection
{
    block in;
    block out;

    std::list<effect_block*> effects;
    size_t channelIdx, clipHandle;

    void draw(ImDrawList* drawList, ImVec2 windowPos, ImVec2 windowSize);
    void interact(size_t& i, std::list<effect_block>& freeEffects);
    void add_effect(effect_block* ef);
    void remove_effect(effect_block* ef);

    bool is_contained(ImVec2 pos);
};
class block_editor
{
  public:
    static constexpr ImVec2 nodeSize{ 150.f, 50.f };
    static constexpr float socketSize{ 10.f };
    static constexpr float nodeYMargin{ 25.f };

  public:
    block_editor();
    ~block_editor();

    void add_effect();
    void add_connection(size_t channelIdx, size_t clipHandle);
    void draw();

  private:
    std::deque<connection> connections;
    std::list<effect_block> effects;
};