#pragma once

#include <cclog/cclog.h>

#include "data_types.h"
#include "atomic.h"

class RefCounted {
  public:
    virtual void ref() {
        _count.Inc();
    }

    virtual void unref() {
        if (_count.Dec() == 0) delete this;
    }

    uint32 ref_count() {
        return _count.value();
    }

  protected:
    atomic_t _count;

    RefCounted()
        : _count(1) {
    }

    virtual ~RefCounted() {
        DCHECK(ref_count() == 0);
    }

  private:
    DISALLOW_COPY_AND_ASSIGN(RefCounted);
};
