// test_operator_id.cpp - test ma hoa Operator ID bang opendroneid.c
#include <Arduino.h>
#include "check.h"
#include "config.h"
extern "C" {
#include "opendroneid.h"
}

static void test_operator_id_bytes_keep_hyphens(void) {
  ODID_OperatorID_data in, out;
  odid_initOperatorIDData(&in);
  in.OperatorIdType = ODID_OPERATOR_ID;
  strncpy(in.OperatorId, OPERATOR_ID, ODID_ID_SIZE);
  in.OperatorId[sizeof(in.OperatorId) - 1] = 0;

  ODID_OperatorID_encoded enc;
  CHECK(encodeOperatorIDMessage(&enc, &in) == ODID_SUCCESS);
  CHECK(sizeof(enc) == 25);

  const uint8_t *raw = (const uint8_t *) &enc;
  CHECK(raw[0] == ((ODID_MESSAGETYPE_OPERATOR_ID << 4) | ODID_PROTOCOL_VERSION));
  CHECK(memcmp(raw + 2, "LAB-OPERATOR-01", 15) == 0);
  for (int i = 2 + 15; i < 2 + 20; i++) CHECK(raw[i] == 0);

  memset(&out, 0, sizeof out);
  CHECK(decodeOperatorIDMessage(&out, &enc) == ODID_SUCCESS);
  CHECK_STR(out.OperatorId, "LAB-OPERATOR-01");
}

void run_operator_id_tests(void) {
  RUN(test_operator_id_bytes_keep_hyphens);
}
