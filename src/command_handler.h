#pragma once

#include <functional>
#include <queue>
#include <string>
#include <unordered_map>

class command_handler
{
  public:
    static void send(const std::string& command)
    {
        commands.push(command);
    }
    static void listen(const std::string& command, const std::function<void()>& callback)
    {
        handler[command] = callback;
    }
    static void update()
    {
        while(commands.size())
        {
            auto& command = commands.front();
            if(auto listener = handler.find(command); listener != handler.end())
            {
                listener->second();
            }
            commands.pop();
        }
    }

  private:
    inline static std::queue<std::string> commands;
    inline static std::unordered_map<std::string, std::function<void()>> handler;
};
