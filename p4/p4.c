#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

// Symbolic Constants //

/* The number of MLF Queues */
#define NUM_QUEUES 4

// Structs //

/* Statistics about finished processes */
typedef struct {
  // Tell scheduling algorithm the amount of time
  // remaining until the program finishes
  unsigned long time_remaining;
  // Time at which the process finished executing
  unsigned long time_finish;
  // The time the program took to finish
  unsigned long time_turnaround;
  // The time the process spent idle
  unsigned long time_latent;
} proc_stats;

/* Stores proccess information */
typedef struct {
  // Process ID number
  unsigned long pid;
  // Time at which process arrived
  unsigned long time_arrival;
  // Process duration
  unsigned long time_burst;
  // Process statistics
  proc_stats stats;
} proc;

/* Keeps track of all processes
 * and scheduling information */
typedef struct {
  // Number of processes in the system
  unsigned long num_procs;
  // Array of processes in the system
  proc *procs;
  // Round robin time quantum
  unsigned long time_quantum;
  // MLFQ S value
  unsigned long mlfq_s;
} pcb;

// Function Declarations //

pcb *parse(FILE *file);
void run_fifo(pcb *proc_ctl_blk);
void run_sjf(pcb *proc_ctl_blk);
void run_rr(pcb *proc_ctl_blk);
void run_mlfq(pcb *proc_ctl_blk);
void calc_and_print_stats(proc *p, unsigned long time);

/* Entry point
 */
int main(int argc, char *argv[]) {
  // Make sure there's only one argument
  if (argc != 2) {
    fprintf(stderr, "Invalid number of arguments\n");
    fprintf(stderr, "Usage: p4 (process information file)\n");
    return EXIT_FAILURE;
  }

  // Parse process information file
  FILE *proc_info = fopen(argv[1], "r");
  if (!proc_info) {
    perror("Unable to open process information file");
    return EXIT_FAILURE;
  }
  pcb *proc_ctl_blk = parse(proc_info);
  fclose(proc_info);

  // Run processes to completion using
  // different scheduling algorithms
  run_fifo(proc_ctl_blk);
  run_sjf(proc_ctl_blk);
  run_rr(proc_ctl_blk);
  run_mlfq(proc_ctl_blk);

  // Housekeeping and return success if all's well
  free(proc_ctl_blk->procs);
  free(proc_ctl_blk);
  return EXIT_SUCCESS;
}

/* Parses given input file for process
 * scheduling information.
 * Format:
 * NUMBER OF PROCESSES
 * ONE PROCESS PER LINE, GIVING ITS T_ARRIVAL, T_BURST
 * ...
 * ROUND ROBIN TIME QUANTUM
 * MLFQ S VALUE */
pcb *parse(FILE *file) {
  // Initialize process control block and
  // fill-ins for its numerical components
  unsigned long num_procs = 0;
  unsigned long time_quantum = 0;
  unsigned long mlfq_s = 0;
  pcb *proc_ctl_blk = (pcb *) malloc(sizeof(pcb));

  // Find number of processes
  fscanf(file, "%lu\n", &num_procs);
  proc_ctl_blk->num_procs = num_procs;

  // Allocate space for process array
  proc_ctl_blk->procs = (proc *) malloc(num_procs * sizeof(proc));

  // read process info and fill process array
  for (int i = 0; i < num_procs; i++) {
    unsigned long time_arrival = 0;
    unsigned long time_burst = 0;
    fscanf(file, "%lu,%lu\n", &time_arrival, &time_burst);
    proc_ctl_blk->procs[i].pid = i;
    proc_ctl_blk->procs[i].time_arrival = time_arrival;
    proc_ctl_blk->procs[i].time_burst = time_burst;
  }

  // Find time quantum and MLFQ S value
  fscanf(file, "%lu\n%lu", &time_quantum, &mlfq_s);
  proc_ctl_blk->time_quantum = time_quantum;
  proc_ctl_blk->mlfq_s = mlfq_s;

  // Return filled process control block
  return proc_ctl_blk;
}

/* Run all processes in the given process control block with the
 * first in first out scheduling algorithm. */
