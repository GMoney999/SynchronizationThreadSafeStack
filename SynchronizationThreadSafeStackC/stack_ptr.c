//// stack_ptr.c
//// STACK WITH MUTEX - SOLUTION TO RACE CONDITION

// This program solves the race condition issue inside "stack_with_race_condition.c" by adding a mutex as a synchronization mechanism.
// Using a mutex ensures that no two threads will read or write to the stack pointer at the same time, avoiding undefined behavior.
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// Define the number of threads and iterations
#define NUM_THREADS 200
#define NUM_ITERATIONS 500

//// Linked list node
// "Node" represents the nodes themselves
// "value_t" is an alias for int and is used to specify the type of data held in the nodes
typedef int value_t;
typedef struct Node
{
    value_t data; // Node will hold 'data' of type 'value_t' (int)
    struct Node *next; // Node will hold 'next' of type 'pointer' pointing to a Node
} StackNode; // renames "struct Node" to "StackNode" for variable declaration

//// Declare the mutex for the stack
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
/// Declare the mutex for writing to the output file
pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;

//// Stack function declarations
////  Using a double pointer ** allows the function to modify the pointer itself and change what the top of the stack points to
// Pushes value_t onto the top of the stack
void    push    (value_t v, StackNode **top, FILE *fp);
// Extracts value_t from the top of the stack
value_t pop     (           StackNode **top, FILE *fp);
// Checks if the stack has a top node. If not, then the stack is empty
int     is_empty(           StackNode *top);
// Serves as the entry point for each thread
void *testStack(void *arg);

//// Main control flow
int main(void) {
    // Declare an array to hold the threads
    pthread_t threads[NUM_THREADS];
    // Open a file called "output.txt" in write mode to log output data.
    FILE *fp = fopen("output.txt", "w");
    // Check if the file was successfully open
    if (!fp) {
        // If it wasn't, print an error message and exit
        perror("Failed to open file");
        return EXIT_FAILURE;
    }
    // Create 200 threads
    for (int i = 0 ; i < NUM_THREADS ; i++) {
        // Attempt to create each thread to run testStack and pass the file pointer as an argument.
        if (pthread_create(&threads[i], NULL, testStack, fp) != 0) {
            // If thread creation fails, print error message
            perror("Failed to create thread");
        }
    }
    // Wait for all the threads to finish
    for (int i = 0 ; i < NUM_THREADS ; i++) {
        // Block until the current thread terminates
        pthread_join(threads[i], NULL);
    }
    // Close the file after all threads have completed writing to it
    fclose(fp);
    // Destroy the mutex used to synchronize stack access
    pthread_mutex_destroy(&mutex);
    // Destroy the mutex used to synchronize file writing
    pthread_mutex_destroy(&file_mutex);
    // Exit the program
    printf("Program complete.\n");
    return 0;
}

void push(value_t v, StackNode **top, FILE *fp) {
    // Lock the mutex to prevent other threads from accessing the stack concurrently
    pthread_mutex_lock(&mutex);
    // Allocate the memory for a new stack node
    StackNode *new_node = malloc(sizeof(StackNode));
    // Check if the memory allocation was successful
    if (new_node == NULL) {
        // If it wasn't output an error message, release the mutex and exit
        fprintf(stderr, "Failed to allocate memory for new node\n");
        pthread_mutex_unlock(&mutex);
        return;
    }
    // Set the data field of the new node to the provided value
    new_node->data = v;
    // Link the new node to the current top of the stack
    new_node->next = *top;
    // Update the top pointer to the new node, making it the new top of the stack
    *top = new_node;
    // Lock the file mutex to safely write to the file
    pthread_mutex_lock(&file_mutex);
    // Write an entry to the output file indicating a node has been pushed
    fprintf(fp, "Pushed %d\n", v);
    // Unlock the file mutex after writing to the file
    pthread_mutex_unlock(&file_mutex);
    // Unlock the main mutex after mutating the stack
    pthread_mutex_unlock(&mutex);
}

value_t pop(StackNode **top, FILE *fp) {
    // Lock the mutex to prevent other threads from accessing the stack
    pthread_mutex_lock(&mutex);
    // Check if the stack si empty before attempting to pop
    if (is_empty(*top)) {
        // Unlock the mutex before returning to avoid deadlock
        pthread_mutex_unlock(&mutex);
        // Return a default value indicating the stack was empty
        return (value_t)0;
    }
    // Retrieve the data from the top node of the stack
    value_t data = (*top)->data;
    // Store the top node in a temporary pointer
    StackNode *temp = *top;
    // Update the top pointer to the next node, effectively removing the top node
    *top = temp->next;
    // Free the memory of the node that was just removed from the stack
    free(temp);
    // Lock the file mutex to safely write to the file
    pthread_mutex_lock(&file_mutex);
    // Write an entry to the output file indicating a node has been popped
    fprintf(fp, "Popped %d\n", data);
    // Unlock the file mutex
    pthread_mutex_unlock(&file_mutex);
    // Unlock the stack mutex
    pthread_mutex_unlock(&mutex);
    // Return the data that was in the node we popped off the stack
    return data;
}

// Check if the stack has a top node
int is_empty(StackNode *top) {
    if (top == NULL) { return 1; } else { return 0; }
}

// Entry point for each thread
// Loop 500 times, executing 3 push and 3 pop operations intermixed
void *testStack(void *arg) {
    // Convert the void* argument back to a FILE* to use for file operations
    FILE *fp = (FILE *)arg;
    // Initialize the top pointer of the stack to NULL, meaning the stack is empty
    StackNode *top = NULL;
    // Loop through 3 intermixed push and pop operations 500 times
    for (int i = 0 ; i < NUM_ITERATIONS ; i++) {
        // First push operation with index as data
        push(i * 3 + 1, &top, fp);

        // Second push operation with index + 1 as data
        push(i * 3 + 2, &top, fp);

        // Third push operation with index + 3 as data
        push(i * 3 + 3, &top, fp);

        if (!is_empty(top)) {
            pop(&top, fp);
        }

        if (!is_empty(top)) {
            pop(&top, fp);
        }

        if (!is_empty(top)) {
            pop(&top, fp);
        }
    }
    return NULL;
}