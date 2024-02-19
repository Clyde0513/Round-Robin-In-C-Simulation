#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/queue.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

typedef uint32_t u32;
typedef int32_t i32;

struct process
{
  u32 pid;
  u32 arrival_time;
  u32 burst_time;
  // u32 remaining_time;

  TAILQ_ENTRY(process)
  pointers;

  /* Additional fields here */
  // u32 response_time;
  // u32 waiting_time;
  // u32 time_remaining;
  // u32 last_arrivalTime_preempted;
  u32 time_end;
  u32 run_time;
  int time_start;
  /* End of "Additional fields here" */
};

TAILQ_HEAD(process_list, process);

u32 next_int(const char **data, const char *data_end)
{
  u32 current = 0;
  bool started = false;
  while (*data != data_end)
  {
    char c = **data;

    if (c < 0x30 || c > 0x39)
    {
      if (started)
      {
        return current;
      }
    }
    else
    {
      if (!started)
      {
        current = (c - 0x30);
        started = true;
      }
      else
      {
        current *= 10;
        current += (c - 0x30);
      }
    }

    ++(*data);
  }

  printf("Reached end of file while looking for another integer\n");
  exit(EINVAL);
}

u32 next_int_from_c_str(const char *data)
{
  char c;
  u32 i = 0;
  u32 current = 0;
  bool started = false;
  while ((c = data[i++]))
  {
    if (c < 0x30 || c > 0x39)
    {
      exit(EINVAL);
    }
    if (!started)
    {
      current = (c - 0x30);
      started = true;
    }
    else
    {
      current *= 10;
      current += (c - 0x30);
    }
  }
  return current;
}

void init_processes(const char *path,
                    struct process **process_data,
                    u32 *process_size)
{
  int fd = open(path, O_RDONLY);
  if (fd == -1)
  {
    int err = errno;
    perror("open");
    exit(err);
  }

  struct stat st;
  if (fstat(fd, &st) == -1)
  {
    int err = errno;
    perror("stat");
    exit(err);
  }

  u32 size = st.st_size;
  const char *data_start = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (data_start == MAP_FAILED)
  {
    int err = errno;
    perror("mmap");
    exit(err);
  }

  const char *data_end = data_start + size;
  const char *data = data_start;

  *process_size = next_int(&data, data_end);

  *process_data = calloc(sizeof(struct process), *process_size);
  if (*process_data == NULL)
  {
    int err = errno;
    perror("calloc");
    exit(err);
  }

  for (u32 i = 0; i < *process_size; ++i)
  {
    (*process_data)[i].pid = next_int(&data, data_end);
    (*process_data)[i].arrival_time = next_int(&data, data_end);
    (*process_data)[i].burst_time = next_int(&data, data_end);
  }

  munmap((void *)data, size);
  close(fd);
}

