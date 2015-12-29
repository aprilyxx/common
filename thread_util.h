#pragma once

#include <cclog/cclog.h>
#include "data_types.h"
#include "closure.h"
#include "scoped_ptr.h"

#ifndef _WIN32
#  include <string.h>
#  include <unistd.h>
#  include <errno.h>
#  include <time.h>
#  include <pthread.h>
#else
#  pragma warning (disable:4355)
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#endif

/******************************=> Mutex & RwLock ******************************/
#ifndef _WIN32 // unix
class Mutex {
  public:
    Mutex() {
        CHECK(pthread_mutex_init(&_mutex, NULL) == 0);
    }
    ~Mutex() {
        CHECK(pthread_mutex_destroy(&_mutex) == 0);
    }

    void Lock() {
        int err = pthread_mutex_lock(&_mutex);
        CHECK_EQ(err, 0)<< ::strerror(err);
    }

    void UnLock() {
        int err = pthread_mutex_unlock(&_mutex);
        CHECK_EQ(err, 0) << ::strerror(err);
    }

    bool TryLock() {
        return pthread_mutex_trylock(&_mutex) == 0;
    }

    pthread_mutex_t* mutex() {
        return &_mutex;
    }

private:
    pthread_mutex_t _mutex;

    DISALLOW_COPY_AND_ASSIGN(Mutex);
};

class RwLock {
  public:
    RwLock() {
        CHECK(pthread_rwlock_init(&_lock, NULL) == 0);
    }
    ~RwLock() {
        CHECK(pthread_rwlock_destroy(&_lock) == 0);
    }

    void ReadLock() {
        int err = pthread_rwlock_rdlock(&_lock);
        CHECK_EQ(err, 0)<< ::strerror(err);
    }

    void WriteLock() {
        int err = pthread_rwlock_wrlock(&_lock);
        CHECK_EQ(err, 0) << ::strerror(err);
    }

    void ReadUnLock() {
        int err = pthread_rwlock_unlock(&_lock);
        CHECK_EQ(err, 0) << ::strerror(err);
    }

    void WriteUnLock() {
        int err = pthread_rwlock_unlock(&_lock);
        CHECK_EQ(err, 0) << ::strerror(err);
    }

    bool TryReadLock() {
        return pthread_rwlock_tryrdlock(&_lock) == 0;
    }

    bool TryWriteLock() {
        return pthread_rwlock_trywrlock(&_lock) == 0;
    }

private:
    pthread_rwlock_t _lock;

    DISALLOW_COPY_AND_ASSIGN(RwLock);
};

#else // win
class Mutex {
  public:
    Mutex() {
        ::InitializeCriticalSection(&_mutex);
    }

    ~Mutex() {
        ::DeleteCriticalSection(&_mutex);
    }

    void Lock() {
        ::EnterCriticalSection(&_mutex);
    }

    void UnLock() {
        ::LeaveCriticalSection(&_mutex);
    }

    bool TryLock() {
        return ::TryEnterCriticalSection(&_mutex) != FALSE;
    }

    ::CRITICAL_SECTION* mutex() {
        return &_mutex;
    }

  private:
    ::CRITICAL_SECTION _mutex;

    DISALLOW_COPY_AND_ASSIGN(Mutex);
};

class RwLock {
  public:
    RwLock() {
        ::InitializeSRWLock(&_lock);
    }
    ~RwLock() {}

    void ReadLock() {
        ::AcquireSRWLockShared(&_lock);
    }

    void WriteLock() {
        ::AcquireSRWLockExclusive(&_lock);
    }

    void ReadUnLock() {
        ::ReleaseSRWLockShared(&_lock);
    }

    void WriteUnLock() {
        ::ReleaseSRWLockExclusive(&_lock);
    }

    bool TryReadLock() {
        return ::TryAcquireSRWLockShared(&_lock) != FALSE;
    }

    bool TryWriteLock() {
        return ::TryAcquireSRWLockExclusive(&_lock) != FALSE;
    }

  private:
    ::SRWLOCK _lock;

    DISALLOW_COPY_AND_ASSIGN(RwLock);
};
#endif // _WIN32

class ScopedMutex {
  public:
    explicit ScopedMutex(Mutex& mutex)
        : _mutex(mutex) {
        _mutex.Lock();
    }

    ~ScopedMutex() {
        _mutex.UnLock();
    }

  private:
    Mutex& _mutex;

    DISALLOW_COPY_AND_ASSIGN(ScopedMutex);
};

class ScopedReadLock {
  public:
    explicit ScopedReadLock(RwLock& lock)
        : _lock(lock) {
        _lock.ReadLock();
    }

