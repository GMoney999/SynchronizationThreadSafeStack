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
//// Declare the mutex for writing to the output file
pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;

//// Stack function declarations
////  Using a double pointer ** allows the function to modify the pointer itself and change what the top of the stack points to
void    push    (value_t v, StackNode **top, FILE *fp); // Pushes value_t onto the top of the stack
value_t pop     (           StackNode **top, FILE *fp); // Extracts value_t from the top of the stack
int     is_empty(           StackNode *top); // Checks if the stack has a top node. If not, then the stack is empty
void *testStack(void *arg); // Serves as the entry point for each thread

//// Main control flow
int main(void) {
    pthread_t threads[NUM_THREADS]; // Declare an array to hold the threads
    FILE *fp = fopen("output.txt", "w"); // Open a file called "output.txt" in write mode to log output data.
    if (!fp) { // Check if the file was successfully open
        perror("Failed to open file"); // If it wasn't, print an error message and exit
        return EXIT_FAILURE;
    }
    // Create 200 threads
    for (int i = 0 ; i < NUM_THREADS ; i++) {
        // Attempt to create each thread to run testStack and pass the file pointer as an argument.
        if (pthread_create(&threads[i], NULL, testStack, fp) != 0) {
            perror("Failed to create thread"); // If thread creation fails, print error message
        }
    }
    // Wait for all the threads to finish
    for (int i = 0 ; i < NUM_THREADS ; i++) {
        pthread_join(threads[i], NULL); // Block until the current thread terminates
    }
    fclose(fp); // Close the file after all threads have completed writing to it
    pthread_mutex_destroy(&mutex); // Destroy the mutex used to synchronize stack access
    pthread_mutex_destroy(&file_mutex); // Destroy the mutex used to synchronize file writing
    // Exit the program
    printf("Program complete.\n");
    return 0;
}

void push(value_t v, StackNode **top, FILE *fp) {
    pthread_mutex_lock(&mutex); // Lock the mutex to prevent other threads from accessing the stack concurrently
    StackNode *new_node = malloc(sizeof(StackNode)); // Allocate the memory for a new stack node
    // Check if the memory allocation was successful
    if (new_node == NULL) {
        // If it wasn't output an error message, release the mutex and exit
        fprintf(stderr, "Failed to allocate memory for new node\n");
        pthread_mutex_unlock(&mutex);
        return;
    }
    new_node->data = v; // Set the data field of the new node to the provided value
    new_node->next = *top; // Link the new node to the current top of the stack
    *top = new_node; // Update the top pointer to the new node, making it the new top of the stack
    pthread_mutex_lock(&file_mutex); // Lock the file mutex to safely write to the file
    fprintf(fp, "Pushed %d\n", v); // Write an entry to the output file indicating a node has been pushed
    pthread_mutex_unlock(&file_mutex); // Unlock the file mutex after writing to the file
    pthread_mutex_unlock(&mutex); // Unlock the main mutex after mutating the stack
}

value_t pop(StackNode **top, FILE *fp) {
    pthread_mutex_lock(&mutex); // Lock the mutex to prevent other threads from accessing the stack
    if (is_empty(*top)) { // Check if the stack is empty before attempting to pop
        pthread_mutex_unlock(&mutex); // Unlock the mutex before returning to avoid deadlock
        return (value_t)0; // Return a default value indicating the stack was empty
    }
    value_t data = (*top)->data; // Retrieve the data from the top node of the stack
    StackNode *temp = *top; // Store the top node in a temporary pointer
    *top = temp->next; // Update the top pointer to the next node, effectively removing the top node
    free(temp); // Free the memory of the node that was just removed from the stack
    pthread_mutex_lock(&file_mutex); // Lock the file mutex to safely write to the file
    fprintf(fp, "Popped %d\n", data); // Write an entry to the output file indicating a node has been popped
    pthread_mutex_unlock(&file_mutex); // Unlock the file mutex
    pthread_mutex_unlock(&mutex); // Unlock the stack mutex
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
        // 3 intermixed pushes and pops
        // 'i * 3 + _' is a way to generate distinct values for each iteration of the loop that are evenly spaced apart
        push(i * 3 + 1, &top, fp);

        push(i * 3 + 2, &top, fp);

        if (!is_empty(top)) {
            pop(&top, fp);
        }

        push(i * 3 + 3, &top, fp);

        if (!is_empty(top)) {
            pop(&top, fp);
        }

        if (!is_empty(top)) {
            pop(&top, fp);
        }
    }
    return NULL;
}