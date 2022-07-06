#pragma once

#include <functional>
#include <queue>
#include <string>
#include <unordered_map>

struct command
{
    std::string name;
    void* data{};
};
class command_handler
{
  public:
    static void send(const command& com)
    {
        commands.push(com);
    }
    static void listen(const std::string& commandName, const std::function<void(command&)>& callback)
    {
        handler[commandName] = callback;
    }
    static void update()
    {
        while(commands.size())
        {
            auto& com = commands.front();
            if(auto listener = handler.find(com.name); listener != handler.end())
            {
                listener->second(com);
            }
            delete com.data;
            commands.pop();
        }
    }

  private:
    inline static std::queue<command> commands;
    inline static std::unordered_map<std::string, std::function<void(command&)>> handler;
};
