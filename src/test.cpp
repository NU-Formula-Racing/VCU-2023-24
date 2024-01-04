// uncomment to disable assert()
// #define NDEBUG
#include <cassert>
 
// Use (void) to silence unused warnings.
#define assertm(exp, msg) assert(((void)msg, exp))
 
#include <Arduino.h>
#include <map>

#include "inverter_driver.h"
#include "throttle_driver.h"
#include "virtualTimer.h"
#include "teensy_can.h"

#define SERIAL_DEBUG

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

int test()
{
    assert(2+2==4);
    return 0;
}