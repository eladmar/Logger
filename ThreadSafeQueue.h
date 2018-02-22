#ifndef	__THREADSAFEQUEUE_H__
#define __THREADSAFEQUEUE_H__

#include <mutex>
#include <condition_variable>
#include <queue>

template <class T>
class ThreadSafeQueue
{
public:
    // Push value to queue.
    void Push(const T& value)
    {
        std::unique_lock<std::mutex> lock(mtx);
		queue.push(value);
		condVar.notify_one();
    }

    // Push to queue with rvalue reference.
    void Push(T && value)
    {
        std::unique_lock<std::mutex> lock(mtx);
		queue.push(std::move(value));
		condVar.notify_one();
    }

    // Pop element from queue and write to value.
	// will block until the queue will not be empty
    void Pop(T& value)
    {
        std::unique_lock<std::mutex> lock(mtx);
        // If there is no item then we wait until there is one.
        while (queue.empty())
		{
            condVar.wait(lock);
		}
		value = queue.front();
		queue.pop();
    }

	// try to pop element from queue and write to value.
	// if the queue empty will return  without operation on value and return with false
	// if the queue not empty will pop element from queue and write to value.
	bool TryPop(T& value)
	{
		std::unique_lock<std::mutex> lock(mtx);
		if(queue.empty())
		{
			return false;
		}
		value = queue.front();
		queue.pop();
		return true;
	}


private:
    // The condition variable to use for wait on empty queue when popping.
    std::condition_variable condVar;
    // The mutex for locking the queue.
    std::mutex mtx;
    std::queue<T> queue;
};


#endif //__THREADSAFEQUEUE_H__