void run_fifo(pcb *proc_ctl_blk) {
  // Print header
  printf("FIFO:\n");

  // Initialize overall statistic fields
  double avg_turnaround = 0;
  double avg_latency = 0;

  // Make sure there are processes to work with
  if (proc_ctl_blk && (proc_ctl_blk->num_procs > 0)) {

    // Create and populate a process queue
    // and initialize the time_remaining field
    queue *proc_queue = queue_init();
    for (int i = 0; i < proc_ctl_blk->num_procs; i++) {
      proc *p = &proc_ctl_blk->procs[i];
      p->stats.time_remaining = p->time_burst;
      queue_enqueue(proc_queue, (void *) p);
    }

    // Run the processes in the process queue
    // and calculate stats
    for (int time = 0; !queue_empty(proc_queue); time++) {
      proc *p = (proc *) queue_peek(proc_queue);
      if ((p->time_arrival <= time) && (p->stats.time_remaining)) {
        p->stats.time_remaining--;
      } else {
        queue_dequeue(proc_queue);
        calc_and_print_stats(p, time);
        avg_turnaround += p->stats.time_turnaround;
        avg_latency += p->stats.time_latent;
        time--;
      }
    }
    avg_turnaround /= proc_ctl_blk->num_procs;
    avg_latency /= proc_ctl_blk->num_procs;

    // Free process queue
    queue_free(proc_queue);
  }

  // Print statistics
  printf("  Average turnaround time: %lf\n", avg_turnaround);
  printf("  Average latency:         %lf\n", avg_latency);
}

/* Run all processes in the given process control block with the
 * shortest job first scheduling algorithm. */
void run_sjf(pcb *proc_ctl_blk) {
  // Print header
  printf("SJF:\n");

  // Initialize overall statistic fields
  double avg_turnaround = 0;
  double avg_latency = 0;

  // Make sure there are processes to work with
  if (proc_ctl_blk && (proc_ctl_blk->num_procs > 0)) {

    // Create and populate a process array
    proc *proc_array[proc_ctl_blk->num_procs];
    for (int i = 0; i < proc_ctl_blk->num_procs; i++) {
      proc *p = &proc_ctl_blk->procs[i];
      p->stats.time_remaining = p->time_burst;
      proc_array[i] = p;
    }

    // Run the processes in the process array
    // and calculate stats
    proc *shortest = NULL;
    unsigned long shortest_index = 0;
    unsigned long procs_remaining = proc_ctl_blk->num_procs;
    for (int time = 0; procs_remaining; time++) {
      // Remove shortest job from the array if it's done
      if (shortest && !(shortest->stats.time_remaining)) {
        calc_and_print_stats(shortest, time);
        avg_turnaround += shortest->stats.time_turnaround;
        avg_latency += shortest->stats.time_latent;
        proc_array[shortest_index] = NULL;
        shortest = NULL;
        shortest_index = 0;
        procs_remaining--;
      }
      // Find shortest process for given time
      for (int i = 0; i < proc_ctl_blk->num_procs; i++) {
        proc *p = proc_array[i];
        if (p && (p->time_arrival <= time)) {
          if (!shortest) {
            shortest = p;
            shortest_index = i;
          } else if ((shortest->time_burst) > (p->time_burst)) {
              shortest = p;
              shortest_index = i;
          }
        }
      }
      // Execute shortest job for one unit of time
      if (shortest && (shortest->stats.time_remaining > 0)) {
        shortest->stats.time_remaining--;
      } else {
        time--;
      }
    }
    avg_turnaround /= proc_ctl_blk->num_procs;
    avg_latency /= proc_ctl_blk->num_procs;
  }

  // Print statistics
  printf("  Average turnaround time: %lf\n", avg_turnaround);
  printf("  Average latency:         %lf\n", avg_latency);
}

/* Run all processes in the given process control block with the
 * round robin scheduling algorithm. */
