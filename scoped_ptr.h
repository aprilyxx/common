#pragma once

#include "cclog.h"

template<typename T>
class scoped_ptr {
  public:
    explicit scoped_ptr(T* p = 0)
        : _p(p) {
    }

    ~scoped_ptr() {
        static_cast<void>(sizeof(T));
        delete _p;
    }

    T* get() const {
        return _p;
    }

    T* release() {
        T* p = _p;
        _p = 0;
        return p;
    }

    void reset(T* p = 0) {
        if (_p != p) {
            static_cast<void>(sizeof(T));
            delete _p;
            _p = p;
        }
    }

    T* operator->() const {
        DCHECK(_p != 0);
        return _p;
    }

    T& operator*() const {
        DCHECK(_p != 0);
        return *_p;
    }

    bool operator==(T* p) const {
        return _p == p;
    }

    bool operator!=(T* p) const {
        return _p != p;
    }

    void swap(scoped_ptr& o) {
        T* p = _p;
        _p = o._p;
        o._p = p;
    }

  private:
    T* _p;

    scoped_ptr(const scoped_ptr&);
    void operator=(const scoped_ptr&);
};
