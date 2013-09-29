#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_CARS 3

#define TO_NORWICH = 0
#define TO_HANOVER = 1

/* Global Variables */

// Awakens cars waiting to get on the bridge.
pthread_cond_t cvar = PTHREAD_COND_INITIALIZER;

// Protects the bridge state.
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int num_cars_on_bridge = 0;
int bridge_direction;

/* Function Prototypes */
void arriveBridge(int direction, char *name);
void onBridge(int direction, char *name);

/* Function Implementations */

int main(int argc, char **argv) {
  return 0;
}

/*
 First the thread acquires the mutex. Then it checks whether it can enter the bridge
 (i.e. no cars on bridge or < MAX_CARS on bridge going in same direction). If it can,
 it gets on the bridge. Otherwise, it waits to be awoken by an exiting car.
 */
void arriveBridge(int direction, char *name) {
  printf("%s: I want to go on the bridge with direction: %d", name, direction);
  int rc;

  // Obtain the lock that protects the bridge state.
  rc = pthread_mutex_lock(&mutex);
  if (rc) {
    fprintf(stderr, "Mutex lock failed.");
    exit(-1);
  }

  // Can I get on the bridge?
  while (true) {
    if (0 == num_cars_on_bridge) {
      printf("%s: I'm the first car on the bridge going in direction: %d", name, direction);
      num_cars_on_bridge++;
      bridge_direction = direction;
      break;
    } else if (direction == bridge_direction && num_cars_on_bridge < MAX_CARS) {
      printf("%s: I'm car #%d on the bridge going in direction: %d", name, num_cars_on_bridge, direction);
      num_cars_on_bridge++;
      break;
    } else {
      pthread_cond_wait(&cvar, &mutex);
    }
  }

  rc = pthread_mutex_unlock(&mutex);
  if (rc) {
    fprintf(stderr, "Mutex release failed.");
    exit(-1);
  }
}

/*
 Do stuff while you are on the bridge!
 */
void onBridge(int direction, char *name) {
  printf("%s: I'm on the bridge, yo, going in direction %d", name, direction);
}

/*
oneVehicle(int direction) {
    arriveBridge(direction); // Now the car is on the bridge.

    onBridge(direction);

    exitBridge(direction); // Now the car is off the bridge.
}
*/
