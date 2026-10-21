// test_operator_id.cpp - bang chung firmware gui dung chu OPERATOR_ID cua config.h, ke ca dau '-'
// va du 20 ky tu (dinh dang so dang ky EU). Ban cu "LAB-OPERATOR-01" bi Drone Scanner canh bao
// khong hop le (CHANGELOG), khong phai do firmware lam mat ky tu.
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
  strncpy(in.OperatorId, OPERATOR_ID, ODID_ID_SIZE);        // dung cach id_open.cpp lam (dong 185)
  in.OperatorId[sizeof(in.OperatorId) - 1] = 0;

  ODID_OperatorID_encoded enc;
  CHECK(encodeOperatorIDMessage(&enc, &in) == ODID_SUCCESS);
  CHECK(sizeof(enc) == 25);

  const uint8_t *raw = (const uint8_t *) &enc;
  CHECK(raw[0] == ((ODID_MESSAGETYPE_OPERATOR_ID << 4) | ODID_PROTOCOL_VERSION));
  CHECK(memcmp(raw + 2, "VNM87labtest0001-abc", 20) == 0);        // du 20 byte chu, gom dau '-'
  CHECK(raw[2 + 16] == '-');                                // dau '-' o vi tri thu 17

  memset(&out, 0, sizeof out);
  CHECK(decodeOperatorIDMessage(&out, &enc) == ODID_SUCCESS);
  CHECK_STR(out.OperatorId, "VNM87labtest0001-abc");
}

void run_operator_id_tests(void) {
  RUN(test_operator_id_bytes_keep_hyphens);
}
