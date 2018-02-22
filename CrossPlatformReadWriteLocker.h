#ifndef __CROSSPLATFORMREADWRITELOCKER_H__
#define __CROSSPLATFORMREADWRITELOCKER_H__

#ifdef WIN32
#include <Windows.h>
#define wrC_lock SRWLOCK
#else
#include <pthread.h>
#define wrC_lock pthread_rwlock_t
#endif
#include <iostream>
#include <thread>
class ReadWriteLocker
{
public:
	ReadWriteLocker();
	~ReadWriteLocker();
	void ReaderLock();
	void WriterLock();
	void ReaderUnlock();
	void WriterUnlock();
private:
	wrC_lock lock;

	ReadWriteLocker(const ReadWriteLocker&);//not copyable
	void operator=(const ReadWriteLocker&);//not copyable

};



struct ReaderGuard
{
	inline ReaderGuard(ReadWriteLocker& lock): locker(lock)
	{
		locker.ReaderLock();
	}
	inline ~ReaderGuard()
	{
		locker.ReaderUnlock();
	}
private:
	ReadWriteLocker& locker;
};


struct WriterGuard
{
	inline WriterGuard(ReadWriteLocker& lock): locker(lock)
	{
		locker.WriterLock();
	}
	inline ~WriterGuard()
	{
		locker.WriterUnlock();
	}
private:
	ReadWriteLocker& locker;
};

#endif //__CROSSPLATFORMREADWRITELOCKER_H__
