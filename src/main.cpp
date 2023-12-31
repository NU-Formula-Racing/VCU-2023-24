#include <Arduino.h>
#include <map>

#include "inverter_driver.h"
#include "throttle_driver.h"
#include "virtualTimer.h"
#include "teensy_can.h"

#define SERIAL_DEBUG

TeensyCAN<2> can_bus_priority{},can_bus{};

// Structure for handling timers
VirtualTimerGroup read_timer;

// Instantiate throttle
Throttle throttle{can_bus};

// Instantiate inverter
Inverter inverter(can_bus);

enum class BMSState
{
    kShutdown = 0,
    kPrecharge = 1,
    kActive = 2,
    kCharging = 3,
    kFault = 4
};

enum class BMSCommand
{
    NoAction = 0,
    PrechargeAndCloseContactors = 1,
    Shutdown = 2
};

enum state
{
    OFF,
    N,
    DRIVE,
    FDRIVE
};

// CAN Signals

void changeState()
{
    // Write code here
}

void processState()
{
    // Write code here
}



void setup()
{
    // Write code here
}

void loop()
{
    // Write code here
}

float lookup(std::map<int, float> table, int key) {
    std::map<int, float>::iterator it = table.find(key);
    if(it != table.end()) {
        return table.at(key);
    }
    else {
        it = table.begin();
        int prev = it->first;
        it++;
        for (it; it != table.end(); it++) {
            int curr = it->first;
            if(key > prev && key < curr) {
                return (table.at(prev) + table.at(curr)) / 2;
            }
            prev = curr;
        }
    }
}