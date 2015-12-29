#pragma once

#include "data_types.h"

#ifdef __APPLE__
#  include <libkern/OSAtomic.h>
#endif

#ifdef _WIN32
#  include <intrin.h>
#endif

/*
 * atomic_t
 *
 *  @Inc, Dec, Add    return new value
 *
 *  @And, Or, Xor     return original value
 *
 *  @CompareSwap      return true if swapped
 */

#ifndef _WIN32 // unix
class atomic_t {
  public:
    explicit atomic_t(uint32 v = 0)
        : _v(v) {
    }
    ~atomic_t() {
    }

    uint32 value() volatile {
        return _v;
    }

#ifndef __APPLE__
    uint32 Inc() volatile {
        return __sync_add_and_fetch(&_v, 1);
    }

    uint32 Dec() volatile {
        return __sync_add_and_fetch(&_v, -1);
    }

    uint32 Add(int32 i) volatile {
        return __sync_add_and_fetch(&_v, i);
    }

    uint32 And(uint32 i) volatile {
        return __sync_fetch_and_and(&_v, i);
    }

    uint32 Or(uint32 i) volatile {
        return __sync_fetch_and_or(&_v, i);
    }

    uint32 Xor(uint32 i) volatile {
        return __sync_fetch_and_xor(&_v, i);
    }

    bool CompareSwap(int32 oldv, int32 newv) volatile {
        return __sync_bool_compare_and_swap(&_v, oldv, newv);
    }

#else // mac
    uint32 Inc() volatile {
        return OSAtomicIncrement32Barrier(&_v);
    }

    uint32 Dec() volatile {
        return OSAtomicDecrement32Barrier(&_v);
    }

    uint32 Add(int32 i) volatile {
        return OSAtomicAdd32Barrier(i, &_v);
    }

    uint32 And(uint32 i) volatile {
        return OSAtomicAnd32OrigBarrier(i, (volatile uint32*) &_v);
    }

    uint32 Or(uint32 i) volatile {
        return OSAtomicOr32OrigBarrier(i, (volatile uint32*) &_v);
    }

    uint32 Xor(uint32 i) volatile {
        return OSAtomicXor32OrigBarrier(i, (volatile uint32*) &_v);
    }

    bool CompareSwap(int32 oldv, int32 newv) volatile {
        return OSAtomicCompareAndSwap32Barrier(oldv, newv, &_v);
    }
#endif // __APPLE__

  private:
    int32 _v;

    DISALLOW_COPY_AND_ASSIGN(atomic_t);
};

#else // win
class atomic_t {
  public:
    explicit atomic_t(uint32 v = 0)
        : _v(v) {
    }
    ~atomic_t() {
    }

    uint32 value() volatile {
        return _v;
    }

    uint32 Inc() volatile {
        return ::_InterlockedIncrement(&_v);
    }

    uint32 Dec() volatile {
        return ::_InterlockedDecrement(&_v);
    }

    uint32 Add(int32 i) volatile {
        return ::_InterlockedExchangeAdd(&_v, i) + i;
    }

    uint32 And(uint32 i) volatile {
        return ::_InterlockedAnd(&_v, i);
    }

    uint32 Or(uint32 i) volatile {
        return ::_InterlockedOr(&_v, i);
    }

    uint32 Xor(uint32 i) volatile {
        return ::_InterlockedXor(&_v, i);
    }

    bool CompareSwap(int32 oldv, int32 newv) volatile {
        return ::_InterlockedCompareExchange(&_v, newv, oldv) == oldv;
    }

  private:
    ::LONG _v;

    DISALLOW_COPY_AND_ASSIGN(atomic_t);
};
#endif // _WIN32
