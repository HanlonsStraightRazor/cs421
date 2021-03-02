#include<assert.h>
#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<time.h>

/*
 * I used two locks: one for the number of elves waiting for
 * Santa's help, and one for the number of reindeer waiting
 * in the warming shed. I also used two condition variables:
 * one for the elves waiting to get in line, and one for the
 * elves waiting in line for Santa's help. In order to keep
 * track of the threads waiting on Santa, I created one variable
 * for counting the number of elves waiting in line for Santa's
 * help, and one variable to keep track of the number of
 * reindeer that have returned from vacation.
 */

// Minimum random wait time in milliseconds
#define TIME_MIN 100
// Maximum random wait time in milliseconds
#define TIME_MAX 2000
// Number of elf threads
#define NUM_ELVES 10
// Number of reindeer threads
#define NUM_REINDEER 9
// Number of elves needed
// to interrupt Santa
#define QUORUM 3

// Global variables
int elvesWaiting;
int reindeerBack;
pthread_cond_t queueFull;
pthread_cond_t needHelp;
pthread_mutex_t elfLock;
pthread_mutex_t reindeerLock;

// Function declarations
void waitRand();
void *beElves();
void *beReindeer();

/*
 * main
 */
int main() {
  // Initialize elvesWaiting
  elvesWaiting = 0;
  // Initialize elf lock and condition variable
  assert(pthread_mutex_init(&elfLock, NULL) == 0);
  assert(pthread_cond_init(&queueFull, NULL) == 0);
  assert(pthread_cond_init(&needHelp, NULL) == 0);
  // Declare elf threads and ids
  pthread_t elves[NUM_ELVES];
  int elfids[NUM_ELVES];
  for (int i = 0; i < NUM_ELVES; i++) {
    elfids[i] = i + 1;
  }
  // Start elf threads
  for (int i = 0; i < NUM_ELVES; i++) {
    pthread_create(elves + i, NULL, beElves, elfids + i);
  }
  // This thread is the Santa thread, so start that stuff now
  for (;;) {
    // Initialize reindeerBack
    reindeerBack = 0;
    // Initialize reindeer lock
    assert(pthread_mutex_init(&reindeerLock, NULL) == 0);
    // Declare reindeer threads and ids
    pthread_t reindeer[NUM_REINDEER];
    int reindeerids[NUM_REINDEER];
    for (int i = 0; i < NUM_REINDEER; i++) {
      reindeerids[i] = i + 1;
    }
    // Start reindeer threads
    for (int i = 0; i < NUM_REINDEER; i++) {
      pthread_create(reindeer + i, NULL, beReindeer, reindeerids + i);
    }
    // Help elves and sleep while reindeer are on vacation
    while (reindeerBack != NUM_REINDEER) {
      // Help elves in a disorderly fashion
      if (elvesWaiting == QUORUM) {
        pthread_mutex_lock(&elfLock);
        printf("Santa helping elves\n");
        waitRand();
        pthread_cond_broadcast(&needHelp);
        elvesWaiting = 0;
        pthread_cond_broadcast(&queueFull);
        printf("Santa done helping elves\n");
        pthread_mutex_unlock(&elfLock);
      }
      // Nap for a bit
      printf("Santa sleeping\n");
      waitRand();
    }
    // Once reindeer are back, deliver presents
    printf("Santa and reindeer delivering presents\n");
    waitRand();
    printf("Santa and reindeer done delivering presents\n");
  }
  return 0;
}

/*
 * Wait for a random amount of milliseconds
 * between TIME_MIN and TIME_MAX.
 */
void waitRand() {
  int num = (rand() % (TIME_MAX - TIME_MIN + 1)) + TIME_MIN;
  struct timespec time = {num / 1000, (num % 1000) * 1000000};
  int remaining = 1;
  while (remaining) {
    remaining = nanosleep(&time, &time);
  }
}

/*
 * Elves work, wait for Santa's help, or get Santa's help.
 * Elves do not terminate by themselves.
 */
void *beElves(void *num) {
  // Get id
  int id = *((int *) num);
  for (;;) {
    // Make toys
    printf("Elf %d making toys\n", id);
    waitRand();
    // Get Santa's help
    pthread_mutex_lock(&elfLock);
    printf("Elf %d waiting for other elves\n", id);
    while (elvesWaiting >= QUORUM) {
      pthread_cond_wait(&queueFull, &elfLock);
    }
    elvesWaiting++;
    printf("Elf %d waiting for help\n", id);
    pthread_cond_wait(&needHelp, &elfLock);
    printf("Elf %d helped\n", id);
    pthread_mutex_unlock(&elfLock);
  }
  return NULL;
}

/*
 * Reindeer vacation, return, and let Santa know they're back.
 * Reindeer terminate once finished.
 */
void *beReindeer(void *num) {
  // Get id
  int id = *((int *) num);
  // Go on vacation
  printf("Reindeer %d starting vacation\n", id);
  waitRand();
  // Return and let Santa know it's back
  pthread_mutex_lock(&reindeerLock);
  printf("Reindeer %d returning\n", id);
  reindeerBack++;
  printf("Reindeer %d waiting in warming shed\n", id);
  pthread_mutex_unlock(&reindeerLock);
  return NULL;
}
