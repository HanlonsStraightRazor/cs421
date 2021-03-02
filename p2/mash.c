#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "history.h"
#include "commandlist.h"

#define FANCY_PROMPT "mash> "
#define MAX_INPUT_LENGTH 100
#define HISTORY_SIZE 5

CommandList *parse(char *input, History *history);
int eval(CommandList *list, History *history);
int isEmpty(char *string);
int isValid(char *firstToken, char *secondToken);
int isShellChar(char *string);

/*
 * Main executes the REPL and handles errors.
 */
int main() {
  // Create variables for the prompt, input string, and history
  char *prompt = FANCY_PROMPT;
  char input[MAX_INPUT_LENGTH + 1];
  History *history = initHistory(HISTORY_SIZE);
  if (history == NULL) {
    fprintf(stderr, "Could not allocate memory for command history.\n");
    return 1;
  }
  // Start REPL
  for (;;) {
    // Output prompt
    printf("%s", prompt);
    // Read input
    if (!fgets(input, MAX_INPUT_LENGTH + 1, stdin)) {
      fprintf(stderr, "Could not read command(s).\n");
      return 2;
    }
    input[MAX_INPUT_LENGTH] = '\0';
    if (memchr(input, '\n', MAX_INPUT_LENGTH + 1)) {
      *((char *) memchr(input, '\n', MAX_INPUT_LENGTH + 1)) = '\0';
    }
    // Parse input
    CommandList *list = parse(input, history);
    if (list == NULL) {
      fprintf(stderr, "Could not parse command(s).\n");
      return 3;
    }
    // Evaluate input (and maybe exit)
    if (eval(list, history)) {
      // Prevent memory leaks
      freeCommandList(list);
      break;
    }
    // See above
    freeCommandList(list);
  }
  // See above
  freeHistory(history);
}

/*
 * Parses a given shell command string
 * and returns a CommandList (which is
 * sorta like an AST but with a linked list).
 */
CommandList *parse(char *input, History *history) {
  CommandList *list = initCommandList();
  if (list == NULL) {
    return NULL;
  }
  char *lastToken = NULL;
  char *token = strtok(input, " \t\r\n");
  while (token != NULL) {
    if (!isEmpty(token)) {
      if (isValid(lastToken, token)) {
        if (strcmp(token, "r") == 0) {
          token = strtok(NULL, " \t\r\n");
          if (token == NULL) {
            return NULL;
          }
          char **historyList = getHistory(history);
          if (atoi(token) <= getHistorySize(history) && atoi(token) >= 1) {
            token = strdup(historyList[getHistorySize(history) - atoi(token)]);
          } else {
            return NULL;
          }
          char *tok = strtok(token, " \t\r\n");
          while (tok != NULL) {
            if (!isEmpty(token)) {
              addToken(list, tok);
            }
            tok = strtok(NULL, " \t\r\n");
          }
          free(token);
          free(historyList);
        } else {
          addToken(list, token);
        }
      } else {
        freeCommandList(list);
        return NULL;
      }
    }
    lastToken = token;
    token = strtok(NULL, " \t\r\n");
  }
  char **parseList = getList(list);
  if (parseList == NULL) {
    return list;
  }
  int size = 0;
  int spaces = -1;
  for (int i = 0; parseList[i] != NULL; i++) {
    size += strlen(parseList[i]);
    spaces++;
  }
  size++;
  char *command = (char *) malloc((size + spaces) * sizeof(char));
  command[size + spaces - 1] = '\0';
  size = 0;
  for (int i = 0; parseList[i] != NULL; i++) {
    int j = 0;
    for (; j < strlen(parseList[i]); j++) {
      command[size + j] = parseList[i][j];
    }
    if (parseList[i + 1]) {
      command[size + j] = ' ';
      size += j + 1;
    }
  }
  if (!((strncmp(parseList[0], "hist", 4) == 0) && (parseList[1] == NULL))) {
    pushHistory(history, command);
  }
  free(command);
  free(parseList);
  return list;
}

