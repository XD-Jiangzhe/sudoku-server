#ifndef THREAD_POOL
#define THREAD_POOL


#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <deque>
#include <vector>
#include <functional>
#include <string>

using namespace std;
namespace tp
{
	typedef function<void()> TASK;
	class threadpool
	{
		public:

			threadpool(int ,const string);
			~threadpool();

			void run(TASK);							
			void start();
			
			TASK take_task();						//这里从任务队列中取一个任务，需要加锁
			void push_task(TASK task);

			void run_in_thread();

			void stop();					

			bool get_running_state();

			int tasks_length()
			{
				lock_guard<mutex> lg(mu);
				return tasks_.size();
			}

			//查看当前的任务队列是否为空
		private:
			vector<thread> threads_;					//线程池
			deque<TASK> tasks_;							//任务队列

			mutable mutex mu;							//锁住条件变量
			condition_variable wait_task;


			int max_size_;
			string poolname_;
			bool running = true;
	};
}
#endif