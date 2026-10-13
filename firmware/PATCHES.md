# Ban va thu vien

Nguon goc:
- sxjack/uav_electronic_ids @ 627a328 (2023-05-07), MIT
- opendroneid/opendroneid-core-c tag v2.0, Apache-2.0 (4 file trong id_open/)
- Commit ban goc trong repo: 99c2cef

| # | File | Thay doi | Ly do | Commit |
|---|---|---|---|---|
| 1 | id_open/id_open.h | ID_OD_WIFI_BEACON 1->0, ID_OD_BT 0->1 | Chi phat BLE; bo API Wi-Fi cu | f488c44 |
| 2 | id_open/id_open_esp32.cpp | transmit_ble2(): kiem tra ma loi, them ble_tx_cnt/ble_err_cnt | Phat hien loi BLE tren S3, phuc vu do T07 | f488c44 |
| 3 | id_open/id_open_esp32.cpp | init2(): in tx power enum | Ghi cong suat phat cho T08 | f488c44 |
| 4 | id_open/id_open_esp32.cpp | construct2(): adv_type ADV_TYPE_IND -> ADV_TYPE_NONCONN_IND | Dung de bai: non-connectable; Remote ID chi phat 1 chieu, tranh thiet bi la ket noi lam gian doan | fw/D-full |

Ban va tuy chon (them khi dung): #5 SATS_LEVEL_2 (G7), #6 last_msecs (E3).