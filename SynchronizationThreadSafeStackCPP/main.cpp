#include <iostream>
#include <pthread.h>
#include <fstream>
#include <stdexcept>
#include <vector>

template<typename T>
class StackNode {
public:
    T data;
    StackNode* next;

    explicit StackNode(T val) : data(val), next(nullptr) {}
};

template<typename T>
class ThreadSafeStack {
private:
    StackNode<T>* top;
    pthread_mutex_t mutex;
    std::ofstream file;
    std::atomic<bool> isClearing{false};

public:
    ThreadSafeStack() : top(nullptr), mutex(PTHREAD_MUTEX_INITIALIZER) {
        pthread_mutex_init(&mutex, nullptr);
        file.open("output.txt", std::ios::out);
        if (!file.is_open()) {
            throw std::runtime_error("Unable to open file");
        }
    }

    ~ThreadSafeStack() {
        clear();
        pthread_mutex_destroy(&mutex);
        if (file.is_open()) {
            file.close();
        }
    }

    void push(T value) {
        if (isClearing.load()) return;

        pthread_mutex_lock(&mutex);
        auto newNode = new StackNode<T>(value);
        newNode->next = top;
        top = newNode;
        if (file.is_open()) {
            file << "Pushed: " << value << std::endl;
        }
        pthread_mutex_unlock(&mutex);
    }

    T pop() {
        pthread_mutex_lock(&mutex);
        if (top == nullptr) {
            pthread_mutex_unlock(&mutex);
            std::cerr << "Error: Attempted to pop from an empty stack.\n";
            throw std::runtime_error("Attempted to pop an empty stack.");
        }
        auto node = top;
        T data = node->data;
        top = node->next;
        delete node;
        if (file.is_open()) {
            file << "Popped: " << data << "\n";
        }
        pthread_mutex_unlock(&mutex);
        return data;
    }

    void clear() {
        isClearing.store(true);
        pthread_mutex_lock(&mutex);
        while (top != nullptr) {
            try {
                pop();
            } catch (const std::runtime_error& e) {
                std::cerr << "Clearing error: " << e.what() << "\n";
                break;
            }
        }
        pthread_mutex_unlock(&mutex);
        isClearing.store(false);
    }
};

void* testStack(void* arg) {
    auto stack = static_cast<ThreadSafeStack<int>*>(arg);
    for (int i = 0 ; i < 500 ; ++i) {
        try {
            stack->push(i * 3 + 1);
            stack->push(i * 3 + 2);
            stack->pop();
            stack->push(i * 3 + 3);
            stack->pop();
            stack->pop();
        } catch (const std::runtime_error& e) {
            std::cerr << "Operation error: " << e.what() << "\n";
        }
    }
    return nullptr;
}

int main() {
    // Allocate space for 200 threads
    std::vector<pthread_t> threads(200);
    ThreadSafeStack<int> stack;

    // Create threads
    for (auto& thread : threads) {
        if (pthread_create(&thread, nullptr, testStack, &stack) != 0) {
            std::cerr << "Failed to create thread." << std::endl;
            return 1;
        }
    }

    // Join threads
    for (auto& thread : threads) {
        pthread_join(thread, nullptr);
    }

    stack.clear();

    std::cout << "Program complete.\n";

    return 0;
}