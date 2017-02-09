#pragma once
#include <functional>
#include <vector>
#include <deque>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>

template <typename Ret, typename... Args> 
class ThreadPool
{
	typedef std::function<Ret()> Func;
public:
	typedef std::function<Ret(Args...)> Task;
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
			threads_.push_back(std::shared_ptr<std::thread>(new std::thread(std::bind(&ThreadPool<Ret, Args...>::workerThread, this))));
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

	void run(const Task & task, Args && ...args)
	{
		if (threads_.empty())
		{
			task(args...);
		}
		else
		{
			std::unique_lock<std::mutex> lock(mutex_);
			while (isFull())
			{
				notFull_.wait(lock);
			}
			Func f = [task, &args...]{ return task(std::forward<Args>(args)...); };
			tasks_.push_back(f);
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
			std::unique_lock<std::mutex> lock(mutex_);
			while (tasks_.empty() && running_)
			{
				notEmpty_.wait(lock);
			}

			if (!tasks_.empty())
			{
				auto task = tasks_.front();
				tasks_.pop_front();
				if (maxTask_ > 0)
					notFull_.notify_one();
				task();
			}
		}
	}
public:
private:
	std::vector<std::shared_ptr<std::thread> > threads_;
	std::deque<Func> tasks_;
	bool running_;
	int maxTask_;

	std::mutex mutex_;
	std::condition_variable notFull_;
	std::condition_variable notEmpty_;
};
