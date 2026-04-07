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

### Blocking cases handled
all the concurrency issues addressed in your solution (e.g., deadlock prevention and Coffman’s conditions, starvation prevention, cooldown handling, precise burnout detection, and log serialization).


## Instructions
Compile with:

    cd coders
    make

    // or
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

### Scheduling policies
- **FIFO (First In, First Out)**: The dongle is granted to the coder whose request arrived first.
- [**EDF (Earliest Deadline First)**](https://en.wikipedia.org/wiki/Earliest_deadline_first_scheduling): The dongle is granted to the coder whose deadline is the closest. Deadline = `last_compile_start`+ `time_to_burnout`.

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