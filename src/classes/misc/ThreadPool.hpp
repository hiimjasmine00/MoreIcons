#include <functional>
#include <queue>
#include <thread>
#include <mutex>

// https://github.com/geode-sdk/geode/blob/v4.5.0/loader/src/ui/nodes/LazySprite.cpp

class ThreadPool {
private:
    size_t m_threadsInit = 0;
    size_t m_threadsMax = 0;
    size_t m_tasksBusy = 0;
    std::mutex m_mutex;
    std::queue<std::function<void()>> m_tasks;
    std::unique_ptr<std::thread[]> m_threads;
    std::unique_ptr<std::atomic_bool[]> m_threadsBusy;
    std::atomic_size_t m_spinCount = 0;
    std::atomic_bool m_requestedStop;

    void threadFunc(size_t idx);
    void tryAllocThread();
public:
    static ThreadPool& get();

    ThreadPool();
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete;

    void pushTask(std::function<void()> task);
    void wait();

    ~ThreadPool();
};
