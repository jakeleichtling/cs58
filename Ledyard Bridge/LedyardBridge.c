#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "LedyardBridge.h"

/* Global Variables */

// Awakens cars waiting to get on the bridge.
pthread_cond_t cvar[] = { PTHREAD_COND_INITIALIZER, PTHREAD_COND_INITIALIZER };

// Protects the bridge state.
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int num_cars_on_bridge = 0;
int bridge_direction;

/* Function Prototypes */
void arriveBridge(int direction, int name);
void onBridge(int direction, int name);
void exitBridge(int direction, int name);

/* Function Implementations */

/*
 Main method for each thread.
 */
void *oneCar(void *direction_name_sleep_void_pointer) {
  DirectionNameSleep *direction_name_sleep = (DirectionNameSleep *)direction_name_sleep_void_pointer;

  int direction = direction_name_sleep->direction;
  int name = direction_name_sleep->name;
  float sleep_duration = direction_name_sleep->sleep_duration;

  arriveBridge(direction, name); // Now the car is on the bridge.

  sleep(sleep_duration);

  onBridge(direction, name);

  sleep(sleep_duration);

  exitBridge(direction, name); // Now the car is off the bridge.

  return NULL;
}

/*
 First the thread acquires the mutex. Then it checks whether it can enter the bridge
 (i.e. no cars on bridge or < MAX_CARS on bridge going in same direction). If it can,
 it gets on the bridge. Otherwise, it waits to be awoken by an exiting car.
 */
void arriveBridge(int direction, int name) {
  printf("%d: I want to go on the bridge with direction: %d\n", name, direction);
  int rc;

  // Obtain the lock that protects the bridge state.
  rc = pthread_mutex_lock(&mutex);
  if (rc) {
    fprintf(stderr, "Mutex lock failed.\n");
    exit(-1);
  }

  // Can I get on the bridge?
  while (true) {
    if (0 == num_cars_on_bridge) {
      printf("%d: I'm the first car on the bridge going in direction: %d\n", name, direction);
      num_cars_on_bridge++;
      bridge_direction = direction;
      break;
    } else if (direction == bridge_direction && num_cars_on_bridge < MAX_CARS) {
      num_cars_on_bridge++;
      printf("%d: I'm car #%d on the bridge going in direction: %d\n", name, num_cars_on_bridge, direction);
      break;
    } else {
      printf("%d: I can't get on the bridge with direction %d. The bridge has %d cars and is in direction %d\n", name, direction, num_cars_on_bridge, bridge_direction);
      pthread_cond_wait(&cvar[direction], &mutex);
    }
  }

  rc = pthread_mutex_unlock(&mutex);
  if (rc) {
    fprintf(stderr, "Mutex release failed.\n");
    exit(-1);
  }
}

/*
 Do stuff while you are on the bridge!
 */
void onBridge(int direction, int name) {
  printf("%d: I'm on the bridge, yo, going in direction %d\n", name, direction);
}

/*
 Get off the bridge!
 */
void exitBridge(int direction, int name) {
  printf("%d: I want to get off the bridge with direction: %d\n", name, direction);
  int rc;

  // Obtain the lock that protects the bridge state.
  rc = pthread_mutex_lock(&mutex);
  if (rc) {
    fprintf(stderr, "Mutex lock failed.\n");
    exit(-1);
  }

  // Get off the bridge.
  bool last_car_on_bridge = (num_cars_on_bridge == 1);
  num_cars_on_bridge--;
  printf("%d: I got off the bridge going in direction %d. There are now %d cars on the bridge.\n",
	 name, direction, num_cars_on_bridge);

  rc = pthread_mutex_unlock(&mutex);
  if (rc) {
    fprintf(stderr, "Mutex release failed.\n");
    exit(-1);
  }

  // Definitely signal the cars waiting to go in the same direction as me.
  pthread_cond_broadcast(&cvar[direction]);

  // Only signal the cars waiting to go in the other direction as me if I
  // am the last car on the bridge.
  if (last_car_on_bridge) {
    int opposite_direction = direction == TO_NORWICH ? TO_HANOVER : TO_NORWICH;
    pthread_cond_broadcast(&cvar[opposite_direction]);
  }
}