void run_rr(pcb *proc_ctl_blk) {
  // Print header
  printf("Round Robin:\n");

  // Initialize overall statistic fields
  double avg_turnaround = 0;
  double avg_latency = 0;

  // Make sure there are processes to work with
  if (proc_ctl_blk && (proc_ctl_blk->num_procs > 0)) {

    // Create and populate a process array
    proc *proc_array[proc_ctl_blk->num_procs];
    for (int i = 0; i < proc_ctl_blk->num_procs; i++) {
      proc *p = &proc_ctl_blk->procs[i];
      p->stats.time_remaining = p->time_burst;
      proc_array[i] = p;
    }

    // Run the processes in the process array
    // and calculate stats
    unsigned long procs_remaining = proc_ctl_blk->num_procs;
    unsigned long time = 0;
    while (procs_remaining) {
      for (int i = 0; i < proc_ctl_blk->num_procs; i++) {
        proc *p = proc_array[i];
        if (p && (p->time_arrival <= time)) {
          for (int j = 0; j < proc_ctl_blk->time_quantum; j++) {
            if (p->stats.time_remaining) {
              time++;
              p->stats.time_remaining--;
            } else {
              break;
            }
          }
          if (!(p->stats.time_remaining)) {
            calc_and_print_stats(p, time);
            avg_turnaround += p->stats.time_turnaround;
            avg_latency += p->stats.time_latent;
            proc_array[i] = NULL;
            procs_remaining--;
          }
        }
      }
    }
    avg_turnaround /= proc_ctl_blk->num_procs;
    avg_latency /= proc_ctl_blk->num_procs;
  }

  // Print statistics
  printf("  Average turnaround time: %lf\n", avg_turnaround);
  printf("  Average latency:         %lf\n", avg_latency);
}

/* Run all processes in the given process control block with the
 * multi-level feedback queue scheduling algorithm. */
void run_mlfq(pcb *proc_ctl_blk) {
  // Print header
  printf("MLFQ:\n");

  // Initialize overall statistic fields
  double avg_turnaround = 0;
  double avg_latency = 0;

  // Make sure there are processes to work with
  if (proc_ctl_blk && (proc_ctl_blk->num_procs > 0)) {

    // Create and populate process queues
    // and initialize the time_remaining field
    queue *proc_queues[NUM_QUEUES];
    for (int i = 0; i < NUM_QUEUES; i++) {
      proc_queues[i] = queue_init();
    }
    for (int i = 0; i < proc_ctl_blk->num_procs; i++) {
      proc *p = &proc_ctl_blk->procs[i];
      p->stats.time_remaining = p->time_burst;
      queue_enqueue(proc_queues[0], (void *) p);
    }

    // Run the processes in the process queues
    // and calculate stats
    unsigned long procs_remaining = proc_ctl_blk->num_procs;
    unsigned long time = 0;
    while (procs_remaining) {
      for (int i = 0; i < NUM_QUEUES; i++) {
        while (!queue_empty(proc_queues[i])) {
          proc *p = (proc *) queue_peek(proc_queues[i]);
          for (int q = 0; q < (1 << i); q++) {
            time++;
            if ((time % proc_ctl_blk->mlfq_s) == 0) {
              for (int j = 1; j < NUM_QUEUES; j++) {
                while (!queue_empty(proc_queues[j])) {
                  queue_enqueue(proc_queues[0], queue_dequeue(proc_queues[j]));
                }
              }
              i = 0;
            }
            p->stats.time_remaining--;
            if (p->stats.time_remaining < 0) {
              time--;
              p->stats.time_remaining = 0;
            }
            if (!p->stats.time_remaining) {
              queue_dequeue(proc_queues[i]);
              calc_and_print_stats(p, time);
              avg_turnaround += p->stats.time_turnaround;
              avg_latency += p->stats.time_latent;
              p = NULL;
              break;
            }
          }
          if (p) {
            if ((i - 1) == NUM_QUEUES) {
              queue_enqueue(proc_queues[i], queue_dequeue(proc_queues[i]));
            } else {
              queue_enqueue(proc_queues[i + 1], queue_dequeue(proc_queues[i]));
            }
          } else {
            procs_remaining--;
          }
        }
        if ((i - 1) == NUM_QUEUES) {
          i--;
        }
      }
    }
    avg_turnaround /= proc_ctl_blk->num_procs;
    avg_latency /= proc_ctl_blk->num_procs;

    // Free process queues
    for (int i = 0; i < NUM_QUEUES; i++) {
      queue_free(proc_queues[i]);
    }
  }

  // Print statistics
  printf("  Average turnaround time: %lf\n", avg_turnaround);
  printf("  Average latency:         %lf\n", avg_latency);
}

/* Calculates stats for a given process and prints them out. */
void calc_and_print_stats(proc *p, unsigned long time) {
  p->stats.time_finish = time;
  p->stats.time_turnaround = time - p->time_arrival;
  p->stats.time_latent = p->stats.time_turnaround - p->time_burst;
  printf("  PID %lu:\n", p->pid);
  printf("    Finish time:     %lu\n", p->stats.time_finish);
  printf("    Turnaround time: %lu\n", p->stats.time_turnaround);
  printf("    Latency:         %lu\n", p->stats.time_latent);
}
