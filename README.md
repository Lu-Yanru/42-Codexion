*This project has been created as part of the 42 curriculum by yanlu.*

# Codexion
## Description
Codexion is a concurrent programming project implementing a variation of the classic [dining philosophers problem](https://en.wikipedia.org/wiki/Dining_philosophers_problem).

N coders sit in a circle around a shared Quantum Compiler. Each coder alternates between three activities: compiling, debugging, and refactoring. Compiling requires holding two USB dongles simultaneously (left and right). There are as many dongles as coders, one between each adjacent pair.

The program stops in one of the following cases:
- All coders compiled at least `number_of_compiles_required` times.
- One of the coders burned out, i.e. they fail to start a new compilation within `time_to_burnout` milliseconds since their last compile or the start of the simulation. Coders should avoid burnout as far as possible.

Each coder runs as an independent POSIX thread. A dedicated monitor thread polls the simulation state every 100 µs and detects burnout within 10 ms.

### Thread synchronization mechanisms
the specific threading primitives used in your implementation (pthread_mutex_t, pthread_cond_t,
custom event implementation) and how they coordinate access to shared resources
(dongles, logging, monitor state). Include examples of how race conditions are
prevented and how thread-safe communication is achieved between coders and the
monitor

### Data race prevention and thread-safe communication
A **race condition** is a condition of a program where its behavior depends on the relative timing or interleaving of multiple threads or processes. When multiple threads access a shared resources without sufficient protections, the behavior can be unpredictable or undefined.

**Mutexes** (type `pthread_mutex_t`) are used to prevent data races. A mutex is essentially a lock that allows us to regulate access to data and prevent shared resources being used at the same time.

In this project, dongles are implemented as mutexes. Picking up a dongle is implemented as locking its mutex for `time_to_compile` ms. This ensures that a dongle cannot be accessed by two coders at the same time.

Logging protected by mutex too to prevent interleaving of the logging messages.
This in done by using a global `write_lock` that each coder points to.
Whenever a coder thread prints a log message, it locks the `write_lock` and only release it after the printing.

### Blocking cases handled
This solution addresses the following concurrency issues:

#### Deadlock prevention and Coffman’s conditions
The use of mutexes can result in **deadlocks**. [Deadlock](https://en.wikipedia.org/wiki/Deadlock_(computer_science)) is a situation in which no thread can proceed because each waits for another to take action.
A deadlock situation can arise if all of the 4 Coffman's conditions occur simultaneously in a system:
- Mutual exclusion: multiple resources are not shareable; only one process at a time may use each resource.
- Hold and wait or resource holding: a process is currently holding at least one resource and requesting additional resources which are being held by other processes.
- No preemption: a resource can be released only voluntarily by the process holding it.
- Circular wait: each process must be waiting for a resource which is being held by another process, which in turn is waiting for the first process to release the resource.
In this project, a deadlock can occur when each coder holds to dongle to their left.

This project breaks the circular wait condition by implementing an asymmetric resource hierarchy.
Even-numbered coders always request their left dongle first, while odd-numbered coders always request their right dongle first. This ensures that the dongles are always locked in the order of odd-indexed dongle -> even-indexed dongle.

#### Starvation prevention
The following scheduling policies are implemented for distributing the dongles:

- **FIFO (First In, First Out)**: The dongle is granted to the coder whose request arrived first.
- [**EDF (Earliest Deadline First)**](https://en.wikipedia.org/wiki/Earliest_deadline_first_scheduling): The dongle is granted to the coder whose deadline is the closest. Deadline = `last_compile_start`+ `time_to_burnout`.

#### Cooldown handling
check current_time - last_used >= time_to_cooldown before a coder picks up a dongle.

#### Precise burnout detection

#### Log serialization
Logging protected by mutex


## Instructions
Compile with:

    make

    // or
    cd coders
    cc -Wall -Wextra -Werror -pthread *.c -o codexion

Run with:

    ./codexion number_of_coders time_to_burnout time_to_compile time_to_debug \
           time_to_refactor number_of_compiles_required dongle_cooldown scheduler

    // Examples:
    ./codexion 4 800 200 100 200 5 400 fifo
    ./codexion 4 800 200 100 200 5 400 edf

All time values are in miliseconds and must be positive integers.
`number_of_coders` starts from 1.

| Argument | Description |
|----------|-------------|
|`number_of_coders`| Number of coders/dongles |
|`time_to_burnout`| Time in ms a coder can go without a compilation |
|`time_to_compile`| Time in ms a coder takes to compile |
|`time_to_debug`| Time in ms a coder takes to debug |
|`time_to_refactor`| Time in ms a coder takes to refactor |
|`number_of_compiles_required`| Target compilations per coder for normal program termination |
|`dongle_cooldown`| Time in ms where a dongle in unavailable after being released |
|`scheduler`| The arbitration policy used by dongles, `fifo` or `edf`|

### Output format
Any state change of a coder should be logged by printing a message to the terminal in the following format:

    timestamp_in_ms coder_id event

Example output:

    0 1 has taken a dongle
    1 1 has taken a dongle
    1 1 is compiling
    201 1 is debugging
    401 1 is refactoring
    402 2 has taken a dongle
    403 2 has taken a dongle
    403 2 is compiling
    603 2 is debugging
    803 2 is refactoring
    1204 3 burned out


## Resources
- [POSIX Threads Programming by Blaise Barney](https://hpc-tutorials.llnl.gov/posix/#note-this-tutorial-is-no-longer-supported-it-remains-for-archival-purposes)
- [CodeVault tutorial: Unix Threads in C](https://www.youtube.com/playlist?list=PLfqABt5AS4FmuQf70psXrsMLEDQXNkLq2)
- [HackerRank tutorial: Heaps](https://www.youtube.com/watch?v=t0Cq6tVNRBA)
- [Threads, mutexes and Concurrent programming in C](https://www.codequoi.com/en/threads-mutexes-and-concurrent-programming-in-c/)
- [Codexion visualizer](https://codexionvisualizer.dev/)
- [Helgrind: a thread error detector](https://valgrind.org/docs/manual/hg-manual.html)
