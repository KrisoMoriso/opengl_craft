#pragma once
#include <condition_variable>
#include <functional>
#include <queue>
#include <thread>
#include <vector>


class ThreadPool {
public:
    //safe queue for threads
    template<typename T>
    class SafeQueue{
    public:
        void push(T item){
            std::lock_guard<std::mutex> lock(m_mutex);
            m_queue.push(std::move(item));
        }
        bool try_pop(T& item){
            std::lock_guard<std::mutex> lock(m_mutex);
            if (m_queue.empty()) return false;
            item = std::move(m_queue.front());
            m_queue.pop();
            return true;
        }
    private:
        std::queue<T> m_queue;
        std::mutex m_mutex;
    };
    ThreadPool(size_t threads);
    ~ThreadPool();
    void enqueue(std::function<void()> task);
private:
    bool stop;
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
};
