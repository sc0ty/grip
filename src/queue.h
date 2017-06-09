#ifndef __QUEUE_H__
#define __QUEUE_H__

#include "sem.h"
#include <queue>
#include <mutex>

template <typename T>
class Queue
{
	public:
		Queue() : m_added(0), m_done(false)
		{}

		~Queue()
		{
			m_sem.post();
		}

		void put(const T &element)
		{
			{
				std::unique_lock<std::mutex> lock(m_mutex);
				m_elements.push(element);
				m_added++;
			}
			m_sem.post();
		}

		bool get(T &element)
		{
			std::unique_lock<std::mutex> lock(m_mutex);
			while (m_elements.empty())
			{
				if (m_done)
					return false;

				lock.unlock();
				m_sem.wait();
				lock.lock();
			}

			element = m_elements.front();
			m_elements.pop();
			return true;
		}

		void done()
		{
			{
				std::unique_lock<std::mutex> lock(m_mutex);
				m_done = true;
			}
			m_sem.post();
		}

		void wait()
		{
			std::unique_lock<std::mutex> lock(m_mutex);
			while (!m_done)
			{
				lock.unlock();
				m_sem.wait();
				lock.lock();
			}
		}

		bool getStats(size_t &added, size_t &removed)
		{
			std::unique_lock<std::mutex> lock(m_mutex);
			added = m_added;
			removed = m_added - m_elements.size();
			return m_done;
		}

	private:
		std::mutex m_mutex;
		Semaphore m_sem;
		std::queue<T> m_elements;
		size_t m_added;
		bool m_done;
};

#endif
