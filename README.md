*This project has been created as part of the 42 curriculum by yanlu.*

# Codexion
## Description
Codexion is a concurrent programming project implementing a variation of the classic [dining philosophers problem](https://en.wikipedia.org/wiki/Dining_philosophers_problem).

N coders sit in a circle around a shared Quantum Compiler. Each coder alternates between three activities: compiling, debugging, and refactoring. Compiling requires holding two USB dongles simultaneously (left and right). There are as many dongles as coders, one between each adjacent pair. A dongle has to wait `time_to_cooldown` ms after being released before it can be used again.

The program stops in one of the following cases:
- All coders compiled at least `number_of_compiles_required` times.
- One of the coders burned out, i.e. they fail to start a new compilation within `time_to_burnout` milliseconds since their last compile or the start of the simulation. Coders should avoid burnout as far as possible.

Each coder runs as an independent POSIX thread. A dedicated monitor thread polls the simulation state every milisecond, and thus stops the program within 10 ms when one of the stop conditions is met.

### Thread synchronization mechanisms
A **race condition** is a condition of a program where its behavior depends on the relative timing or interleaving of multiple threads or processes. When multiple threads access a shared resources without sufficient protections, the behavior can be unpredictable or undefined.

**Mutexes** (type `pthread_mutex_t`) are used to prevent data races. A mutex is essentially a lock that allows us to regulate access to data and prevent shared resources being used at the same time.

In this project, dongles are implemented as mutexes. Picking up a dongle is implemented as locking its mutex for `time_to_compile` ms. This ensures that a dongle cannot be accessed by two coders at the same time.

Logging is protected by mutex too to prevent interleaving of the logging messages.
This in done by using a global `write_lock` that each coder points to.
Whenever a coder thread prints a log message, it locks the `write_lock` and only release it after the printing.

Two more mutexes, `compiles_lock` and `burnout_lock`, are implemented. Whenever a coder compiled, it addes one to its own `already_compiled` counter while locking with `compiles_lock`. The monitoring thread reads the `already_compiled` counter while locking with `compiles_lock` to check whether all coders have compiled enough times. The monitoring thread also checks whether any coder has burned out using the `burnout_lock`. Upon starting to compile, a coder updates their `last_compile` time while locking the `burnout_lock`. The monitoring thread reads the `last_compile` time of each coder while locking the `burnout_lock`, and compare it with `time_to_burnout` to check if a coder has burned out. Once the monitoring thread detects a stop condition, whether it's all coders compiled enough times or one coder has burned out, it sets `flag_stop` to one while locking the `stop_lock`. All coders reads `flag_stop` with the `stop_lock` continuously and stop their routine as soon as they detect that `flag_stop` has been set to one. Using mutexes prevents the coder threads and monitoring thread from accessing the same variable at the same time, and one thread might accessed the value before it was modified by the other.

A condition variable (`pthread_cond_t`) is implemented for each dongle to manage the priority queue. Each coder uses `pthread_cond_wait` while waiting on their turn to take the dongle. Releasing the dongle sends a signal to all waiting threads using `pthread_cond_broadcast` so that they can re-evaluate their priority and attempt to take the dongle again.

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

This project prevents deadlocks by breaking the resource holding condition. A coder will queue for both dongles and only acquire both of them when they are at the front of both queues. This prevents a coder from taking a dongle while waiting for another, which prevents other coders from using this dongle in the meantime.

This project also breaks the circular wait condition by implementing an asymmetric resource hierarchy.
Even-numbered coders always request their left dongle first, while odd-numbered coders always request their right dongle first. This ensures that the dongles are always locked in the order of odd-indexed dongle -> even-indexed dongle.

#### Starvation prevention
In order to ensure fair arbitration of resources and prevent one thread hoard resources and others starve, a priority queue (heap) is implemented for each dongle.
The following scheduling policies are implemented for distributing the dongles:

- **FIFO (First In, First Out)**: The dongle is granted to the coder whose request arrived first.
- [**EDF (Earliest Deadline First)**](https://en.wikipedia.org/wiki/Earliest_deadline_first_scheduling): The dongle is granted to the coder whose deadline is the closest. Deadline = `last_compile_start`+ `time_to_burnout`.

#### Cooldown handling
Dongles have a cooldown time where they are unavailable after being released. This is implemented as follows:
Before a coder attempts to take a dongle, in addition to checking the availablity of the dongle mutex, they also need to check that `current_time >= ready_time`. `ready_time` is `last_used_time + time_to_cooldown`. Otherwise they need to keep waiting.

#### Precise burnout detection
The monitoring thread runs continuously and checks whether a coder has burned out using the `burnout_lock` every milisecond. Once a burnout is detected, the monitoring thread sets `flag_stop` to one with `stop_lock`. All coder threads read `flag_stop` with `stop_lock` and stop their routine.

#### Log serialization
Logging is down through the `print_status` function which is protected by the `write_lock`. This guarantees that no two threads will print at the same time and the terminal output will not be interleaved.


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

AI is used to help debug and refactor the code.
