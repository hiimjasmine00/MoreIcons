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

void ThreadPool::pushTask(std::function<void()> task) {
    std::unique_lock lock(m_mutex);

    if (std::all_of(m_threadsBusy.get(), m_threadsBusy.get() + m_threadsInit, std::identity())) {
        tryAllocThread();
    }

    m_tasks.emplace(std::move(task));
    m_spinCount++;
}

void ThreadPool::threadFunc(size_t idx) {
    while (!m_requestedStop) {
        m_threadsBusy[idx] = false;

        std::function<void()> task;

        {
            std::unique_lock lock(m_mutex);

            if (m_tasks.empty()) {
                lock.unlock();

                while (!m_requestedStop) {
                    auto shouldQuit = false;
                    auto count = m_spinCount.load();
                    while (true) {
                        if (count == 0) break;

                        if (m_spinCount.compare_exchange_weak(count, count - 1)) {
                            shouldQuit = true;
                            break;
                        }
                    }

                    if (shouldQuit) {
                        lock.lock();
                        break;
                    }
                    else {
                        std::this_thread::sleep_for(std::chrono::milliseconds(5));
                        continue;
                    }
                }

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
            if (m_tasks.empty() && m_tasksBusy == 0) m_spinCount.store(0);
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

    while (!m_tasks.empty() || m_tasksBusy > 0) {
        lock.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        lock.lock();
    }
}

ThreadPool::~ThreadPool() {
    m_requestedStop.store(true);

    for (size_t i = 0; i < m_threadsInit; i++) {
        if (m_threads[i].joinable()) m_threads[i].join();
    }
}
