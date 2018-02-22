#ifndef __CROSSPLATFORMREADWRITELOCKER_H__
#define __CROSSPLATFORMREADWRITELOCKER_H__

#ifdef WIN32
#include <Windows.h>
#define LocVer SRWLOCK
#else
#include <pthread.h>
#define LocVer pthread_rwlock_t
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
	LocVer lock;

	ReadWriteLocker(const ReadWriteLocker&);//not copyable
	void operator=(const ReadWriteLocker&);//not copyable

};



struct ReaderGuard
{
	inline ReaderGuard(ReadWriteLocker& lock): locker(lock)
	{
//	std::cout<<"ReaderLock\n";
		locker.ReaderLock();
//	std::cout<<"hereReaderLock\n";
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
	//	std::cout<<"id:" <<std::this_thread::get_id()<<"WriterLock\n";
		locker.WriterLock();
//		std::cout<<"hereWriterLock\n";
	}
	inline ~WriterGuard()
	{
	//	std::cout<<"WriterUnlock\n";
		locker.WriterUnlock();
	//std::cout<<"WriterUnlock ddd\n";
	}
private:
	ReadWriteLocker& locker;
};

#endif //__CROSSPLATFORMREADWRITELOCKER_H__
