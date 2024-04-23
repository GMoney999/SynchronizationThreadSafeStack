#include <iostream>
#include <pthread.h>
#include <fstream>
#include <stdexcept>
#include <vector>

// Define a template class StackNode that can store any type T.
template<typename T>
class StackNode {
public:
    T data; // Public member variable to hold the data of type T.
    StackNode* next; // Pointer to the next StackNode in the stack.

    // Explicit constructor that initializes a StackNode with a given value and sets the next pointer to nullptr.
    explicit StackNode(T val) : data(val), next(nullptr) {}
};


// Define a generic, thread-safe stack class that can handle any type T.
template<typename T>
class ThreadSafeStack {
private:
    StackNode<T>* top;  // Pointer to the top node of the stack.
    pthread_mutex_t mutex;  // Mutex to ensure thread safety during operations.
    std::ofstream file;  // File stream for logging stack operations.
    std::atomic<bool> isClearing{false};  // Atomic flag to prevent operations during stack clearing.

public:
    // Constructor to initialize the stack.
    ThreadSafeStack() : top(nullptr), mutex(PTHREAD_MUTEX_INITIALIZER) {
        pthread_mutex_init(&mutex, nullptr);  // Initialize the mutex.
        file.open("output.txt", std::ios::out);  // Open the output file for logging.
        if (!file.is_open()) {  // Check if the file opened successfully.
            throw std::runtime_error("Unable to open file");  // Throw an exception if the file cannot be opened.
        }
    }

    // Destructor to clean up resources.
    ~ThreadSafeStack() {
        clear();  // Clear the stack.
        pthread_mutex_destroy(&mutex);  // Destroy the mutex.
        if (file.is_open()) {  // Check if the file is still open.
            file.close();  // Close the file.
        }
    }

    // Method to push a value onto the stack.
    void push(T value) {
        if (isClearing.load()) return;  // Return immediately if the stack is being cleared.

        pthread_mutex_lock(&mutex);  // Lock the mutex before modifying the stack.
        auto newNode = new StackNode<T>(value);  // Create a new node.
        newNode->next = top;  // Set the new node's next to the current top.
        top = newNode;  // Update the top to be the new node.
        if (file.is_open()) {  // Check if the file is open.
            file << "Pushed: " << value << std::endl;  // Log the push operation.
        }
        pthread_mutex_unlock(&mutex);  // Unlock the mutex after modifying the stack.
    }

    // Method to pop a value from the stack.
    T pop() {
        pthread_mutex_lock(&mutex);  // Lock the mutex before modifying the stack.
        if (top == nullptr) {  // Check if the stack is empty.
            pthread_mutex_unlock(&mutex);  // Unlock the mutex if the stack is empty.
            std::cerr << "Error: Attempted to pop from an empty stack.\n";  // Log an error message.
            throw std::runtime_error("Attempted to pop an empty stack.");  // Throw an exception.
        }
        auto node = top;  // Store the top node.
        T data = node->data;  // Get the data from the top node.
        top = node->next;  // Update the top to the next node.
        delete node;  // Delete the old top node.
        if (file.is_open()) {  // Check if the file is open.
            file << "Popped: " << data << "\n";  // Log the pop operation.
        }
        pthread_mutex_unlock(&mutex);  // Unlock the mutex after modifying the stack.
        return data;  // Return the popped data.
    }

    // Method to clear the stack.
    void clear() {
        isClearing.store(true);  // Set the clearing flag to true.
        pthread_mutex_lock(&mutex);  // Lock the mutex before modifying the stack.
        while (top != nullptr) {  // Loop until the stack is empty.
            try {
                pop();  // Pop each node.
            } catch (const std::runtime_error& e) {  // Catch any exceptions from pop.
                std::cerr << "Clearing error: " << e.what() << "\n";  // Log clearing errors.
                break;  // Break the loop on error.
            }
        }
        pthread_mutex_unlock(&mutex);  // Unlock the mutex after modifying the stack.
        isClearing.store(false);  // Reset the clearing flag to false.
    }
};

// Define a function that performs a sequence of stack operations. This function is intended to be used with pthreads.
void* testStack(void* arg) {
    // Cast the void pointer to a pointer of ThreadSafeStack<int> type.
    auto stack = static_cast<ThreadSafeStack<int>*>(arg);
    // Loop 500 times to perform stack operations.
    for (int i = 0; i < 500; ++i) {
        try {
            // 3 intermixed push and pop operations
            // 'i * 3 + _' is a way to generate distinct values for each iteration of the loop that are evenly spaced apart
            stack->push(i * 3 + 1);
            stack->push(i * 3 + 2);
            stack->pop();
            stack->push(i * 3 + 3);
            stack->pop();
            stack->pop();
        } catch (const std::runtime_error& e) {
            // If an error occurs during the stack operations, log the error message.
            std::cerr << "Operation error: " << e.what() << "\n";
        }
    }
    // Return null as the function is used as a thread routine which does not need to return any value.
    return nullptr;
}

// Main Control Flow
int main() {
    // Create a vector to store handles of 200 threads.
    std::vector<pthread_t> threads(200);
    // Instantiate a thread-safe stack to be shared among threads.
    ThreadSafeStack<int> stack;

    // Iterate over the vector to create threads.
    for (auto& thread : threads) {
        // Create a new thread that runs the testStack function with a reference to 'stack'.
        if (pthread_create(&thread, nullptr, testStack, &stack) != 0) {
            // If thread creation fails, print an error message and exit with code 1.
            std::cerr << "Failed to create thread." << std::endl;
            return 1;
        }
    }

    // Wait for all threads to finish execution by joining each one.
    for (auto& thread : threads) {
        pthread_join(thread, nullptr);
    }

    // Clear the stack to release any remaining resources.
    stack.clear();

    std::cout << "Program complete.\n";
    return 0;
}