    ~ScopedReadLock() {
        _lock.ReadUnLock();
    }

  private:
    RwLock& _lock;

    DISALLOW_COPY_AND_ASSIGN(ScopedReadLock);
};

class ScopedWriteLock {
  public:
    explicit ScopedWriteLock(RwLock& lock)
        : _lock(lock) {
        _lock.WriteLock();
    }

    ~ScopedWriteLock() {
        _lock.WriteUnLock();
    }

  private:
    RwLock& _lock;

    DISALLOW_COPY_AND_ASSIGN(ScopedWriteLock);
};

class ScopedTryLock {
  public:
    explicit ScopedTryLock(Mutex& mutex)
        : _mutex(mutex) {
        _mutex.TryLock();
    }

    ~ScopedTryLock() {
        _mutex.UnLock();
    }

  private:
    Mutex& _mutex;

    DISALLOW_COPY_AND_ASSIGN(ScopedTryLock);
};

/************************ SyncEvent & Condition Variable **********************/
#ifndef _WIN32
class SyncEvent {
  public:
    explicit SyncEvent(bool manual_reset = true, bool signaled = false);

    ~SyncEvent() {
        CHECK(pthread_cond_destroy(&_cond) == 0);
    }

    void Notify() {
        ScopedMutex m(_mutex);
        if (!_signaled) {
            _signaled = true;
            pthread_cond_broadcast(&_cond);
        }
    }

    void Reset() {
        ScopedMutex m(_mutex);
        _signaled = false;
    }

    bool Signaled() {
        ScopedMutex m(_mutex);
        return _signaled;
    }

    void Wait() {
        ScopedMutex m(_mutex);
        if (!_signaled) pthread_cond_wait(&_cond, _mutex.mutex());
        if (!_manual_reset) _signaled = false;
    }

    // return false if timeout
    bool TimedWait(uint32 ms);

  private:
    pthread_cond_t _cond;
    Mutex _mutex;

    const bool _manual_reset;
    bool _signaled;
};

class Condition {
  public:
    Condition();
    ~Condition() {
        CHECK(pthread_cond_destroy(&_cond) == 0);
    }

    void Notify() {
        pthread_cond_signal(&_cond);
    }

    void NotifyAll() {
        pthread_cond_broadcast(&_cond);
    }

    void Wait(Mutex& mutex) {
        ScopedTryLock m(mutex);
        pthread_cond_wait(&_cond, mutex.mutex());
    }

    // return false if timeouot
    bool TimedWait(Mutex& mutex, uint32 ms);

  private:
    pthread_cond_t _cond;

    DISALLOW_COPY_AND_ASSIGN(Condition);
};

#else // win
class SyncEvent {
  public:
    explicit SyncEvent(bool manual_reset = true, bool signaled = false) {
        _h = ::CreateEvent(NULL, manual_reset, signaled, NULL);
        CHECK(_h != INVALID_HANDLE_VALUE);
    }

    ~SyncEvent() {
        ::CloseHandle(_h);
    }

    void Notify() {
        ::SetEvent(_h);
    }

    void Reset() {
        ::ResetEvent(_h);
    }

    bool Signaled() {
        return ::WaitForSingleObject(_h, 0) == WAIT_OBJECT_0;
    }

    void Wait() {
        ::WaitForSingleObject(_h, INFINITE);
    }

    // return false if timeout
    bool TimedWait(uint32 ms) {
        ::DWORD ret = ::WaitForSingleObject(_h, ms);
        if (ret == WAIT_TIMEOUT) return false;

        CHECK_EQ(ret, WAIT_OBJECT_0);
        return true;
    }

  private:
    ::HANDLE _h;

    DISALLOW_COPY_AND_ASSIGN(SyncEvent);
};

class Condition {
  public:
    Condition() {
        ::InitializeConditionVariable(&_cond);
    }
    ~Condition() {}

    void Notify() {
        ::WakeConditionVariable(&_cond);
    }

    void NotifyAll() {
        ::WakeAllConditionVariable(&_cond);
    }

    void Wait(Mutex& mutex) {
        ScopedTryLock m(mutex);
        ::SleepConditionVariableCS(&_cond, mutex.mutex(), INFINITE);
    }

    // return false if timeouot
    bool TimedWait(Mutex& mutex, uint32 ms) {
        ScopedTryLock m(mutex);

        if (::SleepConditionVariableCS(&_cond, mutex.mutex(), ms) != FALSE) {
            return true;
        }

        CHECK_EQ(::GetLastError(), ERROR_TIMEOUT);
        return false;
    }

  private:
    CONDITION_VARIABLE _cond;

