
#include "Logging.h"
#include <unordered_map>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>

using Task = std::function<void()>;
class WorkThread {
public:
    explicit WorkThread(int id) : threadKey(id), stop(false) {
        
    }

	void start() {
		thread = std::thread([this]() {
            while (true) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(mutex);
                    condition.wait(lock, [this]() { return stop || !tasks.empty(); });
                    if (stop && tasks.empty())
                        return;
                    task = std::move(tasks.front());
                    tasks.pop();
                }
				LOG_DEBUG << "Thread " << threadKey << " executing task." ;
                task();
            }
        });
	}

    void submitTask(std::function<void()> task) {
        {
            std::lock_guard<std::mutex> lock(mutex);
            tasks.emplace(std::move(task));
        }
        condition.notify_one();
    }

    void stopThread() {
        {
            std::lock_guard<std::mutex> lock(mutex);
            stop = true;
        }
        condition.notify_one();
        if (thread.joinable())
            thread.join();
    }

private:
    int threadKey;
    std::thread thread;
    std::queue<Task> tasks;
    std::mutex mutex;
    std::condition_variable condition;
    bool stop;
};

class WorkThreadPool {
public:
    explicit WorkThreadPool(size_t numThreads) : numThreads_(numThreads) {
        for (size_t i = 0; i < numThreads; ++i) {
            threads_.insert({i, std::unique_ptr<WorkThread>(new WorkThread(i))});
        }
    }

    // template<typename Task>
    void submitTask(int threadId, Task task) {
        if (threads_.find(threadId) != threads_.end()) {
            threads_[threadId]->submitTask(std::move(task));
        } else {
            LOG_DEBUG << "Invalid thread id." ;
        }
    }

	void start() {
        for (const auto& thread : threads_) {
            thread.second->start();
        }
    }

    void stopThreads() {
        for(int i = 0; i < threads_.size(); i++) {
            threads_[i]->stopThread();
        }
    }
	
	size_t numThreads() const {
		return numThreads_;
	}

	int hash(const std::string key) {
		return std::hash<std::string>{}(key) % numThreads_;
	}

private:
    std::unordered_map<int, std::unique_ptr<WorkThread>> threads_;
	size_t numThreads_;
};