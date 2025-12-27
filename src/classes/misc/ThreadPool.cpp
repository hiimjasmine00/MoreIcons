#include "ThreadPool.hpp"
#include <algorithm>

ThreadPool& ThreadPool::get() {
    static ThreadPool instance;
    return instance;
}

ThreadPool::ThreadPool() {
    m_threadsMax = std::max(std::thread::hardware_concurrency() - 2, 2u);
    m_threads = std::make_unique<std::thread[]>(m_threadsMax);
    m_threadsBusy = std::make_unique<std::atomic_bool[]>(m_threadsMax);
}

void ThreadPool::pushTask(std23::move_only_function<void()> task) {
    std::unique_lock lock(m_mutex);

    if (std::all_of(m_threadsBusy.get(), m_threadsBusy.get() + m_threadsInit, std::identity())) {
        tryAllocThread();
    }

    m_tasks.emplace(std::move(task));
    m_condvar.notify_one();
}

void ThreadPool::threadFunc(size_t idx) {
    while (!m_requestedStop) {
        m_threadsBusy[idx] = false;

        std23::move_only_function<void()> task;

        {
            std::unique_lock lock(m_mutex);

            if (m_tasks.empty()) {
                m_condvar.wait_for(lock, std::chrono::milliseconds(50), [this] { return !m_tasks.empty(); });
                if (m_tasks.empty()) continue;
            }

            m_threadsBusy[idx] = true;

            task = std::move(m_tasks.front());
            m_tasks.pop();

            m_tasksBusy++;
        }

        task();

        {
            std::unique_lock lock(m_mutex);
            m_tasksBusy--;
            if (m_tasks.empty() && m_tasksBusy == 0) m_condvar.notify_all();
        }
    }
}

void ThreadPool::tryAllocThread() {
    if (m_threadsInit >= m_threadsMax) return;

    m_threads[m_threadsInit] = std::thread(&ThreadPool::threadFunc, this, m_threadsInit);
    m_threadsInit++;
}

void ThreadPool::wait() {
    std::unique_lock lock(m_mutex);

    m_condvar.wait(lock, [this] { return m_tasks.empty() && m_tasksBusy == 0; });
}

ThreadPool::~ThreadPool() {
    m_requestedStop.store(true);

    for (size_t i = 0; i < m_threadsInit; i++) {
        if (m_threads[i].joinable()) m_threads[i].join();
    }
}
