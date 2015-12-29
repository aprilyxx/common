#pragma once

#include <tuple>

/*
 * Indices<N + 1> ==> Indices<0, 0, 1, 2, ..., N> ==> Seq<0, 1, 2, ... , N>
 */
namespace xx {
template<int...> struct Seq {};

template<int N, int ... S>
struct Indices : Indices<N - 1, N - 1, S...> {
};

template<int ... S>
struct Indices<0, S...> {
    typedef Seq<S...> SeqType;
};

template<typename R, typename ... A, int ... S>
inline R Apply(R (*f)(A ...), const std::tuple<A...>& t, Seq<S...>) {
    return (R) f(std::get<S>(t)...);
}

template<typename R, typename ... A>
inline R Apply(R (*f)(A ...), const std::tuple<A...>& t) {
    return Apply<R>(f, t, typename Indices<sizeof...(A)>::SeqType());
}

template<typename R, typename T, typename ... A, int ... S>
inline R Apply(T* obj, R (T::*f)(A ...), const std::tuple<A...>& t, Seq<S...>) {
    return (R) (obj->*f)(std::get<S>(t)...);
}

template<typename R, typename T, typename ... A>
inline R Apply(T* obj, R (T::*f)(A ...), const std::tuple<A...>& t) {
    return Apply<R>(obj, f, t, typename Indices<sizeof...(A)>::SeqType());
}
}

class Closure {
  public:
    Closure() {
    }
    virtual ~Closure() {
    }

    virtual void Run() = 0;

  private:
    Closure(const Closure&);
    void operator=(const Closure&);
};

template<typename ... A>
class FunctionClosure : public Closure {
  public:
    typedef void (*F)(A ...);

    FunctionClosure(F f, A ... a)
        : _f(f), _a(a...) {
    }

    virtual ~FunctionClosure() {
    }

    virtual void Run() {
        xx::Apply<void>(_f, _a);
    }

  private:
    F _f;
    std::tuple<A...> _a;
};

template<typename T, typename ... A>
class MethodClosure : public Closure {
  public:
    typedef void (T::*F)(A ...);

    MethodClosure(T* obj, F f, A ... a)
        : _obj(obj), _f(f), _a(a...) {
    }

    virtual ~MethodClosure() {
    }

    virtual void Run() {
        xx::Apply<void>(_obj, _f, _a);
    }

  private:
    T* _obj;
    F _f;
    std::tuple<A...> _a;
};

template<typename ... A>
class FunctionCallback : public Closure {
  public:
    typedef void (*F)(A ...);

    FunctionCallback(F f, A ... a)
        : _f(f), _a(a...) {
    }

    virtual ~FunctionCallback() {
    }

    virtual void Run() {
        xx::Apply<void>(_f, _a);
        delete this;
    }

  private:
    F _f;
    std::tuple<A...> _a;
};

template<typename T, typename ... A>
class MethodCallback : public Closure {
  public:
    typedef void (T::*F)(A ...);

    MethodCallback(T* obj, F f, A ... a)
        : _obj(obj), _f(f), _a(a...) {
    }

    virtual ~MethodCallback() {
    }

    virtual void Run() {
        xx::Apply<void>(_obj, _f, _a);
        delete this;
    }

  private:
    T* _obj;
    F _f;
    std::tuple<A...> _a;
};

template<typename ... A>
inline Closure* NewPermanentCallback(void (*f)(A ...), A ... a) {
    return new FunctionClosure<A...>(f, a...);
}

template<typename T, typename ... A>
inline Closure* NewPermanentCallback(T* obj, void (T::*f)(A ...), A ... a) {
    return new MethodClosure<T, A...>(obj, f, a...);
}

template<typename ... A>
inline Closure* NewCallback(void (*f)(A ...), A ... a) {
    return new FunctionCallback<A...>(f, a...);
}

template<typename T, typename ... A>
inline Closure* NewCallback(T* obj,  void (T::*f)(A ...), A ... a) {
    return new MethodCallback<T, A...>(obj, f, a...);
}
