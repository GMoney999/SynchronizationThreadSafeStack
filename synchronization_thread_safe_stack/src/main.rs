use std::thread;
use std::sync::{Mutex, Arc};
use std::fs::{File};
use std::io::{Write, BufWriter};

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
    let file = File::create("output.txt").expect("Failed to create output file.");
    let writer = BufWriter::new(file);
    let shared_writer = Arc::new(Mutex::new(writer));

    let shared_stack = Arc::new(Mutex::new(Stack::<i32>::new()));
    let mut handles = vec![];

    for _ in 0..200 {
        let stack_clone = Arc::clone(&shared_stack);
        let writer_clone = Arc::clone(&shared_writer);
        let handle = thread::spawn(move || {
            let mut stack = stack_clone.lock().unwrap();
            let mut writer = writer_clone.lock().unwrap();
            test_stack(&mut stack, &mut writer);
        });
        handles.push(handle);
    }

    // Wait for all threads to complete
    for handle in handles {
        handle.join().unwrap();
    }

    println!("Program completed.");
}

fn test_stack(stack: &mut Stack<i32>, writer: &mut BufWriter<File>) {
    for i in 0..500 {
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


fn pop_and_log<T: std::fmt::Display>(stack: &mut Stack<T>, writer: &mut BufWriter<File>) {
    if let Some(value) = stack.pop() {
        writeln!(writer, "Popped {}", value).expect("Error writing to file");
    } else {
        writeln!(writer, "Stack was empty, nothing to pop").expect("Error writing to file");
    }
}


