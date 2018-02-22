#include "CrossPlatformReadWriteLocker.h"

ReadWriteLocker::ReadWriteLocker()
{

#ifdef WIN32
InitializeSRWLock(&lock);
#else
pthread_rwlock_init(&lock,NULL);
#endif
}


ReadWriteLocker::~ReadWriteLocker()
{
#ifdef WIN32
	//nothing to be done in windows
#else
pthread_rwlock_destroy(&lock);
#endif
}


void ReadWriteLocker::ReaderLock()
{
#ifdef WIN32
	AcquireSRWLockShared(&lock);
#else
	pthread_rwlock_rdlock(&lock);
#endif
}


void ReadWriteLocker::WriterLock()
{
#ifdef WIN32
	AcquireSRWLockExclusive(&lock);
#else
	pthread_rwlock_wrlock(&lock);
#endif
}


void ReadWriteLocker::ReaderUnlock()
{
#ifdef WIN32
	ReleaseSRWLockShared(&lock);
#else
	pthread_rwlock_unlock(&lock);
#endif
}


void ReadWriteLocker::WriterUnlock()
{
#ifdef WIN32
	_Requires_lock_held_(&lock);
	ReleaseSRWLockExclusive(&lock);
#else
	pthread_rwlock_wrlock(&lock);
#endif
}
