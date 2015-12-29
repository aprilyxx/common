# common
BASE
----
Reference Counting  
------------------
```cpp
class T : public RefCounted {};

T* obj = new T;              // ref_count ==> 1
{
    obj->ref();              // ref_count ==> 2
    scoped_ref<T> oo(obj);   // auto unref() 
}
obj->unref();                // ref_count ==> 0 ==> delete obj
```

scoped_ptr & shared_ptr  
-----------------------
```cpp
scoped_ptr<int> i(new int(3));
cout << *i << endl;

shared_ptr<int> ia(new int(7));    // ref_count ==> 1
shared_ptr<int> ib = ia;           // ref_count ==> 2
cout << *ib << ", " << ib.ref_count() << endl;
```

Time Util  
---------
```cpp
uint64 mnow = NowInMs(); // gettimeofday: in ms
uint64 unow = NowInUs(); // gettimeofday: in us

WallTimer tm;  // start a timer

/* sleep one second */
SleepInSeconds(1);
SleepInMs(1000);
SleepInUs(1000000);

cout << t.Elapse() << ", " << t.ElapseInMs() << ", " 
     << t.ElapseInUs() << endl;
```

Closure & Thread based on `Variadic Templates`   
----------------------------------------------
* Closure is the wrapper of a function(member or non-member).  
* Thread owns the closure object.  

```cpp
int fun() {
    cout << "hello fun" << endl;
}

struct T {
    int hello(int i) {
        cout << s << ": " << i << endl;
        return ++i;
    }

    void world(int v) {
        ::SleepInMs(200);
        cout << s << ": " << v << endl;
    }
};

// Normal Thread, must be started and joined manually.
Thread t(&fun);
t.Start();   
t.Join();

// StoppableThread:  will not stop until Stop() is called
T obj;
StoppableThread st(&obj, &T::world, 7);
st.Start();
st.Stop();
```

Mutex & RwLock   
--------------
```cpp
Mutex m;
m.Lock();    m.UnLock();    m.TryLock();

RwLock l;
l.ReadLock();    l.ReadUnLock();    l.TryReadLock();      // ReadLock
l.WriteLock();   l.WriteUnLock();   l.TryWriteLock();     // WriteLock

// scoped lock: lock in constructor and unlock in destructor
ScopedMutex sm(m);

ScopedReadLock r(l);

ScopedWriteLock w(l);
```

SyncEvent & Condition   
----------------
```cpp
SyncEvent ev;
ev.TimedWait(1000);       // wait for one second.
ev.Signal();              // set event to be signaled.
ev.Reset();               // set event to be non-signaled.

Mutex m;
Condition cond;              
cond.TimedWait(m, 50); 

cond.Notify();            // wakeup one
cond.NotifyAll();         // wakeup all
```
