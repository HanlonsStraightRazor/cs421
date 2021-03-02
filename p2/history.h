#ifndef HISTORY_H
#define HISTORY_H

typedef struct history History;

History *initHistory(int limit);
int pushHistory(History *history, char *command);
char *popHistory(History *history);
char **getHistory(History *history);
int getHistorySize(History *history);
void freeHistory(History *history);

#endif
