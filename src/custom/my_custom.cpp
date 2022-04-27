/* MIT License - Copyright (c) 2019-2022 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

// USAGE: - Copy this file and rename it to my_custom.cpp
//        - Change false to true on line 9

#include "hasplib.h"

#if defined(HASP_USE_CUSTOM) // <-- set this to true in your code

#include "hasp_debug.h"

void custom_setup()
{
    // Initialization code here
    randomSeed(millis());
}

void custom_loop()
{
    // Non-blocking code here, this should execute very fast!
}

void custom_every_second()
{

}

void custom_every_5seconds()
{
}

bool custom_pin_in_use(uint8_t pin)
{
}

void custom_get_sensors(JsonDocument& doc)
{
    /* Sensor Name */
    JsonObject sensor = doc.createNestedObject(F("Custom"));

    /* Key-Value pair of the sensor value */
    sensor[F("Random")] = random(256);
}

void custom_topic_payload(const char* topic, const char* payload, uint8_t source){
    // Not used
}

#endif // HASP_USE_CUSTOM