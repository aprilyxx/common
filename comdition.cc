#ifndef _WIN32

#include "thread_util.h"

#ifdef __APPLE__
#  include <sys/time.h> // for gettimeofday()
#endif

static inline void CondInit(pthread_cond_t* cond) {
#ifdef __APPLE__
    CHECK(pthread_cond_init(cond, NULL) == 0);
#else
    pthread_condattr_t attr;
    CHECK(pthread_condattr_init(&attr) == 0);
    CHECK(pthread_condattr_setclock(&attr, CLOCK_MONOTONIC) == 0);
    CHECK(pthread_cond_init(cond, &attr) == 0);
    CHECK(pthread_condattr_destroy(&attr) == 0);
#endif
}

SyncEvent::SyncEvent(bool manual_reset, bool signaled)
    : _manual_reset(manual_reset), _signaled(signaled) {
    CondInit(&_cond);
}

Condition::Condition() {
    CondInit(&_cond);
}

static inline void SetTimeToWait(struct timespec* ts, uint32 ms) {
#ifdef __APPLE__
    struct timeval tv;
    gettimeofday(&tv, NULL);
    ts->tv_sec = tv.tv_sec;
    ts->tv_nsec = tv.tv_usec * 1000;
#else
    clock_gettime(CLOCK_MONOTONIC, ts);
#endif

    ts->tv_sec += ms / 1000;
    ts->tv_nsec += ms % 1000 * 1000000;
    if (ts->tv_nsec > 999999999) {
        ts->tv_nsec -= 1000000000;
        ++ts->tv_sec;
    }
}

bool SyncEvent::TimedWait(uint32 ms) {
    ScopedMutex m(_mutex);
    if (!_signaled) {
        struct timespec ts;
        SetTimeToWait(&ts, ms);

        int ret = pthread_cond_timedwait(&_cond, _mutex.mutex(), &ts);
        if (ret == ETIMEDOUT) return false;
        CHECK_EQ(ret, 0);
    }

    if (!_manual_reset) _signaled = false;
    return true;
}

bool Condition::TimedWait(Mutex& mutex, uint32 ms) {
    ScopedTryLock m(mutex);

    struct timespec ts;
    SetTimeToWait(&ts, ms);

    int ret = pthread_cond_timedwait(&_cond, mutex.mutex(), &ts);
    if (ret == ETIMEDOUT) return false;

    CHECK_EQ(ret, 0);
    return true;
}

#endif // _WIN32
