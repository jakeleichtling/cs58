#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "LedyardBridge.h"

/* Function Prototypes */

void testCase0();
void testCase1();

/* Function Implementations */

int main(int argc, char **argv) {
  testCase0();
  testCase1();

  return 0;
}

/*
 5 cars going in each direction, all released at the same time.
 */
void testCase0() {
  printf("---------------------------------------------------------------------------------------\n");
  printf("5 cars going in each direction, all released at the same time,\n");
  printf("\tsleeping for 1 second after getting on, and another second before getting off:\n");
  printf("---------------------------------------------------------------------------------------\n");

  int num_cars_to_hanover = 5;
  int num_cars_to_norwich = 5;

  DirectionNameSleep *direction_names[num_cars_to_hanover + num_cars_to_norwich];
  pthread_t car_threads[num_cars_to_hanover + num_cars_to_norwich];

  int rc;

  int i;
  for (i = 0; i < num_cars_to_hanover; i++) {
    direction_names[i] = (DirectionNameSleep *)malloc(sizeof(DirectionNameSleep));
    direction_names[i]->direction = TO_HANOVER;
    direction_names[i]->name = i;
    direction_names[i]->sleep_duration = 1;

    rc = pthread_create(&car_threads[i], NULL, oneCar, (void *)direction_names[i]);
    if (rc) {
      fprintf(stderr, "pthread_create failed at i = %d\n", i);
      exit(-1);
    }
  }

  for (; i < num_cars_to_hanover + num_cars_to_norwich; i++) {
    direction_names[i] = malloc(sizeof(DirectionNameSleep));
    direction_names[i]->direction = TO_NORWICH;
    direction_names[i]->name = i;

    rc = pthread_create(&car_threads[i], NULL, oneCar, (void *)direction_names[i]);
    if (rc) {
      fprintf(stderr, "pthread_create failed at i = %d\n", i);
      exit(-1);
    }
  }

  // Wait for all the threads and free their DirectionNames
  for (i = 0; i < num_cars_to_hanover + num_cars_to_norwich; i++) {
    rc = pthread_join(car_threads[i], NULL);

    if (rc) {
      fprintf(stderr, "pthread_join failed at i = %d\n", i);
      exit(-1);
    }

    free(direction_names[i]);
  }

  printf("---------------------------------------------------------------------------------------\n");
}

void testCase1() {
  printf("---------------------------------------------------------------------------------------\n");
  printf("10 cars going in random directions, all released at the same time,\n");
  printf("\tsleeping for 0 <= t <= 5 seconds after getting on, and another t seconds before getting off:\n");
  printf("---------------------------------------------------------------------------------------\n");

  int num_cars = 10;
  DirectionNameSleep *direction_names[num_cars];
  pthread_t car_threads[num_cars];

  int min_sleep_duration = 0;
  int max_sleep_duration = 5;

  int rc;

  int i;
  for (i = 0; i < num_cars; i++) {
    direction_names[i] = (DirectionNameSleep *)malloc(sizeof(DirectionNameSleep));
    direction_names[i]->direction = rand() % 2;
    direction_names[i]->name = i;
    direction_names[i]->sleep_duration = rand() % (max_sleep_duration + 1 - min_sleep_duration) + min_sleep_duration;

    rc = pthread_create(&car_threads[i], NULL, oneCar, (void *)direction_names[i]);
    if (rc) {
      fprintf(stderr, "pthread_create failed at i = %d\n", i);
      exit(-1);
    }
  }

  // Wait for all the threads and free their DirectionNames
  for (i = 0; i < num_cars; i++) {
    rc = pthread_join(car_threads[i], NULL);

    if (rc) {
      fprintf(stderr, "pthread_join failed at i = %d\n", i);
      exit(-1);
    }

    free(direction_names[i]);
  }

  printf("---------------------------------------------------------------------------------------\n");
}
