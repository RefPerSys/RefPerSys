# RefPerSys Concurrency Model Analysis

## Overview

RefPerSys implements a sophisticated multi-threaded concurrency model centered around an **agenda-based task execution system** with **cooperative multi-threading**, **event-driven I/O**, and **precise garbage collection coordination**. The system uses POSIX threads with careful synchronization primitives to ensure thread safety while maintaining high performance.

## Architecture Overview

### Core Components

1. **Agenda System**: Priority-based task scheduling with worker threads
2. **Event Loop**: Poll-based I/O multiplexing with signal handling
3. **Thread Coordination**: Mutexes, condition variables, and atomic operations
4. **Tasklets**: Small executable units with closures and metadata
5. **Call Frames**: Thread-local execution contexts with GC integration

### Threading Architecture

- **Main Thread**: Event loop, signal handling, I/O operations
- **Worker Threads**: Task execution from agenda queues
- **Per-Thread Storage**: Call frames, thread indices, random number generators
- **Thread-Safe Primitives**: Atomic counters, recursive mutexes, condition variables

## Key Classes and Structures

### Rps_Agenda

```cpp
class Rps_Agenda {
    static std::recursive_mutex agenda_mtx_;
    static std::condition_variable_any agenda_changed_condvar_;
    static std::deque<Rps_ObjectRef> agenda_fifo_[AgPrio__Last];
    static std::atomic<unsigned long> agenda_add_counter_;
    static std::atomic<bool> agenda_is_running_;
    static std::atomic<std::thread*> agenda_thread_array_[rps_JMAX];
    static std::atomic<workthread_state_en> agenda_work_thread_state_[rps_JMAX];
    static std::atomic<bool> agenda_needs_garbcoll_;
    static std::atomic<uint64_t> agenda_cumulw_gc_;
    static std::atomic<Rps_CallFrame*> agenda_work_gc_callframe_[rps_JMAX];
};
```

**Responsibilities:**
- Task scheduling with three priority levels (Low, Normal, High)
- Worker thread management and coordination
- Garbage collection synchronization across threads
- Tasklet lifecycle management

### Rps_PayloadTasklet

```cpp
class Rps_PayloadTasklet : public Rps_Payload {
    Rps_ClosureValue tasklet_todoclos;
    double tasklet_obsoltime;
    bool tasklet_permanent;
};
```

**Key Features:**
- Executable closure with optional expiration time
- Permanent vs. temporary task classification
- GC marking with time-based filtering

### Event Loop Data Structure

```cpp
struct event_loop_data_st {
    unsigned eld_magic;
    int eld_polldelaymillisec;
    unsigned eld_lastix;
    std::recursive_mutex eld_mtx;
    Rps_EventHandler_sigt* eld_handlarr[RPS_MAXPOLL_FD+1];
    const char* eld_explarr[RPS_MAXPOLL_FD+1];
    struct pollfd eld_pollarr[RPS_MAXPOLL_FD+1];
    void* eld_datarr[RPS_MAXPOLL_FD+1];
    int eld_sigfd, eld_timfd;
    int eld_selfpipereadfd, eld_selfpipewritefd;
    std::deque<unsigned char> eld_selfpipefifo;
    std::atomic<bool> eld_eventloopisactive;
    std::atomic<long> eld_nbloops;
    std::vector<std::function<void(struct pollfd*, int& npoll, Rps_CallFrame*)>> eld_prepollvect;
};
```

## Critical Algorithms

### Agenda Worker Thread Execution

```cpp
void Rps_Agenda::run_agenda_worker(int ix) {
    RPS_LOCALFRAME(RPS_ROOT_OB(_1aGtWm38Vw701jDhZn), RPS_NULL_CALL_FRAME, ...);
    
    while (agenda_is_running_.load()) {
        // Check GC threshold and trigger collection if needed
        if (cumulative_words > threshold) {
            do_garbage_collect(ix, &_);
        }
        
        // Fetch and execute tasklet
        _f.obtasklet = fetch_tasklet_to_run();
        if (_f.obtasklet) {
            taskpayl = _f.obtasklet->get_dynamic_payload<Rps_PayloadTasklet>();
            if (taskpayl && taskpayl->todo_closure())
                taskpayl->todo_closure().apply1(&_, _f.obtasklet);
        } else {
            // Wait for new tasks with timeout
            agenda_changed_condvar_.wait_for(agenda_mtx_, 500ms+ix*10ms);
        }
    }
}
```

