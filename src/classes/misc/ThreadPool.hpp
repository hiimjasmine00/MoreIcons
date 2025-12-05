#include <queue>
#include <std23/move_only_function.h>
#include <thread>
#include <mutex>

class ThreadPool {
private:
    size_t m_threadsInit = 0;
    size_t m_threadsMax = 0;
    size_t m_tasksBusy = 0;
    std::mutex m_mutex;
    std::queue<std23::move_only_function<void()>> m_tasks;
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

    void pushTask(std23::move_only_function<void()> task);
    void wait();

    ~ThreadPool();
};
