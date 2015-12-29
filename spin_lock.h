#pragma once

#include "atomic.h"

class SpinLock {
  public:
    SpinLock() {
    }
    ~SpinLock() {
    }

    bool TryLock() {
        return _lock.CompareSwap(0, 1);
    }

    void Lock() {
        while (!this->TryLock());
    }

    void UnLock() {
        _lock.And(0);
    }

  private:
    atomic_t _lock;
};
