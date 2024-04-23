//// stack_with_race_condition.c
////  THIS PROGRAM CONTAINS RACE CONDITIONS AND IS NOT OPTIMAL

//// Race Condition
// A race condition occurs when two or more threads can access shared data, and they try to change it at the same time.
// Because the outcome depends on the sequence/timing of the threads' execution, this can lead to unpredictable results.
// In this program, the race condition stems from shared access to the 'top' pointer
// If multiple threads attempt to push or pop from the stack simultaneously, they could potentially be modifying the top pointer at the same time.
//     Push - Two threads might both read the current 'top' then both attempt to update 'top' to their newly created node.
//               One node could effectively be lost (not link to the stack), or worse, the 'top' pointer could end up pointing to an invalid memory location.
//     Pop - If two threads both check that the stack is not empty and then proceed to modify 'top' and free the popped node simultaneously,
//             this could lead to one thread attempting to access or free a node that has already been freed by another thread.
//             This leads to undefined behavior like dangling pointers, double frees, or segmentation faults.
//// Problem: No synchronization mechanism
// The functions that manipulate the 'top' pointer do not use any locking mechanism
// (mutex, semaphore, etc.) that ensures that only one thread can modify the pointer at a time
//// Solution: Pthread Mutex Lock
// Introduce a mutex that guards every access (both read and write) to the 'top' pointer.
// Lock the mutex at the start of each function (push, pop, is_empty) and unlock it before each function returns.
// This ensures that only one thread can modify the stack at a time.

// SEE stack_ptr.c FOR MUTEX SOLUTION




//// Passing &top (a reference to a pointer) vs. top (just a pointer)
// This decision involves how the function can interact with and modify the original pointer.
// '&top' is the address to the pointer variable 'top'.
//     &top is a pointer to a pointer (StackNode **) and allows the function to modify the original pointer 'top'
//     When you pass &top, you are passing a reference to the pointer. This means if the function modifies the pointer (e.g. changing what it points to),
//         those changes will affect the original 'top' in the calling function (main).
//     Passing the address to the pointer to the top of the stack is needed because when you push or pop a StackNode, you need to change the 'top' pointer.
// 'top' is the pointer itself, NOT the address of the pointer.
//     When you pass 'top', you are passing a copy of the pointer.
//     Any modifications to this pointer inside the function will only affect the copy, NOT the original 'top' pointer in main.
//     While you can still access and modify the data that the pointer refers to, you cannot change which node is the top of the stack from within the function.
//     Changes to the pointer itself will be discarded when the function exits.

//// Double dereference ** vs. Single dereference *
// A double dereference is needed whenever we are modifying the value of the top pointer
//    Push and Pop need to modify the pointer to the top of the stack because a StackNode is being added or removed.
// A single dereference is needed whenever we are reading the value of the top pointer (NOT modifying)
//    is_empty only needs to read the value of the top pointer (either something or Null), it does not change any of its contents.

#include <stdio.h>
#include <stdlib.h>

//// Linked list node
// "Node" represents the nodes themselves
// "value_t" is an alias for int and is used to specify the type of data held in the nodes
typedef int value_t;
typedef struct Node
{
    value_t data; // Node will hold 'data' of type 'value_t' (int)
    struct Node *next; // Node will hold 'next' of type 'pointer' pointing to a Node
} StackNode; // renames "struct Node" to "StackNode" for variable declaration

//// Stack function declarations
////  Using a double pointer ** allows the function to modify the pointer itself and change what the top of the stack points to
// Pushes value_t onto the top of the stack
void    push    (value_t v, StackNode **top);
// Extracts value_t from the top of the stack
value_t pop     (           StackNode **top);
// Checks if the stack has a top node. If not, then the stack is empty
int     is_empty(           StackNode *top);

