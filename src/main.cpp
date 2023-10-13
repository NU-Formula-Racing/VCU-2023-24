#include <Arduino.h>
// comment
#include "inverter_driver.h"
#include "throttle_driver.h"
#include "virtualTimer.h"

#define SERIAL_DEBUG

#include "esp_can.h"
// The tx and rx pins are constructor arguments to ESPCan, which default to TX = 5, RX = 4
ESPCAN can_bus{10, gpio_num_t::GPIO_NUM_22, gpio_num_t::GPIO_NUM_21};

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