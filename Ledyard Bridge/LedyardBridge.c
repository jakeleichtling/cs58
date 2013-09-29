#define MAX_CARS 3

#define TO_NORWICH = 0
#define TO_HANOVER = 1

oneVehicle(int direction) {
    arriveBridge(direction); // Now the car is on the bridge.

    onBridge(direction);

    exitBridge(direction); // Now the car is off the bridge.
}