    DISALLOW_COPY_AND_ASSIGN(Condition);
};
#endif

/************************************ Thread **********************************/
#ifndef _WIN32
class Thread {
  public:
    explicit Thread(Closure* c)
        : _c(c), _id(0) {
    }

    template<typename ... A>
    Thread(void (*f)(A ...), A ... a)
        : _c(NewPermanentCallback(f, a...)), _id(0) {
    }

    template<typename T, typename ... A>
    Thread(T* obj, void (T::*f)(A ...), A ... a)
        : _c(NewPermanentCallback(obj, f, a...)), _id(0) {
    }

    ~Thread() {
    }

    bool Start() {
        CHECK(_c != NULL);
        return pthread_create(&_id, 0, &Thread::Run, (void*) _c.get()) == 0;
    }

    void Join() {
        if (_id != 0) {
            pthread_join(_id, NULL);
            _id = 0;
        }
    }

    void Detach() {
        if (_id != 0) {
            pthread_detach(_id);
            _id = 0;
        }
    }

    void Cancel() {
        pthread_cancel(_id);
    }

    uint64 id() const {
        return _id;
    }

  private:
    scoped_ptr<Closure> _c;
    pthread_t _id;

    DISALLOW_COPY_AND_ASSIGN(Thread);

    static void* Run(void* p) {
        Closure* c = (Closure*) p;
        c->Run();
        return NULL;
    }
};

#else  // win
class Thread {
  public:
    explicit Thread(Closure* c)
        : _c(c), _h(INVALID_HANDLE_VALUE) {
    }

    template <typename ... A>
    Thread(void (*f)(A ...), A... a)
        : _c(NewPermanentCallback(f, a...)), _h(INVALID_HANDLE_VALUE) {
    }

    template <typename T, typename ... A>
    Thread(T* obj, void (T::*f)(A ...), A ... a)
        : _c(NewPermanentCallback(obj, f, a...)), _h(INVALID_HANDLE_VALUE) {
    }

    ~Thread() {
    }

    bool Start() {
        CHECK(_c != NULL);
        _h = ::CreateThread(NULL, 0, &Thread::Run, (void*) _c.get(), 0, NULL);
        return _h != INVALID_HANDLE_VALUE;
    }

    void Join() {
        if (_h != INVALID_HANDLE_VALUE) {
            ::WaitForSingleObject(_h, INFINITE);
            ::CloseHandle(_h);
            _h = INVALID_HANDLE_VALUE;
        }
    }

    void Detach() {
        if (_h != INVALID_HANDLE_VALUE) {
            ::CloseHandle(_h);
            _h = INVALID_HANDLE_VALUE;
        }
    }

    void Cancel() {
        ::TerminateThread(_h, 0);
    }

private:
    scoped_ptr<Closure> _c;
    ::HANDLE _h;

    DISALLOW_COPY_AND_ASSIGN(Thread);

    static ::DWORD WINAPI Run(void* p) {
        Closure* c = (Closure*) p;
        c->Run();
        return 0;
    }
};
#endif

class StoppableThread {
  public:
    explicit StoppableThread(Closure* c)
        : _t(NewPermanentCallback(this, &StoppableThread::Run)), _c(c),
          _stop(false) {
    }

    template<typename ... A>
    StoppableThread(void (*f)(A ...), A ... a)
        : _t(NewPermanentCallback(this, &StoppableThread::Run)),
          _c(NewPermanentCallback(f, a...)), _stop(false) {
    }

    template<typename T, typename ... A>
    StoppableThread(T* obj, void (T::*f)(A ...), A ... a)
        : _t(NewPermanentCallback(this, &StoppableThread::Run)),
          _c(NewPermanentCallback(obj, f, a...)), _stop(false) {
    }

    ~StoppableThread() {
    }

    bool Start() {
        CHECK(_c != NULL);
        return _t.Start();
    }

    void Stop() {
        if (_stop) return;
        _stop = true;
        _t.Join();
    }

  private:
    Thread _t;
    scoped_ptr<Closure> _c;
    bool _stop;

    void Run() {
        while (!_stop) {
            _c->Run();
        }
    }
};

class AutoThread {
  public:
    static void NewAutoThread(Closure* c) {
        (void) new AutoThread(c);
    }

  private:
    Thread _t;
    scoped_ptr<Closure> _c;

    explicit AutoThread(Closure* c)
        : _t(NewPermanentCallback(this, &AutoThread::Run)), _c(c) {
        CHECK(_c != NULL && _t.Start());
    }
    ~AutoThread() {
    }

    void Run() {
        _c->Run();
        _t.Detach();
        delete this;
    }
};
