#include "pch.h"
#include "SandCastle/Core/Worker.h"

namespace SandCastle
{
	WorkerThread::WorkerThread() :
		m_currentQueue(false),
		m_threadRunning(false),
		m_taskAvailable(false),
		m_haveTask(false)
	{

	}
	WorkerThread::~WorkerThread()
	{
		StopThread();
	}

	void WorkerThread::StartThread()
	{
		m_threadRunning = true;
		m_thread = std::move(std::thread(&WorkerThread::Thread, this));
	}

	void WorkerThread::StopThread()
	{
		m_threadRunning = false;
		std::unique_lock lock(m_waiterMutex);
		m_taskAvailable = true;
		lock.unlock();
		m_waiter.notify_one();
		if (m_thread.joinable())
			m_thread.join();
	}

	void WorkerThread::Queue(sptr<OpaqueTask> task)
	{
		// Lock m_waiterMutex BEFORE reading m_currentQueue: Thread() flips it
		// under that mutex, so reading it first could pick (and lock) one queue
		// while emplacing into the other — the one the worker is iterating.
		std::unique_lock waiterLock(m_waiterMutex);
		const size_t back = (size_t)!m_currentQueue;
		std::unique_lock queueLock(m_queueMutex[back]);

		m_queue[back].emplace_back(task);

		m_taskAvailable = true;
		m_haveTask = true;
		queueLock.unlock();
		waiterLock.unlock();

		m_waiter.notify_one();
	}

	bool WorkerThread::HaveTask()
	{
		std::unique_lock waiterLock(m_waiterMutex);
		return m_haveTask;
	}

	size_t WorkerThread::TaskCount()
	{
		std::scoped_lock lock(m_queueMutex[0], m_queueMutex[1]);
		return m_queue[0].size() + m_queue[1].size();
	}

	void WorkerThread::Wait()
	{
		std::unique_lock lock(m_doneMutex);
		m_doneCondition.wait(lock, [this]() {
			return !m_haveTask;
			});
	}

	void WorkerThread::Thread()
	{
		while (m_threadRunning)
		{
			std::unique_lock waiterLock(m_waiterMutex);
			m_waiter.wait(waiterLock, [this] {return m_taskAvailable; });
			m_currentQueue = !m_currentQueue;
			m_haveTask = true;
			m_taskAvailable = false;
			waiterLock.unlock();

			std::lock_guard queueLock(m_queueMutex[(size_t)m_currentQueue]);
			
			for (auto& task : m_queue[(size_t)m_currentQueue])
			{
				if (!m_threadRunning)
					break;
				task->Perform();
			}

			m_queue[m_currentQueue].clear();
			{
				// The back buffer may be receiving a concurrent Queue(); lock it
				// so the emptiness check can't read the vector mid-mutation and
				// wrongly report "done" while a task is pending. The current
				// buffer was just cleared under its (held) mutex.
				std::lock_guard backLock(m_queueMutex[(size_t)!m_currentQueue]);
				m_haveTask = !m_queue[(size_t)!m_currentQueue].empty();
			}

			if (!m_haveTask)
			{
				std::unique_lock doneLock(m_doneMutex);
				m_doneCondition.notify_all();
			}
		}
		std::unique_lock queueLock1(m_queueMutex[(size_t)m_currentQueue]);
		m_queue[m_currentQueue].clear();
		queueLock1.unlock();
		std::unique_lock queueLock2(m_queueMutex[(size_t)!m_currentQueue]);
		m_queue[!m_currentQueue].clear();
		queueLock2.unlock();
	}
}