**Algorithm Phases:**

1. **Initialization**: Set thread-local variables and GC callframe pointers
2. **GC Monitoring**: Check memory usage against thresholds
3. **Task Execution**: Fetch high-priority tasks and execute closures
4. **Synchronization**: Wait for new tasks or GC coordination

### Garbage Collection Coordination

```cpp
void Rps_Agenda::do_garbage_collect(int ix, Rps_CallFrame*callframe) {
    agenda_work_thread_state_[ix].store(WthrAg_GC);
    agenda_work_gc_callframe_[ix].store(callframe);
    
    // Wait for all threads to enter GC state
    while (!every_worker_is_gc) {
        agenda_changed_condvar_.wait_for(ulock, 50ms+ix*10ms, [=,&every_worker_is_gc] {
            for (int wix=1; wix<rps_nbjobs; wix++) {
                if (agenda_work_thread_state_[wix].load() != WthrAg_GC)
                    return false;
            }
            every_worker_is_gc = true;
            return true;
        });
    }
    
    // Primary thread performs GC
    if (ix==1) {
        rps_garbage_collect(&gcfun); // Marks call stacks
    }
    
    // Transition to end-GC state
    for (int wix=1; wix<rps_nbjobs; wix++) {
        if (agenda_work_thread_state_[wix].load() == WthrAg_GC)
            agenda_work_thread_state_[wix].store(WthrAg_EndGC);
    }
}
```

### Event Loop Processing

```cpp
void rps_event_loop(void) {
    while (!rps_stop_event_loop_flag.load()) {
        // Prepare poll file descriptors
        nbfdpoll = setup_poll_fds(pollarr);
        
        // Call prepoll functions
        for (auto fun : eld_prepollvect) {
            if (fun) fun(pollarr, nbfdpoll, &_);
        }
        
        // Poll with timeout
        respoll = poll(pollarr, nbfdpoll, poll_timeout);
        
        // Handle events
        for (int pix=0; pix<nbfdpoll; pix++) {
            if (pollarr[pix].revents != 0) {
                if (handlarr[pix])
                    handlarr[pix](&_, pollarr[pix].fd, pollarr[pix].revents);
            }
        }
    }
}
```

## Interface Contracts

### Agenda Management API

```cpp
// Task scheduling
void Rps_Agenda::add_tasklet(agenda_prio_en prio, Rps_ObjectRef obtasklet);

// Task retrieval
Rps_ObjectRef Rps_Agenda::fetch_tasklet_to_run(void);

// Lifecycle control
void rps_run_agenda_mechanism(int nbjobs);
void rps_stop_agenda_mechanism(void);
```

### Event Loop API

```cpp
// Handler registration
void rps_event_loop_add_input_fd_handler(int fd, Rps_EventHandler_sigt* f, const char* expl, void* data);
void rps_event_loop_add_output_fd_handler(int fd, Rps_EventHandler_sigt* f, const char* expl, void* data);

// Handler removal
void rps_event_loop_remove_input_fd_handler(int fd);
void rps_event_loop_remove_output_fd_handler(int fd);

// Pre-poll registration
int rps_register_event_loop_prepoller(std::function<void(struct pollfd*, int& npoll, Rps_CallFrame*)> fun);
```

### Thread-Local Storage

```cpp
// Per-thread variables
extern thread_local int rps_curthread_ix;
extern thread_local Rps_CallFrame* rps_curthread_callframe;

// Thread identification
static inline pid_t rps_thread_id(void);
static inline std::string rps_current_pthread_name(void);
static inline int rps_current_pthread_index(void);
```

## Integration Points

### Call Frame Integration

```cpp
class Rps_ProtoCallFrame {
    void gc_mark_frame(Rps_GarbageCollector* gc);
    void set_additional_gc_marker(const std::function<void(Rps_GarbageCollector*)>& gcmarkfun);
};
```

