#include <Arduino.h>

#include <map>

#include "LUT.h"
#include "inverter_driver.h"
#include "teensy_can.h"
#include "throttle_driver.h"
#include "virtualTimer.h"

#define SERIAL_DEBUG

// All messages received on priority bus
// All messages sent on both buses
TeensyCAN<1> can_bus_priority{};
// TeensyCAN<2> can_bus_gen{};
// TeensyCAN<3> inverted_bus{};

// Structure for handling timers
VirtualTimerGroup read_timer;

// Instantiate throttles
Throttle throttle{can_bus_priority};
// Throttle throttle_gen{can_bus_gen};

// Instantiate inverters
Inverter inverter(can_bus_priority);
// Inverter inverter_gen(can_bus_gen);

enum BMSState
{
    kShutdown = 0,
    kPrecharge = 1,
    kActive = 2,
    kCharging = 3,
    kFault = 4
};

enum BMSCommand
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

#define DRIVE_PIN 9
#define MAX_TORQUE_ALLOWED 20
uint8_t maxtorque = 230;
bool debug = true;
bool drive_lever = false;

// CAN Signals priority
CANSignal<BMSState, 0, 8, CANTemplateConvertFloat(1), CANTemplateConvertFloat(0), false> BMS_State{};
CANSignal<BMSCommand, 0, 8, CANTemplateConvertFloat(1), CANTemplateConvertFloat(0), false> BMS_Command{};
CANSignal<float, 0, 12, CANTemplateConvertFloat(0.1), CANTemplateConvertFloat(0), false> BMS_Max_discharge_current{};
CANSignal<uint8_t, 0, 8, CANTemplateConvertFloat(1), CANTemplateConvertFloat(0), false> currentState{};
CANRXMessage<1> BMS_soe_message{can_bus_priority, 0x240, BMS_Max_discharge_current};
CANRXMessage<1> BMS_status_message{can_bus_priority, 0x241, BMS_State};
CANTXMessage<1> BMS_command_message{can_bus_priority, 0x242, 8, 100, read_timer, BMS_Command};
CANTXMessage<1> Drive_status{can_bus_priority, 0x000, 8, 100, read_timer, currentState};
CANSignal<int16_t, 0, 16, CANTemplateConvertFloat(1), CANTemplateConvertFloat(0), true> maxTorqueSignal{};
CANTXMessage<1> Torque_request{can_bus_priority, 0x001, 8, 100, read_timer, maxTorqueSignal};
CANSignal<uint32_t, 0, 32, CANTemplateConvertFloat(1), CANTemplateConvertFloat(0), false> errorSignal{};
CANTXMessage<1> Inverter_error{can_bus_priority, 0x002, 8, 100, read_timer, errorSignal};
// Can Signals general
// CANSignal<BMSState, 0, 8, CANTemplateConvertFloat(1), CANTemplateConvertFloat(0), false> BMS_State_gen{};
// CANSignal<BMSCommand, 0, 8, CANTemplateConvertFloat(1), CANTemplateConvertFloat(0), false> BMS_Command_gen{};
// CANSignal<float, 8, 8, CANTemplateConvertFloat(1), CANTemplateConvertFloat(-40), false> batt_temp_gen{};
// CANRXMessage<2> BMS_message_gen{can_bus_gen, 0x241, BMS_State_gen, batt_temp_gen};
// CANTXMessage<1> BMS_command_message_gen{can_bus_gen, 0x242, 8, 100, read_timer, BMS_Command_gen};

