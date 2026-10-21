// id_open.h gia lap: ID_OpenDrone ghi lai cac lan goi thay vi phat BLE
#pragma once
#include <Arduino.h>
#include <time.h>
#include <sys/time.h>
#include "utm.h"
extern "C" {
#include "opendroneid.h"
}

struct ID_OpenDrone {
  UTM_parameters params = {};
  std::string    self_id;
  int            transmit_calls = 0;
  void init(UTM_parameters *p)        { params = *p; }
  void set_self_id(char *s)           { self_id = s; }
  int  transmit(UTM_data *)           { transmit_calls++; return 0; }
};
