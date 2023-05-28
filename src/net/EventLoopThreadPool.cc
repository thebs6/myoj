

#include "EventLoopThread.h"
#include "EventLoop.h"
#include "EventLoopThreadPool.h"
#include "Logging.h"

EventLoopThreadPool::EventLoopThreadPool(EventLoop* loop, const std::string& nameArg)
    : baseloop_(loop)
    , name_(nameArg)
    , started_(false)
    , numThreads_(0)
    , next_(0)
{}

EventLoopThreadPool::~EventLoopThreadPool()
{}

bool setThreadAffinity(int i) {
    cpu_set_t mask;
    CPU_ZERO(&mask);
    if (i >= 0) {
        CPU_SET(i, &mask);
    } else {
        for (auto j = 0u; j < std::thread::hardware_concurrency(); ++j) {
            CPU_SET(j, &mask);
        }
    }
    if (!pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask)) {
        return true;
    }
    LOG_WARN << "pthread_setaffinity_np failed: ";
    return false;
}

// 开启线程池
void EventLoopThreadPool::start(const ThreadInitCallBack &cb)
{
    auto cpus = std::thread::hardware_concurrency();
    numThreads_ = numThreads_ >= 0 ? numThreads_ : cpus;

    started_ = true;
    // 循环开启numThreads_个子线程
    for(int i = 0; i < numThreads_; i++)
    {   
        // 设定子线程的名称: “线程池名称线程索引”
        char buf[name_.size() + 32];
        snprintf(buf, sizeof buf, "%s%d", name_.c_str(), i);
        // 给每个子线程传递ThreadInitCallBack

        ThreadInitCallBack new_cb;
        if(enable_cpu_affinity_) {
            new_cb = std::move(
                [cpus,i, &cb](EventLoop* loop) {
                    setThreadAffinity(i % cpus);
                    if(cb) {
                        cb(loop);
                    }
                }
            );
        }

        EventLoopThread *t = new EventLoopThread(new_cb, buf);
        threads_.push_back(std::unique_ptr<EventLoopThread>(t));
        // 运行子线程，返回子线程的eventloop指针
        loops_.push_back(t->startLoop());
    }

    // 只有一个线程
    if(numThreads_ == 0 && cb)
    {
        cb(baseloop_);
    }
}

// 获取下一个EventLoop，采用轮询算法
EventLoop* EventLoopThreadPool::getNextLoopByRB()
{
    EventLoop* loop = baseloop_;
    // 如果loops为空，说明只有一个线程，直接返回baseloop_
    if(!loops_.empty())
    {
        // 循环获取下一个EventLoop
        loop = loops_[next_++];
        if(next_ >= loops_.size())
        {
            next_ = 0;
        }
    }   
    return loop;
}

// 通过最小负载分配从reactor
EventLoop* EventLoopThreadPool::getNextLoopByMinLoad()
{
    EventLoop* loop = baseloop_;
    // 如果loops为空，说明只有一个线程，直接返回baseloop_
    if(!loops_.empty())
    {
        auto thread_pos = _thread_pos;
        if (thread_pos >= loops_.size()) {
            thread_pos = 0;
        }

        auto executor_min_load = loops_[thread_pos];
        auto min_load = executor_min_load->load();

        for (size_t i = 0; i < loops_.size(); ++i, ++thread_pos) {
            if (thread_pos >= loops_.size()) {
                thread_pos = 0;
            }

            auto th = loops_[thread_pos];
            auto load = th->load();

            if (load < min_load) {
                min_load = load;
                executor_min_load = th;
            }
            if (min_load == 0) {
                break;
            }
        }
        _thread_pos = thread_pos;
        return executor_min_load;
    }   
    return loop;
}

// 获取线程池中所有的EventLoop*
std::vector<EventLoop*> EventLoopThreadPool::getAllLoops()
{
    if(loops_.empty())
    {
        return std::vector<EventLoop*>(1,baseloop_);
    }
    else
    {
        return loops_;
    }
}