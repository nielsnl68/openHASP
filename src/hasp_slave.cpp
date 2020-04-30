/*********************
 *      INCLUDES
 *********************/
#include "hasp_slave.h"
#include <Arduino.h>
#include "ArduinoJson.h"
#include "ArduinoLog.h"
#include "hasp_dispatch.h"
#include "hasp_gui.h"
#include "tasmotaSlave.h"

// set RX and TX pins
HardwareSerial Serial2(PD6, PD5);
TasmotaSlave slave(&Serial2);

#define slaveNodeTopic "hasp/"

unsigned long updateLedTimer = 0;  // timer in msec for tele mqtt send
unsigned long updatLedPeriod = 1000;  // timer in msec for tele mqtt send

bool ledstate = false;


void IRAM_ATTR slave_send_state(const __FlashStringHelper * subtopic, const char * payload)
{
  // page = 0
  // p[0].b[0].attr = abc
  // dim = 100
  // idle = 0/1
  // light = 0/1
  // brightness = 100

  char cBuffer[strlen(payload) + 64];
  memset(cBuffer, 0 ,sizeof(cBuffer));
  snprintf(cBuffer, sizeof(cBuffer), PSTR("publish %sstate/%s %s"), slaveNodeTopic ,subtopic, payload);
  slave.ExecuteCommand((char*)cBuffer);

  // Log after char buffers are cleared
  Log.notice(F("TAS PUB: %sstate/%S = %s"), slaveNodeTopic, subtopic, payload);
}

void IRAM_ATTR slave_send_obj_attribute_str(uint8_t pageid, uint8_t btnid, const char * attribute, const char * data)
{
  char cBuffer[192];
  memset(cBuffer, 0 ,sizeof(cBuffer));
  snprintf_P(cBuffer, sizeof(cBuffer), PSTR("publish %sstate/json {\"p[%u].b[%u].%s\":\"%s\"}"), slaveNodeTopic, pageid, btnid, attribute, data);
  slave.ExecuteCommand((char*)cBuffer);
  // Log after char buffers are cleared
  Log.notice(F("TAS PUB: %sstate/json = {\"p[%u].b[%u].%s\":\"%s\"}"), slaveNodeTopic, pageid, btnid, attribute, data);
}

void slave_send_input(uint8_t id, const char * payload)
{
  // Log.trace(F("MQTT TST: %sstate/input%u = %s"), mqttNodeTopic, id, payload); // to be removed

  char cBuffer[strlen(payload) + 64];
  memset(cBuffer, 0 ,sizeof(cBuffer));
  snprintf_P(cBuffer, sizeof(cBuffer), PSTR("publish %sstate/input%u %s"), slaveNodeTopic, id, payload);
  slave.ExecuteCommand((char*)cBuffer);

  // Log after char buffers are cleared
  Log.notice(F("TAS PUB: %sstate/input%u = %s"), slaveNodeTopic, id, payload);
}

// void slave_send_statusupdate()
// { // Periodically publish a JSON string indicating system status
//     char data[3 * 128];
//     {
//         char buffer[128];
//         snprintf_P(data, sizeof(data), PSTR("{\"status\":\"available\",\"version\":\"%s\",\"uptime\":%lu,"),
//                    haspGetVersion().c_str(), long(millis() / 1000));
//         strcat(buffer, data);
//         snprintf_P(buffer, sizeof(buffer), PSTR("\"heapFree\":%u,\"heapFrag\":%u,\"espCore\":\"%s\","),
//                    ESP.getFreeHeap(), halGetHeapFragmentation(), halGetCoreVersion().c_str());
//         strcat(data, buffer);
//         snprintf_P(buffer, sizeof(buffer), PSTR("\"espCanUpdate\":\"false\",\"page\":%u,\"numPages\":%u}"),
//                    haspGetPage(), (HASP_NUM_PAGES));
//         strcat(data, buffer);
//         snprintf_P(buffer, sizeof(buffer), PSTR("\"tftDriver\":\"%s\",\"tftWidth\":%u,\"tftHeight\":%u}"),
//                    tftDriverName().c_str(), (TFT_WIDTH), (TFT_HEIGHT));
//         strcat(data, buffer);
//     }
//     slave_send_state(F("statusupdate"), data);
//     debugLastMillis = millis();
// }

void TASMO_DATA_RECEIVE(char *data)
{
  Log.verbose(F("TAS: Slave IN [%s]"), data);

  char slvCmd[20],slvVal[60];
  memset(slvCmd, 0 ,sizeof(slvCmd));
  memset(slvVal, 0 ,sizeof(slvVal));
  sscanf(data,"%s %s", slvCmd, slvVal);

  Log.verbose(F("TAS: Cmd[%s] Val[%s]"), slvCmd, slvVal);

  if (!strcmp(slvCmd, "calData")){
    if (strlen(slvVal) != 0) {
      char cBuffer[strlen(slvVal) + 24];
      memset(cBuffer, 0 ,sizeof(cBuffer));
      snprintf_P(cBuffer, sizeof(cBuffer), PSTR("{'calibration':[%s]}"), slvVal);
      dispatchConfig("gui",cBuffer);
    } else {
      dispatchConfig("gui","");
    }
  } else if (!strcmp(slvCmd, "jsonl")) {
    dispatchJsonl(slvVal);
  } else {
    char cBuffer[strlen(data)+1];
    snprintf_P(cBuffer, sizeof(cBuffer), PSTR("%s=%s"), slvCmd, slvVal);
    dispatchCommand(cBuffer);
  }
}

void TASMO_EVERY_SECOND(void)
{
  if (ledstate) {
    ledstate = false;
    digitalWrite(HASP_OUTPUT_PIN, 1);
    // Log.verbose(F("LED OFF"));
  } else {
    ledstate = true;
    digitalWrite(HASP_OUTPUT_PIN, 0);
    // Log.verbose(F("LED ON"));
  }
}

void slaveSetup()
{
  Serial2.begin(HASP_SLAVE_SPEED);
  // slave.attach_FUNC_EVERY_SECOND(TASMO_EVERY_SECOND);
  slave.attach_FUNC_COMMAND_SEND(TASMO_DATA_RECEIVE);

  Log.notice(F("TAS: HASP SLAVE LOADED"));
}

void slaveLoop(void)
{
  slave.loop();
  // demo code to run the led without tasmota
  // if ((millis() - updateLedTimer) >= updatLedPeriod) {
  //     updateLedTimer = millis();
  //     TASMO_EVERY_SECOND();
  // }

}