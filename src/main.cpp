#include <Arduino.h>
#include "virtualTimer.h"

#define SERIAL_DEBUG

#include "esp_can.h"
// The tx and rx pins are constructor arguments to ESPCan, which default to TX = 5, RX = 4
ESPCAN can_bus{10, gpio_num_t::GPIO_NUM_22, gpio_num_t::GPIO_NUM_21};

// Structure for handling timers
VirtualTimerGroup read_timer;

void setup()
{
#ifdef SERIAL_DEBUG
    // Initialize serial output
    Serial.begin(9600);  // Baud rate (Can transfer max of 9600 bits/second)
#endif

    // Initialize can bus
    can_bus.Initialize(ICAN::BaudRate::kBaud1M);
}

void loop()
{
    delay(0);
    read_timer.Tick(millis());
    can_bus.Tick();
}