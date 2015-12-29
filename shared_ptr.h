#pragma once

#include "ref_counting.h"
#include "scoped_ref.h"

/*
 * obj owns the T* pointer, and shared_ptrs share the ref-counted obj
 */
template<typename T>
class shared_ptr {
  public:
    explicit shared_ptr(T* ptr = 0) {
        if (ptr != 0) _obj.reset(new Obj(ptr));
    }
    ~shared_ptr() {
    }

    shared_ptr(const shared_ptr& obj) {
        _obj.reset(obj.Copy());
    }

    shared_ptr& operator=(const shared_ptr& obj) {
        if (&obj != this) _obj.reset(obj.Copy());
        return *this;
    }

    T* get() const {
        return _obj != 0 ? _obj->ptr : 0;
    }

    void reset() {
        _obj.reset();
    }

    void reset(T* ptr) {
        if (this->get() != ptr) {
            ptr != 0 ? _obj.reset(new Obj(ptr)) : _obj.reset();
        }
    }

    int RefCount() const {
        return _obj != 0 ? _obj->RefCount() : 0;
    }

    T* operator->() const {
        DCHECK(this->get() != 0);
        return this->get();
    }

    T& operator*() const {
        DCHECK(this->get() != 0);
        return *this->get();
    }

    bool operator==(T* ptr) const {
        return this->get() == ptr;
    }

    bool operator!=(T* ptr) const {
        return this->get() != ptr;
    }

  private:
    struct Obj : public RefCounted {
        explicit Obj(T* p)
            : ptr(p) {
        }

        ~Obj() {
            delete ptr;
        }

        T* ptr;
    };

    Obj* Copy() const {
        _obj->ref();
        return _obj.get();
    }

  private:
    scoped_ref<Obj> _obj;
};