//// Main control flow
int main(void)
{
    //// Initialize a pointer to the top of the stack
    // Since no node has been pushed, it begins as NULL
    StackNode *top = NULL;
    //// Mutate the stack
    // Push operations take a value to give a StackNode and the address of the 'top' pointer to modify
    // Pop operations take the address of the 'top' pointer to remove the top StackNode
    push(5, &top);
    push(10,&top);
    pop (   &top);
    push(15,&top);
    pop (   &top);
    pop (   &top);
    push(20,&top);
    push(-5, &top);
    pop (    &top);
    push(-10,&top);
    pop (    &top);
    pop (    &top);
    push(-15,&top);
    pop (    &top);
    push(-20,&top);
    return 0;
}

//// Stack function definitions
//// push function
// Pushes value_t onto the top of the stack
void push(value_t v, StackNode **top)
{
    // Create a Node called 'new_node' by allocating memory equal to its size
    StackNode * new_node = malloc(sizeof(StackNode));
    // Set the 'data' of new_node to the value_t (int) passed to the function
    new_node->data = v;
    // Set the 'next' pointer of new_node to the node currently at the top of the stack
    // *top dereferences the pointer to the top node, making new_node point to the current top of the stack
    // This prepares new_node to be inserted at teh top of the stack by linking it to the current top node
    new_node->next = *top;
    // Update the top of the stack to new_node
    // 'top' is a pointer to a StackNode, meaning it points to the 'StackNode' pointer that holds the address of the top node of the stack
    // By setting '*top' to new_node, we change the pointer that 'top' points to, now pointing to new_node
    *top           = new_node;
}
//// pop function
// Extracts value_t from the top of the stack
value_t pop(StackNode **top)
{
    // If the stack is empty, i.e. if the 'top' pointer is pointing to NULL, return a value of 0
    // (value_t)0 is a typecast, making it explicit that the type value_t (int) is returned.
    // This may seem redundant since the function declaration already specifies that a value_t is being returned,
    // but this is a good practice for clarity. In a large codebase, it's possible that later on,
    // value_t may be changed to a float or another custom type.
    // It can also prevent bugs during refactoring or type changes.
    // In this program, however, (value_t) may be omitted and the program will work as expected
    if (is_empty(*top)) return (value_t)0;
    // Extract the data field (value_t) from the top node into a variable named 'data'
    // (*top) - Since 'top' is a pointer to a pointer (StackNode **top), dereferencing it once with *top gets us the current top node of the stack, which is a pointer to a StackNode.
    // (*top)->data - this accesses the 'data' field of the StackNode that 'top' points to.
    // -> is used to access a member of a structure through a pointer.
    // Assign the value of the 'data' field from the top node in the stack to a variable named 'data'.
    value_t     data  = (*top)->data;
    // Create a temporary pointer to the Node at the top of the stack
    // *top - dereferencing 'top' once gets you the current top Node of the stack
    // temp = *top - store the address of the top node in 'temp'
    // Having a temporary pointer to the current top node lets you access this node after 'top' is modified
    StackNode * temp  = *top;
    // Update the 'top' pointer to point to the next node in the stack
    // This basically removes the current top node from the stack
    // (*top) - refers to the node that is currently at the top of the stack
    // ->next - accesses the 'next' pointer of the current top node, which points to the node immediately below it in the stack
    // Setting *top to (*top)->next updates the top pointer of the stack to point to what was the second node, now making it first.
    *top              = (*top)->next;
    // Free the memory we allocated when creating the temporary pointer to top
    free(temp);
    // Return the 'data' field we extracted from the node that was previously at the top of the stack
    return data;
}
//// is_empty function
// Checks if the stack has a top node. If not, then the stack is empty
// Returns an integer (1 = true, 0 = false)
// Takes a pointer to the top StackNode
int is_empty(StackNode *top) {
    // If the top pointer points to nothing (NULL), the stack is empty, return true
    if (top == NULL) return 1;
    // If the top pointer points to anything that is not NULL, the stack is not empty, return false
    else             return 0;
}

