#pragma once

#include <atomic>
#include <condition_variable>
#include <list>
#include <mutex>
#include <vector>

namespace libpgmaker {
template<class T, std::size_t N>
class tsqueue
{
  public:
    using size_type = std::size_t;

  private:
    enum class State
    {
        OPEN,
        CLOSED
    };

  public:
    void push(const T& data);
    void pop(T& data);
    bool top(T& data);
    void pop();
    bool try_push(const T& data);
    bool try_pop(T& data);
    void stop();
    void flush();

  private:
    State state{ State::OPEN };
    size_type currentSize{};
    std::condition_variable cvPush, cvPop;
    std::mutex mtx;
    std::list<T> list;
};
template<class T, std::size_t N>
bool tsqueue<T, N>::try_push(const T& data)
{
    std::unique_lock lck(mtx);
    if(list.size() >= N) return false;
    list.push_back(data);
    if((++currentSize) == 1)
    {
        cvPop.notify_one();
    }
    return true;
}
template<class T, std::size_t N>
bool tsqueue<T, N>::try_pop(T& data)
{
    std::unique_lock lck(mtx);
    if(list.empty()) return false;
    data = list.front();
    list.pop_front();
    currentSize -= 1;
    cvPush.notify_one();
    return true;
}
template<class T, std::size_t N>
void tsqueue<T, N>::stop()
{
    {
        std::unique_lock lck(mtx);
        state = State::CLOSED;
    }
    cvPop.notify_all();
    cvPush.notify_all();
}
template<class T, std::size_t N>
void tsqueue<T, N>::flush()
{
    std::unique_lock lck(mtx);
    list.clear();
    state = State::OPEN;
}
template<class T, std::size_t N>
void tsqueue<T, N>::push(const T& data)
{
    {
        std::unique_lock lck(mtx);
        while(currentSize == N && state != State::CLOSED)
        {
            cvPush.wait(lck);
        }
        if(state == State::CLOSED)
        {
            return;
        }
        list.push_back(data);
        if((++currentSize) == 1u)
        {
            cvPop.notify_one();
        }
    }
}
template<class T, std::size_t N>
void tsqueue<T, N>::pop(T& data)
{
    {
        std::unique_lock lck(mtx);
        while(list.empty() && state != State::CLOSED)
        {
            cvPop.wait(lck);
        }
        if(state == State::CLOSED)
        {
            return;
        }
        currentSize -= 1;
        data = list.front();
        list.pop_front();
        cvPush.notify_one();
    }
}

template<class T, std::size_t N>
bool tsqueue<T, N>::top(T& data)
{
    std::unique_lock lck(mtx);
    if(list.empty()) return false;
    data = list.front();
    return true;
}
template<class T, std::size_t N>
void tsqueue<T, N>::pop()
{
    std::unique_lock lck(mtx);
    list.pop_front();
    currentSize -= 1;
    cvPush.notify_one();
}
} // namespace libpgmaker
