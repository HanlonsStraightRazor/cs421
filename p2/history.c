#include <stdlib.h>
#include <string.h>
#include "history.h"

typedef struct node {
  char *command;
  struct node *next;
  struct node *previous;
} Node;

typedef struct history {
  Node *top;
  Node *bottom;
  int size;
  int limit;
} History;

/*
 * Creates a new history stack on the heap.
 */
History *initHistory(int limit) {
  // Allocate space for the history stack and handle malloc problems
  History *history = (History *) malloc(sizeof(History));
  if (history == NULL) {
    return NULL;
  }
  // Initialize and return history stack
  history->top = NULL;
  history->bottom = NULL;
  history->size = 0;
  history->limit = limit;
  return history;
}

/*
 * Adds a new command to the history stack,
 * dropping the bottom if the stack exceeds
 * its size limit. Returns 1 if unsuccessful,
 * else 0.
 */
int pushHistory(History *history, char *command) {
  // Create new node and set it up
  Node *node = (Node *) malloc(sizeof(Node));
  if (node == NULL) {
    return 1;
  }
  node->command = strdup(command);
  if (node->command == NULL) {
    return 1;
  }
  node->previous = NULL;
  // Special case: if the stack is empty
  if (history->size == 0) {
    node->next = NULL;
    history->top = node;
    history->bottom = node;
  } else {
    // If needed, drop the old bottom and update pointers
    if (history->size == history->limit) {
      Node *newBottom = history->bottom->previous;
      free(history->bottom);
      newBottom->next = NULL;
      history->bottom = newBottom;
      history->size--;
    }
    // Add the new top
    node->next = history->top;
    history->top->previous = node;
    history->top = node;
  }
  // Increment history size to reflect new node
  history->size++;
  return 0;
}

/*
 * Removes a command from the history stack
 * and returns it in string form.
 */
char *popHistory(History *history) {
  // If the stack is empty, return NULL
  if (history->top == NULL) {
    return NULL;
  }
  // Grab the top's command
  char *command = history->top->command;
  // Move the next element up and update pointers
  if (history->top->next == NULL) {
    // Special case: if the stack only has one more node
    free(history->top);
    history->top = NULL;
    history->bottom = NULL;
    history->size--;
  } else {
    Node *newTop = history->top->next;
    free(history->top);
    newTop->previous = NULL;
    history->top = newTop;
    history->size--;
  }
  // Return the old top's command
  return command;
}

/*
 * Returns the entire contents of the history
 * stack in the form of a string array.
 */
char **getHistory(History *history) {
  // Allocate space for the array and handle malloc issues
  char **array = (char **) malloc(history->size * sizeof(char *) + 1);
  if (array == NULL) {
    return NULL;
  }
  // Fill the array starting with the
  // bottom of the stack and end with NULL
  array[history->size] = NULL;
  Node *node = NULL;
  if (history->size != 0) {
    node = history->bottom;
  }
  for (int i = 0; i < history->size; i++) {
    array[i] = node->command;
    node = node->previous;
  }
  // Return the history in string array form
  return array;
}

/*
 * Returns the size of the history stack.
 */
int getHistorySize(History *history) {
  return history->size;
}

/*
 * Deallocates the memory for the history stack
 * and its constituent elements.
 */
void freeHistory(History *history) {
  // Free each node
  while (history->size > 0) {
    free(popHistory(history));
  }
  // Free the history stack itself
  free(history);
}
