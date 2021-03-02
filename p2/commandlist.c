#include <stdlib.h>
#include <string.h>
#include "commandlist.h"

typedef struct node {
  char *token;
  struct node *next;
} Node;

struct commandList {
  Node *head;
  Node *tail;
};

/*
 * Creates a new CommandList structure.
 */
CommandList *initCommandList() {
  CommandList *list = (CommandList *) malloc(sizeof(CommandList));
  if (list == NULL) {
    return NULL;
  }
  list->head = NULL;
  list->tail = NULL;
  return list;
}

/*
 * Adds a token to a CommandList.
 */
int addToken(CommandList *list, char *token) {
  Node *node = (Node *) malloc(sizeof(Node));
  if (node == NULL) {
    return 1;
  }
  node->token = strdup(token);
  node->next = NULL;
  if (list->head == NULL) {
    list->head = node;
    list->tail = node;
  } else {
    list->tail->next = node;
    list->tail = node;
  }
  return 0;
}

/*
 * Returns the next token in the CommandList
 * and removes it.
 */
char *nextToken(CommandList *list) {
  if (list->head == NULL) {
    return NULL;
  }
  char *value = list->head->token;
  if (list->head == list->tail) {
    list->head = NULL;
    list->tail = NULL;
    return value;
  }
  Node *newHead = list->head->next;
  free(list->head);
  list->head = newHead;
  return value;
}

/*
 * Returns the command list in string form.
 */
char **getList(CommandList *list) {
  Node *node = list->head;
  if (!node) {
    return NULL;
  }
  char **array = (char **) malloc(sizeof(char *));
  if (!array) {
    return NULL;
  }
  int j = 0;
  for (int i = 2 * sizeof(char *); node != NULL; i += sizeof(char *)) {
    array[j] = node->token;
    j++;
    array = (char **) realloc(array, i);
    if (!array) {
      return NULL;
    }
    node = node->next;
  }
  array[j] = NULL;
  return array;
}

/*
 * Deallocates the memory for the CommandList.
 */
void freeCommandList(CommandList *list) {
  while (list->head) {
    free(nextToken(list));
  }
  free(list);
}
