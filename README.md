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
BASIC
======
C++ Command Line Flags Parser.  

API
======
* Parse Command Line Flags
```cpp
// Parse command line flags from <argc, argv>.  
// Non-flag elements will be put into the vector, if v != NULL.  
void init_ccflag(int argc, char** argv, std::vector<std::string>* v = NULL);

// Parse command line flags from a string.  
// Non-flag elements will be put into the vector, if v != NULL.  
// <usage>  init_ccflag("-i=23 -s=\"hello world\" -t=hello_world");  
void init_ccflag(const std::string& args, std::vector<std::string>* v = NULL);

// Parse command line flags from <argc, argv> first, and then from config file.
// Errors are ignored in the config file
void init_ccflag(int argc, char** argv, const std::string& config);
```

* Set Flag Value
```cpp
// set value of a flag by name, return false if flag not found or value invalid.  
// <usage>  SetFlagValue("boo", "true"); // --boo=true.  
bool SetFlagValue(const std::string& name, const std::string& value);
```

* Declare or Define a flag
```cpp
// available flag type: bool, int32, int64, uint32, uint64, string, double
DEC_bool(name);                // declare a bool flag.
DEF_int32(name, value, help);  // define an int32 flag.
```

USAGE
======
* Simple Example
```cpp
DEF_bool(b, true, "bool flag");
if (FLG_b) std::cout << "b is true" << std::endl;

DEF_string(s, "hello world", "s is a string flag");
std::cout << "s = " << FLG_s << ", size: " << FLG_s.size() << std::endl;

FLG_s = "hello";
std::cout << "s = " << FLG_s << ", size: " << FLG_s.size() << std::endl;
```

* Config File Format
```cpp
# comments
n  =  23            # spaces before or after '=' are ignored 
  s = hello world   # spaces at the beginning or the end of line are ignored
t = "hello world"   # '"' are ignored, string s and t have the same value

i = 1k              # i = 1024   available units: <K,k,M,m,G,g,T,t>
j = -032            # j = -26    octal number
k = 0x32            # k = 50     hexadecimal number
bt = true           # bool flag with value: true
bf = false          # bool flag with value: false
```

TEST
======
* Build  
```cpp
cd test && ./build.sh  
```

* Run with default flag value  
```cpp
vin@envy:~/cc/ccflag/test$ ./exe 
i32: -32
i64: -64
u32: 32
u64: 64
str: hello
dbl: 3.14
boo: false
x: false
y: false
z: false
```

* Run with new flag value  
```cpp
vin@envy:~/cc/ccflag/test$ ./exe -i32=-4k -i64=-8G -u32=032 -u64=0x32 -str="are you sure?" -xz -boo
i32: -4096
i64: -8589934592
u32: 26
u64: 50
str: are you sure?
dbl: 3.14
boo: true
x: true
y: false
z: true
```

* Show Flags Info
```cpp
// ./exe - or ./exe --    print flags info to stderr  
// ./exe ---              write flags info to file: flg.log  
// NOTES  flags with empty help info are unvisible  
vin@envy:~/cc/ccflag/test$ ./exe --
--boo: bool flag
     type: bool      default: false
     from: test.cc
--dbl: double flag
     type: double    default: 3.14
     from: test.cc
--i32: int32
     type: int32     default: -32
     from: test.cc
--i64: int64
     type: int64     default: -64
     from: test.cc
--str: std::string
     type: string    default: "hello"
     from: test.cc
--u32: uint32
     type: uint32    default: 32
     from: test.cc
--u64: uint64
     type: uint64    default: 64
     from: test.cc
--x: bool flag x
     type: bool      default: false
     from: test.cc
--y: bool flag y
     type: bool      default: false
     from: test.cc
--z: bool flag z
     type: bool      default: false
     from: test.cc
```

* Application will be terminated immediately once init_ccflag() encounters an error.
```cpp
vin@envy:~/cc/ccflag/test$ ./exe -dbl=3.14.15
invalid value for double: -dbl=3.14.15

vin@envy:~/cc/ccflag/test$ ./exe -u32=8g -dbl=3.14.15
overflow for 32 bit integer: -u32=8g
```

* Error info will be print to file: err.log, if the first argument is a dot.
```cpp
vin@envy:~/cc/ccflag/test$ ./exe . -boo=3.14
vin@envy:~/cc/ccflag/test$ cat err.log 
invalid value for bool: -boo=3.14
```
