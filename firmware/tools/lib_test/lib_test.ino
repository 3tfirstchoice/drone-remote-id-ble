// lib_test.ino - kiem tra bien dich thu vien id_open va khoi tao BLE
#include "id_open.h"

ID_OpenDrone   squitter;
UTM_parameters params;

void setup() {
  Serial.begin(115200);
  delay(3000);
  Serial.println("# lib_test start");
  memset(&params, 0, sizeof(params));
  strcpy(params.UAV_id, "LAB-ESP32-0001");
  params.ID_type = ODID_IDTYPE_SERIAL_NUMBER;
  params.UA_type = ODID_UATYPE_HELICOPTER_OR_MULTIROTOR;
  squitter.init(&params);
  Serial.println("# lib_test init done");
}

void loop() {}