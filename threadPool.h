#pragma once
#include <functional>
#include <vector>
#include <deque>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>

template <typename Ret>
class ThreadPool
{
    typedef std::function<Ret()> Func;
public:
    ThreadPool() :running_(false), maxTask_(0), mutex_()
    {
    }
    ~ThreadPool()
    {
        if (running_)
            stop();
    }
    void start(int numThreads)
    {
        running_ = true;
        threads_.reserve(numThreads);
        for (int i = 0; i < numThreads; ++i)
        {
            threads_.emplace_back(new std::thread(std::bind(&ThreadPool<Ret>::workerThread, this)));
        }
    }
    void stop()
    {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            running_ = false;
            notEmpty_.notify_all();
        }
        for (auto t : threads_)
        {
            t->join();
        }
    }

    template<typename FUNC, typename... Args>
    void run(FUNC && task, Args && ...args)
    {
        if (threads_.empty())
        {
            task(std::forward<Args>(args)...);
        }
        else
        {
            std::unique_lock<std::mutex> lock(mutex_);
            while (isFull())
            {
                notFull_.wait(lock);
            }
            Func f = std::bind(task, std::forward<Args>(args)...);
            tasks_.push_back(std::move(f));
            notEmpty_.notify_one();
        }
    }

    void setMaxTask(int maxTask)
    {
        maxTask_ = maxTask;
    }
private:
    bool isFull()
    {
        return maxTask_ > 0 && tasks_.size() >= maxTask_;
    }
    void workerThread()
    {
        while (running_)
        {
            bool flag = false;
            Func task;
            {
                std::unique_lock<std::mutex> lock(mutex_);
                while (tasks_.empty() && running_)
                {
                    notEmpty_.wait(lock);
                }

                if (!tasks_.empty())
                {
                    task = tasks_.front();
                    tasks_.pop_front();
                    flag = true;
                    if (maxTask_ > 0)
                        notFull_.notify_one();
                }
            }
            if (flag)
                task();
        }
    }

private:
    std::vector<std::shared_ptr<std::thread> > threads_;
    std::deque<Func> tasks_;
    bool running_;
    unsigned int maxTask_;

    std::mutex mutex_;
    std::condition_variable notFull_;
    std::condition_variable notEmpty_;
};
