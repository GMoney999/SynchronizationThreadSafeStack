use std::thread;
use std::sync::{Mutex, Arc};
use std::fs::{File};
use std::io::{Write, BufWriter};

// A Box is needed (allocates memory on the heap) to avoid recursive types.
struct StackNode<T> {
    data: T,
    next: Option<Box<StackNode<T>>>,
}

struct Stack<T> {
    top: Option<Box<StackNode<T>>>,
}

impl<T> Stack<T> {
    fn new() -> Self {
        Self { top: None }
    }
    fn push(&mut self, data: T) {
        // Create a new pointer to a StackNode and set its 'next' pointer to the
        // take() takes the value out of the option, leaving a None in its place.
        // This effectively removes the current top node in the stack,
        // relocating it to the 'next' pointer in the new_node.
        let new_node = Box::new(StackNode {
            data,
            next: self.top.take()
        });
        // Since we used take() and left a None value at the top of the stack, we need to update it
        // Set the new top pointer (currently None since we took it) to the new_node
        self.top = Some(new_node);
    }
    fn pop(&mut self) -> Option<T> {
        // Unlike .take(), .map() DOES unwrap the Option, so we are just dealing with a Box<StackNode<T>>.
        // Box acts like a reference, so we can access the fields of the StackNode inside with '.'
        self.top.take().map(|node| {
            let ret = node.data;
            // Set the new top of the stack to the next node it pointed to
            self.top = node.next;
            ret
        })
    }
}

fn main() {
    // Create and open a new file called 'output.txt', or exit if the file can't be created.
    let file = File::create("output.txt").expect("Failed to create output file.");
    // Wrap the file in a BufWriter for efficient writing.
    let writer = BufWriter::new(file);
    // Wrap the BufWriter in an Arc and Mutex to allow safe shared access across threads.
    let shared_writer = Arc::new(Mutex::new(writer));
    // Create a new stack instance, wrap it in an Arc and Mutex for thread-safe shared access.
    let shared_stack = Arc::new(Mutex::new(Stack::<i32>::new()));
    // Initialize a vector to hold the handles of the spawned threads.
    let mut handles = vec![];
    // Loop 200 times to create 200 threads.
    for _ in 0..200 {
        // Clone the Arc pointing to the stack to pass to the thread.
        let stack_clone = Arc::clone(&shared_stack);
        // Clone the Arc pointing to the writer to pass to the thread.
        let writer_clone = Arc::clone(&shared_writer);
        // Spawn a new thread.
        let handle = thread::spawn(move || {
            // Lock the stack for this thread, panicking if the lock fails.
            let mut stack = stack_clone.lock().unwrap();
            // Lock the writer for this thread, panicking if the lock fails.
            let mut writer = writer_clone.lock().unwrap();
            // Execute the test_stack function which performs operations on the stack and writes to the file.
            test_stack(&mut stack, &mut writer);
        });
        // Store the handle of the spawned thread in the vector.
        handles.push(handle);
    }
    // After all threads are created, wait for each to complete.
    for handle in handles {
        // Block the current thread until the thread represented by handle completes.
        handle.join().unwrap();
    }
    // Print to the console when all threads have completed their execution.
    println!("Program completed.");

}

// Define the test_stack function that operates on a mutable reference to a Stack of i32 and a mutable BufWriter for a File.
fn test_stack(stack: &mut Stack<i32>, writer: &mut BufWriter<File>) {
    // Iterate 500 times, using `i` as the loop counter.
    for i in 0..500 {
        // 3 intermixed push and pop operations
        // 'i * 3 + _' is a way to generate distinct values for each iteration of the loop that are evenly spaced apart
        let next_value1 = i * 3 + 1;
        writeln!(writer, "Pushing {}", next_value1).expect("Error writing to file");
        stack.push(next_value1);

        let next_value2 = i * 3 + 2;
        writeln!(writer, "Pushing {}", next_value2).expect("Error writing to file");
        stack.push(next_value2);

        pop_and_log(stack, writer);

        let next_value3 = i * 3 + 3;
        writeln!(writer, "Pushing {}", next_value3).expect("Error writing to file");
        stack.push(next_value3);

        pop_and_log(stack, writer);

        pop_and_log(stack, writer);
    }
}

// Define a generic function pop_and_log that accepts a stack and a writer.
// The generic type T must implement the 'Display' trait for formatting.
fn pop_and_log<T: std::fmt::Display>(stack: &mut Stack<T>, writer: &mut BufWriter<File>) {
    // Attempt to pop a value from the stack.
    if let Some(value) = stack.pop() {
        // If a value is successfully popped (i.e., the stack was not empty), write a log message stating the popped value.
        writeln!(writer, "Popped {}", value).expect("Error writing to file");
    } else {
        // If no value could be popped (i.e., the stack was empty), write a log message stating that the stack was empty.
        writeln!(writer, "Stack was empty, nothing to pop").expect("Error writing to file");
    }
}



