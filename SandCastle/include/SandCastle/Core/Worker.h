#pragma once

#include <thread>
#include <condition_variable>

#include "SandCastle/Core/std_macros.h"
#include "SandCastle/Core/Task.h"

namespace SandCastle
{
	/// @brief Run given task within it's own thread
	///Thread running only when tasks available in the queue, adding tasks to the queue is not blocking
	class WorkerThread
	{
	public:
		WorkerThread();
		~WorkerThread();

		/// @brief Tasks will be proceeded.
		/// Internally create a new thread 
		/// You wan still queue new tasks
		void StartThread();

		/// @brief Stop Task processing.
		/// internally join the thread.
		/// Usually not necessary to StopThread unless you never need it anymore.
		/// You can still queue tasks but they won't be proceeded until you StartThread() again.
		void StopThread();

		/// @brief Queue any method
		/// @param method method to call
		/// @param object instance to call the method on
		/// @param ...args arguments for the method
		template<typename Obj>
		void Queue(void(Obj::* method)(void), Obj* object)
		{
			auto del = Delegate<void, Obj>(method, object);
			Queue(makesptr<Task<Obj>>(del));
		}

		void Queue(void(*function)(void))
		{
			auto del = Delegate<void, FunctionDelegate>(function);
			Queue(makesptr<Task<FunctionDelegate>>(del));
		}

		template<typename Obj, typename... Args, typename... Ts>
		void Queue(void(Obj::* method)(Args...), Obj* object, Ts&&... args)
		{
			auto del = Delegate(method, object, std::forward<Ts>(args)...);
			Queue(makesptr<Task<Obj, Args...>>(del));
		}

		/// @brief Queue any function (or static method)
		/// @param function function to call
		/// @param object instance to call the function on
		/// @param ...args arguments for the function
		template<typename... Args, typename... Ts>
		void Queue(void(*function)(Args...), Ts&&... args)
		{
			auto del = Delegate(function, std::forward<Ts>(args)...);
			Queue(makesptr<Task<FunctionDelegate, Args...>>(del));
		}

		void Queue(sptr<OpaqueTask> task);

		bool HaveTask();
		size_t TaskCount();

		/// @brief Block the calling thread until there's no more task to process
		void Wait();

	private:
		void Thread();
	private:
		std::condition_variable m_waiter;
		std::condition_variable m_doneCondition;
		std::mutex m_queueMutex[2];
		std::mutex m_waiterMutex;
		std::mutex m_doneMutex;
		std::vector<sptr<OpaqueTask>> m_queue[2];
		std::thread m_thread;
		bool m_currentQueue;
		bool m_taskAvailable;
		std::atomic<bool> m_haveTask;
		std::atomic<bool> m_threadRunning;
	};

}