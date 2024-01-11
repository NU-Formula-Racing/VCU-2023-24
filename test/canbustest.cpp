#include <Arduino.h>
#include <map>

#include "inverter_driver.h"
#include "throttle_driver.h"
#include "virtualTimer.h"
#include "teensy_can.h"
#include "LUT.h"

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
    DRIVE
};

state currentState = OFF;
bool drivelever = false;
int maxtorque;

// CAN Signals
CANSignal<BMSState, 0, 8, CANTemplateConvertFloat(1), CANTemplateConvertFloat(0), false> BMS_State{};
CANSignal<BMSCommand, 0, 8, CANTemplateConvertFloat(1), CANTemplateConvertFloat(0), false> BMS_Command{};
CANSignal<float, 8, 8, CANTemplateConvertFloat(0.1), CANTemplateConvertFloat(0), false> batt_temp{};
CANRXMessage<2> BMS_message{can_bus, 0x241, BMS_State, batt_temp};
CANTXMessage<1> BMS_command_message{can_bus, 0x242, 8, 100, read_timer, BMS_Command};
// BMS State - 241
// BMS Command - 242
// Throttle Angle - throttle.GetThrottleAngle()
// Throttle Active - throttle.IsThrottleActive()
// Brake Pressed - throttle.IsBrakePressed()
// Motor Temp - inverter.GetMotorTemperature()
// Motor RPM - inverter.GetRPM()
// Inverter Temp - inverter.GetInverterTemperature()
// Battery Temp - 241

void changeState()
{
    // Write code here
    switch (currentState)
    {
        case OFF:
            // If BMS is active, switch to N
            if (BMS_State == BMSState::kActive)
            {
                currentState = N;
            }
            break;
        case N:
            // if drive button and brake pressed and potentiometers agree, switch to DRIVE
            if (throttle.IsBrakePressed() && drivelever && throttle.IsThrottleActive())
            {
                BMS_Command = BMSCommand::NoAction;
                currentState = DRIVE;
            }
            // if there is a fault, switch to fault
            if (BMS_State == BMSState::kFault || BMS_State == BMSState::kShutdown)
            {
                currentState = OFF;
            }
            break;
        case DRIVE:
            // listen to BMS status
            // if BMS fault, switch to off
            if (BMS_State != BMSState::kActive)
            {
                currentState = OFF;
            }
            // if drive button is off and speed > threshold, switch to fault drive
            if (drivelever == false)
            {
                currentState = N;
            }
            break;
    }
}

void processState()
{
    // Write code here
    switch (currentState)
    {
        case OFF:
            // do nothing
            BMS_Command = BMSCommand::PrechargeAndCloseContactors;
            inverter.RequestTorque(0);
            break;
        case N:
            // send message to BMS (BMS command message)
            // PrechargeAndCloseContactors
            BMS_Command = BMSCommand::NoAction;
            inverter.RequestTorque(0);
            break;
        case DRIVE:
            // request torque based on pedal values
            maxtorque = getMaxTorque(inverter.GetMotorTemperature(), inverter.GetInverterTemperature(), batt_temp, inverter.GetRPM(), throttle.GetThrottleAngle());
            inverter.RequestTorque(maxtorque/230);
            break;
    }
}

void test()
{
    Serial.print("Hello");
}

void setup()
{
    // Write code here
    #ifdef SERIAL_DEBUG
    // Initialize serial output
    Serial.begin(115200);  // Baud rate (Can transfer max of 115200 bits/second)
    #endif

    // Initialize can bus
    can_bus.Initialize(ICAN::BaudRate::kBaud1M);
    can_bus_priority.Initialize(ICAN::BaudRate::kBaud1M);

    // Initialize our timer(s)
    // read_timer.AddTimer(10, RequestTorque);
    read_timer.AddTimer(10, changeState);
    read_timer.AddTimer(10, processState);
    read_timer.AddTimer(1000, test);

    // Request values from inverter
    inverter.RequestMotorTemperature(100);
    inverter.RequestRPM(100);
}

void loop()
{
    // Write code here
    delay(0);
    read_timer.Tick(millis());
    can_bus.Tick();
}

int lookup(std::map<int, int> table, int key) {
    std::map<int, int>::iterator it = table.find(key);
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

int getMaxTorque(int motortemp, int invtemp, int battemp, int motorrpm, int throttleangle) {
    int mtt = lookup(mttlut, motortemp);
    int ita = lookup(italut, invtemp);
    int bta = lookup(btalut, battemp);
    int mrt = lookup(mrtlut, motorrpm);
    int tm = lookup(tmlut, motortemp);
    int itt = ita*0.94;
    int btt = 9.5488*540*bta/motorrpm;
    int tt = tm*230/100.0;
    int maxtorque = std::min({mtt, mrt, itt, btt, tt});
    return maxtorque;
}