#ifndef COMMANDLIST_H
#define COMMANDLIST_H

typedef struct commandList CommandList;

CommandList *initCommandList();
int addToken(CommandList *list, char *token);
char *nextToken(CommandList *list);
char **getList(CommandList *list);
void freeCommandList(CommandList *list);

#endif