/*
 * Evaluates a CommandList.
 */
int eval(CommandList *list, History *history) {
  char *token = nextToken(list);
  bool isPipe = 0;
  bool arePipes = 0;
  int pipeline[2];
  while (token != NULL) {
    if (strcmp(token, "quit") == 0) {
      free(token);
      return 1;
    } else if (strcmp(token, "hist") == 0) {
      char **historyList = getHistory(history);
      if (historyList == NULL) {
        printf("\n");
      } else {
        for (int i = 0; i < HISTORY_SIZE && (historyList[i] != NULL); i++) {
          printf("%d %s\n", getHistorySize(history) - i, historyList[i]);
        }
      }
      free(token);
      token = nextToken(list);
    } else {
      char **argsList = (char **) malloc(sizeof(char *));
      int size = 1;
      while (token != NULL && !isShellChar(token)) {
        argsList = (char **) realloc(argsList, ++size * sizeof(char *));
        argsList[size - 2] = token;
        token = nextToken(list);
      }
      argsList[size - 1] = NULL;
      bool callWait = 1;
      bool redirect = 0;
      if (token) {
        callWait = strncmp(token, "&", 1) != 0;
        redirect = strncmp(token, ">", 1) == 0;
        isPipe = strncmp(token, "|", 1) == 0;
        token = nextToken(list);
      }
      if (isPipe) {
        if(pipe(pipeline)) {
          perror("Ope, can't open write pipe.\n");
          return 2;
        }
      }
      int pid = fork();
      if (pid < 0) {
        perror("Ope, can't fork.\n");
      } else if (pid == 0) {
        if (arePipes) {
          close(pipeline[1]);
          dup2(pipeline[0], STDIN_FILENO);
          close(pipeline[0]);
        }
        if (isPipe) {
          close(pipeline[0]);
          dup2(pipeline[1], STDOUT_FILENO);
          close(pipeline[1]);
        } else if (redirect) {
          close(STDOUT_FILENO);
          open(token, O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);
        }
        execvp(argsList[0], argsList);
        perror("Ope, can't execute that.");
        exit(1);
      } else {
        if (isPipe) {
          char **argsList2 = (char **) malloc(sizeof(char *));
          int size2 = 1;
          while (token != NULL && !isShellChar(token)) {
            argsList2 = (char **) realloc(argsList2, ++size * sizeof(char *));
            argsList2[size2 - 2] = token;
            token = nextToken(list);
          }
          argsList2[size - 1] = NULL;
          if (token) {
            arePipes = strncmp(token, "|", 1) == 0;
            token = nextToken(list);
          } else {
            arePipes = false;
          }
          int pid2 = fork();
          if (pid2 < 0) {
            perror("Ope, can't fork.\n");
          } else if (pid2 == 0) {
            close(pipeline[1]);
            dup2(pipeline[0], STDIN_FILENO);
            close(pipeline[0]);
            if (arePipes) {
              close(pipeline[0]);
              dup2(pipeline[1], STDOUT_FILENO);
              close(pipeline[1]);
            }
            execvp(argsList[0], argsList);
            perror("Ope, can't execute that.");
            exit(1);
          } else {
            wait(NULL);
          }
        } else if (callWait) {
          wait(NULL);
        }
      }
    }
  }
  return 0;
}

/*
 * Determines if a given string is empty.
 */
int isEmpty(char *string) {
  return string[0] == '\0';
}

/*
 * Determines if a given 2 token sequence is valid.
 */
int isValid(char *firstToken, char *secondToken) {
  return !(isShellChar(firstToken) && isShellChar(secondToken));
}

/*
 * Determines if a given string is a null terminated shell character.
 */
int isShellChar(char *string) {
  if (!string) {
    return 0;
  }
  if (strlen(string) > 1) {
    return 0;
  }
  return !strncmp(string, "&", 1)
         || !strncmp(string, ">", 1)
         || !strncmp(string, "|", 1);
}