// // BMS State - 241
// // BMS Command - 242
// // Throttle Angle - throttle.GetThrottleAngle()
// // Throttle Active - throttle.IsThrottleActive()
// // Brake Pressed - throttle.IsBrakePressed()
// // Motor Temp - inverter.GetMotorTemperature()
// // Motor RPM - inverter.GetRPM()
// // Inverter Temp - inverter.GetInverterTemperature()
// // Battery Temp - 241
int16_t getMaxTorque(int motortemp, int invtemp, float max_dis_cur, int motorrpm, int16_t throttleangle);
void requestInverter();
void state_change();

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
            if (throttle.IsBrakePressed() && drive_lever && (BMS_State == BMSState::kActive))
            {
                currentState = DRIVE;
            }
            // if there is a fault, switch to fault
            if (BMS_State == BMSState::kFault || BMS_State == BMSState::kShutdown)
            {
                currentState = OFF;
                drive_lever = false;
            }
            break;
        case DRIVE:
            // listen to BMS status
            // if BMS fault, switch to off
            if (BMS_State == BMSState::kFault)
            {
                currentState = OFF;
                drive_lever = false;
            }
            // if drive button is off, switch to neutral
            if (drive_lever == false)
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
            maxtorque = 0;
            inverter.RequestTorque(maxtorque);
            break;
        case N:
            // send message to BMS (BMS command message)
            BMS_Command = BMSCommand::NoAction;
            maxtorque = 0;
            inverter.RequestTorque(maxtorque);
            break;
        case DRIVE:
            // request torque based on pedal values
            // maxtorque = getMaxTorque((int)inverter.GetMotorTemperature(),
            //                          (int)inverter.GetInverterTemperature(),
            //                          BMS_Max_discharge_current,
            //                          (int)inverter.GetRPM(),
            //                          throttle.GetThrottleAngle());
            maxtorque = ((float)throttle.GetThrottleAngle() / 32767) * MAX_TORQUE_ALLOWED;
            // Serial.printf("Throttle Angle: %d\n", (int)throttle.GetThrottleAngle());
            // maxtorque = throttle.GetThrottleAngle();
            Serial.printf("THROTTLE input setting maxtorque to ** %d\n", maxtorque);

                // THIS IF STATEMENT IS FUCKING SHIT UP. For some reason VCU is reading the brake_pressed value from ETC incorrectly
            // if (throttle.IsBrakePressed())
            // {
            //     Serial.println("maxtorque being set to ZERO by IsBrakePressed()!!!!");
            //     maxtorque = 0;
            // }
            if (!throttle.IsThrottleActive())
            {
                Serial.println("maxtorque being set to ZERO by IsThrottleActive()!!!!");
                maxtorque = 0;
            }
            // maxTorqueSignal = (int16_t)maxtorque;

            Serial.printf("Max Torque: %d\n", maxtorque);
            
            inverter.RequestTorque(maxtorque); //maxtorque
            break;
    }
}

void test()
{
    Serial.print("State: ");
    Serial.println((int)currentState);
    Serial.print("BMS State: ");
    Serial.println(BMS_State);
    Serial.print("BMS Command: ");
    Serial.println(BMS_Command);
    Serial.printf("Drive Lever: %d\n", drive_lever);
    Serial.print("Max discharge current: ");
    Serial.println(BMS_Max_discharge_current);
    Serial.printf("Motor Temperature: %f\n", inverter.GetMotorTemperature());
    Serial.printf("Inverter Temperature: %f\n", inverter.GetInverterTemperature());
    Serial.printf("RPM: %f\n", inverter.GetRPM());
    Serial.printf("Throttle Angle: %d\n", (int)throttle.GetThrottleAngle());
    Serial.printf("Throttle Active: %d\n", throttle.IsThrottleActive());
    Serial.printf("Brake Pressed: %d\n", throttle.IsBrakePressed());
    Serial.printf("Maximum Torque: %d\n", maxtorque);
    Serial.printf("Maximum Torque Percent: %d\n", maxtorque * 100 / 230);
    // LUT info
    getMaxTorque((int)inverter.GetMotorTemperature(),
                 (int)inverter.GetInverterTemperature(),
                 BMS_Max_discharge_current,
                 (int)inverter.GetRPM(),
                 (int)throttle.GetThrottleAngle());
}