**Integration Pattern:**
- Call frames automatically mark local variables during GC
- Thread-local callframe pointers enable stack tracing
- Additional markers can be registered for complex locals

### Signal Handling Integration

```cpp
void rps_sigfd_read_handler(Rps_CallFrame*cf, int fd, void* data) {
    struct signalfd_siginfo infsig;
    int nbr = read(fd, &infsig, sizeof(infsig));
    
    switch (infsig.ssi_signo) {
        case SIGTERM: case SIGINT: case SIGQUIT:
            rps_do_stop_event_loop();
            break;
        case SIGCHLD:
            // Handle child process termination
            break;
    }
}
```

### Self-Pipe Communication

```cpp
enum self_pipe_code_en {
    SelfPipe_Dump = 'D',
    SelfPipe_GarbColl = 'G', 
    SelfPipe_Quit = 'Q',
    SelfPipe_Exit = 'X',
    SelfPipe_Process = 'P'
};

void rps_self_pipe_write_byte(unsigned char b);
void handle_self_pipe_byte_rps(unsigned char b);
```

**Integration Pattern:**
- Thread-safe communication via pipe
- Asynchronous command execution
- GC and dump operations triggered from any thread

## Performance Characteristics

### Threading Performance

- **Worker Threads**: Configurable count (3-24 threads)
- **Task Distribution**: Priority-based FIFO queues
- **Synchronization**: Minimal locking with condition variables
- **GC Coordination**: Stop-the-world with precise stack marking

### I/O Performance

- **Poll-Based**: Efficient multiplexing with configurable timeout
- **Signal Handling**: signalfd for reliable signal delivery
- **Self-Pipe**: Lock-free communication for thread commands
- **File Descriptor Limits**: 128 maximum concurrent descriptors

### Memory Performance

- **Per-Thread Allocation**: Thread-local call frames
- **Tasklet Overhead**: Minimal memory per task
- **GC Thresholds**: Configurable memory usage triggers

## Design Rationale

### Why Agenda-Based Tasking?

- **Priority Management**: Three-level priority system for responsiveness
- **Load Balancing**: Multiple worker threads distribute computation
- **GC Safety**: Coordinated garbage collection across all threads
- **Scalability**: Easy to adjust thread count based on workload

### Why Event Loop Architecture?

- **I/O Efficiency**: Single-threaded event loop minimizes context switching
- **Signal Safety**: Reliable signal handling without race conditions
- **Extensibility**: Easy to add new event sources and handlers
- **Integration**: Works with external processes and GUI systems

### Why Thread-Local Storage?

- **Performance**: Avoids synchronization for thread-specific data
- **Safety**: Each thread has its own call frame and execution context
- **GC Integration**: Enables precise stack scanning during collection

### Why Cooperative GC?

- **Precision**: All threads stop for accurate root marking
- **Simplicity**: Avoids complex incremental collection
- **Safety**: No race conditions between mutator and collector
- **Performance**: Fast collection with minimal pause times

## Critical Design Decisions

1. **Thread Model**: POSIX threads with thread-local storage for safety
2. **Synchronization**: Recursive mutexes and condition variables for coordination
3. **Task Scheduling**: Priority queues with FIFO ordering within priorities
4. **GC Coordination**: Stop-the-world collection with precise stack marking
5. **I/O Handling**: Poll-based event loop with signal integration
6. **Communication**: Self-pipe for thread-safe command delivery

## Future Evolution Opportunities

1. **Work Stealing**: Allow idle threads to steal work from busy threads
2. **NUMA Awareness**: Optimize thread placement for multi-socket systems
3. **Concurrent GC**: Implement incremental collection to reduce pauses
4. **Task Affinity**: Allow tasks to prefer specific threads
5. **I/O Threads**: Separate I/O handling from computation threads
6. **Coroutine Support**: Add lightweight coroutine scheduling

This concurrency model provides a solid foundation for RefPerSys's multi-threaded execution, balancing performance, safety, and complexity while enabling efficient parallel processing and I/O handling.