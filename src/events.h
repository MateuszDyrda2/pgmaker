#pragma once

#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

class event_handler
{
  public:
    struct event_base
    {
        virtual ~event_base() = default;
    };
    template<class... Args>
    struct event : public event_base
    {
        void fire(Args&&... args)
        {
            for(auto& s : subscribers) { s(args...); }
        }
        void subscribe(const std::function<void(Args...)>& callable)
        {
            subscribers.push_back(callable);
        }

        std::vector<std::function<void(Args...)>> subscribers;
    };
    template<class... Args>
    static void subscribe(const std::string& e, const std::function<void(Args...)>& callable)
    {
        if(auto ev = events.find(e); ev == events.end())
        {
            events.insert(std::make_pair(e, std::make_unique<event<Args...>>()));
        }
        static_cast<event<Args...>*>(events[e].get())->subscribe(callable);
    }
    template<class... Args>
    static inline void fire_event(const std::string& e, Args&&... args)
    {
        if(auto ev = events.find(e); ev != events.end())
        {
            static_cast<event<Args...>*>(ev->second.get())->fire(std::forward<Args>(args)...);
        }
    }

  private:
    static inline std::unordered_map<std::string, std::unique_ptr<event_base>> events;
};
