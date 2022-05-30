#pragma once

#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <queue>

namespace libpgmaker {
template<class T, std::size_t N>
class spsc_queue
{
  public:
    using self_type       = spsc_queue<T, N>;
    using size_type       = std::size_t;
    using value_type      = T;
    using reference       = T&;
    using const_reference = const T&;
    using pointer         = T*;
    using const_pointer   = const T*;
    using difference_type = std::ptrdiff_t;

  public:
    bool try_pop(T& obj);
    bool try_pop(T& obj, const std::chrono::milliseconds& timeout);
    template<class F>
    void pop(T& obj, F cond);
    template<class F>
    void push(const T& obj, F cond);
    void pop();

    void pop(T& obj);
    void push(const T& obj);
    size_type size() const;
    bool empty() const;
    void notify();
    void flush();

    bool top(T& obj);

  private:
    std::queue<T> buffer;
    mutable std::mutex mtx;
    std::condition_variable emptyCvar;
    std::condition_variable fullCvar;
};
template<class T, std::size_t N>
bool spsc_queue<T, N>::top(T& obj)
{
    std::unique_lock lck(mtx);

    if(buffer.empty()) return false;

    obj = buffer.front();
    return true;
}
template<class T, std::size_t N>
void spsc_queue<T, N>::pop()
{
    std::unique_lock lck(mtx);

    buffer.pop();
    fullCvar.notify_one();
}
template<class T, std::size_t N>
bool spsc_queue<T, N>::try_pop(T& obj, const std::chrono::milliseconds& timeout)
{
    std::unique_lock lck(mtx);

    if(!emptyCvar.wait_for(lck, timeout, [this] { return !buffer.empty(); }))
        return false;
    obj = std::move(buffer.front());
    fullCvar.notify_one();
    buffer.pop();
    return true;
}
template<class T, std::size_t N>
bool spsc_queue<T, N>::try_pop(T& obj)
{
    std::lock_guard lck(mtx);

    if(buffer.empty())
        return false;

    obj = std::move(buffer.front());
    buffer.pop();
    fullCvar.notify_one();
    return true;
}
template<class T, std::size_t N>
void spsc_queue<T, N>::pop(T& obj)
{
    std::unique_lock lck(mtx);
    while(buffer.empty())
    {
        emptyCvar.wait(lck);
    }
    obj = std::move(buffer.front());
    buffer.pop();
    fullCvar.notify_one();
}
template<class T, std::size_t N>
void spsc_queue<T, N>::push(const T& obj)
{
    std::unique_lock lck(mtx);
    while(buffer.size() >= N - 1)
    {
        fullCvar.wait(lck);
    }
    buffer.push(obj);
    emptyCvar.notify_one();
}
template<class T, std::size_t N>
template<class F>
void spsc_queue<T, N>::pop(T& obj, F cond)
{
    {
        std::unique_lock lck(mtx);
        while(buffer.empty() && !cond())
        {
            emptyCvar.wait(lck);
        }
        if(buffer.empty()) return;
        obj = std::move(buffer.front());
        buffer.pop();
    }
    fullCvar.notify_one();
}
template<class T, std::size_t N>
template<class F>
void spsc_queue<T, N>::push(const T& obj, F cond)
{
    {
        std::unique_lock lck(mtx);
        while(buffer.size() >= N - 1 && !cond())
        {
            fullCvar.wait(lck);
        }
        if(buffer.size() >= N - 1) return;
        buffer.push(obj);
    }
    emptyCvar.notify_one();
}
template<class T, std::size_t N>
typename spsc_queue<T, N>::size_type
spsc_queue<T, N>::size() const
{
    std::lock_guard lck(mtx);
    return buffer.size();
}
template<class T, std::size_t N>
bool spsc_queue<T, N>::empty() const
{
    std::lock_guard lck(mtx);
    return buffer.empty();
}
template<class T, std::size_t N>
void spsc_queue<T, N>::notify()
{
    fullCvar.notify_one();
    emptyCvar.notify_one();
}
template<class T, std::size_t N>
void spsc_queue<T, N>::flush()
{
    std::lock_guard lck(mtx);
    std::queue<T>().swap(buffer);
}
} // namespace libpgmaker