int main(int argc, char *argv[])
{
  if (argc != 3)
  {
    return EINVAL;
  }
  struct process *data;
  u32 size;
  init_processes(argv[1], &data, &size);

  u32 quantum_length = next_int_from_c_str(argv[2]);

  struct process_list list;
  TAILQ_INIT(&list);

  u32 total_waiting_time = 0;
  u32 total_response_time = 0;

  /* Your code here */

  if (quantum_length <= 0)
  {
    free(data);
    exit(22);
  }

  u32 finished_tasks = 0;
  u32 time = 0;
  int min = 0;
  struct process *current_process = NULL;

  int i = 0;
  while (i < size)
  {
    data[i].run_time = 0;
    data[i].time_start = -1;
    if (0 > data[i].burst_time)
    {
      free(data);
      exit(22);
    }
    if (0 > data[i].arrival_time)
    {
      free(data);
      exit(22);
    }
    if (data[i].arrival_time <= data[min].arrival_time)
    {
      min = i;
    }
    i++;
  }

  // add the first process
  TAILQ_INSERT_TAIL(&list, &data[min], pointers);
  data[min].time_start = data[min].arrival_time;
  time = data[min].arrival_time;

  while (finished_tasks != size)
  {
    current_process = TAILQ_FIRST(&list);
    int i = 0;
    while (i < quantum_length)
    // for (int i = 0; i < quantum_length; ++i)
    {
      time = time + 1;

      for (int i = 0; i < size + 1; i++)
      {
        if (data[i].arrival_time <= time && data[i].time_start == -1)
        {
          TAILQ_INSERT_TAIL(&list, &data[i], pointers);
          data[i].time_start = -2;
        }
      }

      if (NULL == current_process)
        break;

      if (-2 == current_process->time_start)
        current_process->time_start = time - 1;

      if (current_process->burst_time == ++current_process->run_time)
      {
        current_process->time_end = time;
        finished_tasks = finished_tasks + 1;
        break;
      }
      i++;
    }

    TAILQ_REMOVE(&list, current_process, pointers); // remove current process if burst time == run time

    if (current_process->burst_time != current_process->run_time)
      TAILQ_INSERT_TAIL(&list, current_process, pointers);

    current_process = NULL;
  }

  int k = 0;
  while (k < size)
  {
    total_response_time += data[k].time_start - data[k].arrival_time;
    total_waiting_time += data[k].time_end - data[k].arrival_time - data[k].burst_time;
    ++k;
  }

  /*failed implementation of RR*/
  // for (u32 i = 0; i < size; i++)
  // {
  //   TAILQ_INSERT_TAIL(&list, &data[i], pointers);
  // }

  // TAILQ_FOREACH(currentElements, &list, pointers)
  // {
  //   printf("Each PID: %d\n", currentElements->pid);
  //   printf("Each Arrival Time: %d\n", currentElements->arrival_time);
  //   printf("Each Burst Time: %d\n", currentElements->burst_time);
  // }

  // u32 currentTime = 0;
  // u32 turnAroundTime = 0;
  // u32 waitingTime = 0;
  // u32 responseTime = 0;
  //  TODO: initialize current to first-arrived process

  // do
  // {
  //   // struct process *nextProcess;
  //   struct process *currentProcess;
  //   TAILQ_FOREACH(currentProcess, &list, pointers)
  //   { //&& currentProcess->burst_time
  //     if (currentProcess->arrival_time <= currentTime)
  //     {
  //       u32 timeDelta = quantum_length > currentProcess->burst_time ? currentProcess->burst_time : quantum_length; // min quantum or burst time
  //       currentTime = currentTime > currentProcess->arrival_time ? currentTime : currentProcess->arrival_time;     // max currentTime or arrival time
  //       currentTime += timeDelta;
  //       responseTime += timeDelta;
  //       // update the remaining time of the current process
  //       currentProcess->burst_time -= timeDelta;

  //       if (currentProcess->burst_time == 0)
  //       {
  //         turnAroundTime += currentTime - currentProcess->arrival_time;
  //         waitingTime += turnAroundTime - currentProcess->burst_time;
  //         TAILQ_REMOVE(&list, currentProcess, pointers);
  //       }
  //       // else
  //       // {
  //       //   TAILQ_REMOVE(&list, currentProcess, pointers);
  //       //   TAILQ_INSERT_TAIL(&list, currentProcess, pointers);
  //       // }
  //       // break;
  //     }
  //   }

  //   // move on to the next process in the linked list
  //   // currentProcess = nextProcess;

  // } while (TAILQ_EMPTY(&list) != 0);

  // for (u32 i = 0; i < size; i++)
  // {
  //   total_waiting_time += data[i].waiting_time;
  //   total_response_time += data[i].response_time;
  //   // i++;
  // }
  printf("Average waiting time: %.2f\n", (float)total_waiting_time / (float)size);
  printf("Average response time: %.2f\n", (float)total_response_time / (float)size);

  free(data);
  /* End of "Your code here" */

  return 0;
}