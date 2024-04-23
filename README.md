# Synchronization Thread Safe Stack

This repository contains three separate implementations of a thread-safe stack in C, C++, and Rust. Each implementation uses mutexes to ensure that stack operations are thread-safe, and outputs the operations to an `output.txt` file.

## Directory Structure

- `SynchronizationThreadSafeStackC/` - C implementation.
- `SynchronizationThreadSafeStackCPP/` - C++ implementation.
- `synchronization_thread_safe_stack/` - Rust implementation.

---

## Usage

After building each version, you can run the executable which will perform predefined stack operations and log the output to `output.txt`. Check this file to see the results of the operations.


---

## Problem

The original C implementation provided addresses the issue of race conditions, which occur when multiple threads access and modify shared data — specifically the `top` pointer of a stack — without any synchronization mechanism. Race conditions lead to unpredictable outcomes such as data inconsistency, corruption, or loss, depending on the sequence and timing of thread execution.

---

## Solution

To fix the race condition, the implementation introduces the use of mutex locks (`pthread_mutex_t`). These locks are crucial for synchronizing access to the shared `top` pointer, ensuring that only one thread at a time can execute operations like push or pop on the stack. This approach provides mutual exclusion, which is necessary for preventing concurrent modifications that could lead to several issues:

- **Push Operation**: The mutex ensures that only one thread can modify the `top` pointer to point to a newly added node, thereby avoiding scenarios where multiple threads might add nodes at the same time, which could lead to lost node links or a corrupt stack.

- **Pop Operation**: The mutex lock guarantees that only one thread can modify the `top` pointer to remove a node from the stack. This synchronization is crucial for preventing multiple threads from simultaneously trying to remove the top node, which could result in invalid memory accesses or double frees.

Each stack-manipulating function (push, pop, and is_empty) acquires the mutex lock at its start and releases it before returning. This ensures that the stack remains in a consistent and valid state from any thread's perspective at any time during its operation, effectively serializing access to critical sections of the code that modify the stack.



--- 

## Implementation Details

### C Implementation

Located in `SynchronizationThreadSafeStackC/`, this implementation uses the POSIX threads (pthread) library to create a simple stack that can push and pop integer values. It protects concurrent access with a mutex and writes each operation to an `output.txt` file using a separate file mutex to ensure output consistency.

**Key Features:**
- Uses `pthread_mutex_t` for mutual exclusion.
- Stack operations are logged in `output.txt`.

---

### C++ Implementation

Found in `SynchronizationThreadSafeStackCPP/`, the C++ version also uses pthread for threading but takes advantage of C++ features such as templates, classes, and exceptions. This implementation provides a generic `ThreadSafeStack` that can handle any type supporting the `<<` operator to a `std::ostream`.

**Key Features:**
- Templated stack class to handle generic data types.
- Exception handling for error management.
- Uses `std::mutex` and `std::atomic<bool>` for thread safety and state management.

---

### Rust Implementation

The Rust code in `synchronization_thread_safe_stack/` showcases Rust's ownership, types, and thread safety guarantees with `Mutex` and `Arc` for shared state management. It leverages RAII (Resource Acquisition Is Initialization) to manage mutex locking and unlocking automatically.

**Key Features:**
- Safe concurrency with `Arc<Mutex<T>>`.
- Implementation of stack operations that log directly to a file using `BufWriter`.
- Uses Rust's pattern matching to handle optional values cleanly.

---

## Prerequisites

- GCC for compiling C code
- G++ for compiling C++ code
- Rust's Cargo tool for compiling and running Rust code
- POSIX-compliant system for pthread support

---

## Building and Running

### C

Navigate to the C directory and compile with:
```bash
gcc -o stack_program stack_ptr.c -lpthread
./stack_program
```

### C++
Navigate to the C++ directory and compile with:
```bash
g++ stack_program main main.cpp -lpthread -std=c++17
./main
```

### Rust
Navigate to the Rust directory and run with:
```bash
cargo build --release
cargo run
```
---

## License

This project is licensed under the GNU General Public License v3.0 - see the [LICENSE](LICENSE) file for details.

---