void setup()
{
// Write code here
#ifdef SERIAL_DEBUG
    // Initialize serial output
    Serial.begin(115200);  // Baud rate (Can transfer max of 115200 bits/second)
#endif

    // Initialize can bus
    can_bus_priority.Initialize(ICAN::BaudRate::kBaud1M);
    can_bus_priority.RegisterRXMessage(BMS_status_message);
    can_bus_priority.RegisterRXMessage(BMS_soe_message);
    // can_bus_priority.Initialize(ICAN::BaudRate::kBaud1M);

    // Initialize our timer(s)
    // read_timer.AddTimer(10, RequestTorque);
    read_timer.AddTimer(10, changeState);
    read_timer.AddTimer(10, processState);
    if (debug)
    {
        read_timer.AddTimer(1000, test);
    }

    // Initialize Throttle
    throttle.Initialize();
    //
    // Request values from inverter
    inverter.Initialize();
    inverter.RequestMotorTemperature(100);
    inverter.RequestRPM(100);
    inverter.RequestPowerStageTemp(100);
    inverter.RequestStatus(100);
    read_timer.AddTimer(1000, requestInverter, 5);
    read_timer.AddTimer(10, state_change);

    // Initialize drive lever
    // pinMode(DRIVE_PIN, INPUT_PULLUP);
    // attachInterrupt(digitalPinToInterrupt(DRIVE_PIN), state_change, CHANGE);
    pinMode(3, OUTPUT);
    pinMode(4, OUTPUT);
    digitalWrite(3, LOW);
    digitalWrite(4, LOW);
}

void loop()
{
    // Write code here
    delay(0);
    read_timer.Tick(millis());
    can_bus_priority.Tick();
}

int lookup(std::map<int, int> table, int key)
{
    if (key < table.begin()->first)
    {
        return table.at(table.begin()->first);
    }
    else if (key > (prev(table.end()))->first)
    {
        return table.at(prev(table.end())->first);
    }
    std::map<int, int>::iterator it = table.find(key);
    if (it != table.end())
    {
        return table.at(key);
    }
    else
    {
        it = table.begin();
        int prev = it->first;
        it++;
        while (it != table.end())
        {
            int curr = it->first;
            if (key > prev && key < curr)
            {
                return table.at(prev) - (table.at(prev) - table.at(curr)) * (key - prev) / (curr - prev);
            }
            prev = curr;
            it++;
        }
    }
    return 0;
}

int16_t getMaxTorque(int motortemp, int invtemp, float max_dis_cur, int motorrpm, int16_t throttleangle)
{
    if (motorrpm == 0)
    {
        motorrpm = 1;
    }
    int mtt = lookup(mttlut, motortemp) * 32767 / 230;
    int ita = lookup(italut, invtemp);
    // int bta = lookup(btalut, battemp);
    int mrt = lookup(mrtlut, motorrpm) * 32767 / 230;
    int tm = throttleangle;  // lookup(tmlut, throttleangle);
    int itt = ita * 0.94 * 32767 / 230;
    // int btt = 9.5488 * 540 * max_dis_cur / motorrpm * 32767 / 230;
    // int tt = tm * 2.3;
    int maxtorque = std::min({mtt, mrt, itt, tm});
    return static_cast<int16_t>(maxtorque);
}

float calc_slip(int fws, int rws)
{
    if (fws == 0)
    {
        fws = 1;
    }
    return ((float)rws / fws - 1);
}

int pid(float slip, float target, float eprev, float dt)
{
    float kp, ki, kd;
    float e = slip - target;
    float eint = e + eprev;
    float eder = (e - eprev) / dt;
    float u = kp * e + ki * eint + kd * eder;
    return u;
}

void requestInverter()
{
    // Request values from inverter
    inverter.RequestMotorTemperature(100);
    inverter.RequestRPM(100);
    inverter.RequestStatus(100);
}

void state_change()
{
    // if (digitalRead(DRIVE_PIN) == HIGH)
    // {
    //     drive_lever = false;
    // }
    // else
    // {
    //     drive_lever = true;
    // }
    // errorSignal = inverter.GetErrorStatus();
    if (inverter.GetStatus().En)
    {
        drive_lever = true;
    }
    else
    {
        drive_lever = false;
    }
}