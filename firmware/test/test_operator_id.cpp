// test_operator_id.cpp - bang chung firmware gui dung chu "LAB-OPERATOR-01" (co dau gach ngang).
// Ung dung Drone Scanner hien "LABOPERATOR01" kem canh bao la do ung dung kiem tra dinh dang
// (CHANGELOG: khong theo chuan dang ky EU), khong phai do firmware lam mat ky tu.
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
  CHECK(memcmp(raw + 2, "LAB-OPERATOR-01", 15) == 0);       // 15 byte chu, gom 2 dau '-'
  for (int i = 2 + 15; i < 2 + 20; i++) CHECK(raw[i] == 0); // phan con lai la byte 0

  memset(&out, 0, sizeof out);
  CHECK(decodeOperatorIDMessage(&out, &enc) == ODID_SUCCESS);
  CHECK_STR(out.OperatorId, "LAB-OPERATOR-01");
}

void run_operator_id_tests(void) {
  RUN(test_operator_id_bytes_keep_hyphens);
}
