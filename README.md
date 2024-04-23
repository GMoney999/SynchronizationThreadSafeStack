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
