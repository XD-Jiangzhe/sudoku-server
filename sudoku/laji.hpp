#ifndef THREAD_POOL
#define THREAD_POOL

#include <mutex>
#include <iostream>
#include <functional>
#include <deque>
#include <vector>
#include <memory>
#include <thread>
#include <condition_variable>
#include <cassert>

#include <vector>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/assert.hpp>

namespace tp
{
	typedef std::function<void()> TASK;
	class threadpool
	{
		public:
			threadpool(int max_sz, const std::string& name):max_size(max_sz), name_(name), running(true){}
			~threadpool();

			void start();

			void run_in_thread();
			TASK get_task();
			void push_task(TASK task);

			void stop();

		private:
			boost::ptr_vector<std::thread> threads_;
			std::deque<TASK> tasks_;

			bool running;

			std::mutex mut;
			std::condition_variable cv;

			std::string name_;
			int max_size;
	};

	threadpool::~threadpool()
	{
		if(running)
			stop();
	}

	void threadpool::start()
	{
		for(int i=0; i< max_size; i++)
			threads_.push_back(new std::thread(std::bind(&threadpool::run_in_thread, this)));		
	}

	void threadpool::run_in_thread()
	{
		TASK task;
		while(running)
		{
			task = get_task();
			if(task == NULL)
				return ;
			task();
		}
	}

	TASK threadpool::get_task()
	{
		TASK task;
		std::unique_lock<std::mutex> ul(mut);
		if(tasks_.empty())
		{
			cv.wait(ul);
			if(!running)
				return NULL;
		}
		
		task = tasks_.front();
		tasks_.pop_front();
		return task;
	}

	void threadpool::push_task(TASK task)
	{
		std::unique_lock<std::mutex> ul(mut);
		if(tasks_.empty())
		{
			tasks_.push_back(task);
			cv.notify_one();
		}
		else
		{
			std::cout<<"push success"<<std::endl;
			tasks_.push_back(task);
		}	
	}

	void threadpool::stop()
	{
		running = false;
		{
			std::unique_lock<std::mutex> ul(mut);
			cv.notify_all();
		}
		for(int i=0;i< max_size;i++)
			threads_[i].join();
	}

}

#endif
/*
int a = 0;
std::mutex m;

void add()
{
	std::lock_guard<std::mutex> l(m);
	std::cout<<++a<<std::endl;
}

int main()
{
	tp::threadpool trep(10, "hahaha");
	
	trep.start();

	for(int i=0;i<100; i++)
		trep.push_task(add);

	while(1)
	{
		m.lock();
		if(a == 100)
			break;
		m.unlock();
	}
}*/