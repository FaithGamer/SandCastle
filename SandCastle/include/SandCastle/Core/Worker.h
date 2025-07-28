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

		void StartThread();
		void StopThread();
		void QueueTask(sptr<OpaqueTask> taskdata);
		bool HaveTask();
		size_t TaskCount();

	private:
		void Thread();

	protected:


	private:

		std::condition_variable m_waiter;
		std::mutex m_queueMutex[2];
		std::mutex m_waiterMutex;
		std::vector<sptr<OpaqueTask>> m_queue[2];
		std::thread m_thread;
		bool m_currentQueue;
		bool m_taskAvailable;
		std::atomic<bool> m_haveTask;
		std::atomic<bool> m_threadRunning;
	};


}