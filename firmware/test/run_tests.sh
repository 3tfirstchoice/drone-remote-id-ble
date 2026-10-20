#!/usr/bin/env bash
# run_tests.sh - bien dich va chay test firmware tren may tinh (g++), khong can board ESP32.
# Yeu cau: g++, gcc, git, mang de tai TinyGPSPlus lan dau (ve thu muc .deps/, da .gitignore).
# Chay:  bash firmware/test/run_tests.sh
#
# Gom 3 chuong trinh test va 2 buoc kiem tra tinh:
#   modules        : tung module trong rid_tx/ (GPS, log, den, waypoints, Operator ID)
#   rid_tx_mock    : rid_tx.ino voi USE_GPS 0
#   rid_tx_gps     : rid_tx.ino voi USE_GPS 1
#   waypoints.h    : 2 ban (data/ va firmware/rid_tx/) phai giong het nhau
#   gps_passthrough: kiem tra cu phap bang stub Arduino
set -euo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
FW="$(dirname "$HERE")"
REPO="$(dirname "$FW")"
DEPS="$HERE/.deps"
BUILD="$HERE/.build"
TINYGPS_COMMIT=0a20575      # TinyGPSPlus 1.1.0 (mikalhart), cung phien ban Arduino Library Manager

if [ ! -d "$DEPS/TinyGPSPlus/src" ]; then
  mkdir -p "$DEPS"
  git clone -q https://github.com/mikalhart/TinyGPSPlus.git "$DEPS/TinyGPSPlus"
  git -C "$DEPS/TinyGPSPlus" checkout -q "$TINYGPS_COMMIT"
fi
mkdir -p "$BUILD"

INC=(-I "$HERE/stubs" -I "$HERE" -I "$FW/rid_tx" -I "$FW/libraries/utm" -I "$FW/libraries/id_open"
     -I "$DEPS/TinyGPSPlus/src")
# -DARDUINO: TinyGPSPlus dung millis() cua stub. settimeofday/gettimeofday doi ten sang ham gia lap
# (stubs/stubs.cpp) de test khong bao gio dung vao dong ho that cua may.
CXX=(g++ -std=c++17 -Wall -Wextra -Wno-unused-parameter -DARDUINO=10819
     -Dsettimeofday=stub_settimeofday -Dgettimeofday=stub_gettimeofday "${INC[@]}")

# --- Thu vien: bien dich 1 lan ---
gcc -std=c11 -Wall -Wextra -Wno-unused-parameter -Wno-array-bounds -Wno-stringop-truncation \
    -c "$FW/libraries/id_open/opendroneid.c" -o "$BUILD/opendroneid.o"
# Thu vien ben thu ba: tat canh bao (khong phai code cua du an)
g++ -std=c++17 -w -DARDUINO=10819 "${INC[@]}" -c "$DEPS/TinyGPSPlus/src/TinyGPS++.cpp" -o "$BUILD/TinyGPS++.o"
LIBS=("$BUILD/TinyGPS++.o" "$BUILD/opendroneid.o" -lm)

STUBS="$HERE/stubs/stubs.cpp"
RID="$FW/rid_tx"

# --- 1. Tung module (GPS bat de bien dich gps_source.cpp) ---
"${CXX[@]}" -DUSE_GPS=1 -DGPS_SOURCE_TESTING -o "$BUILD/modules" "$STUBS" "$HERE/test_main.cpp" \
    "$HERE/test_gps_source.cpp" "$HERE/test_telemetry.cpp" "$HERE/test_status_led.cpp" \
    "$HERE/test_mock_waypoints.cpp" "$HERE/test_operator_id.cpp" \
    "$RID/gps_source.cpp" "$RID/telemetry.cpp" "$RID/status_led.cpp" "$RID/mock_source.cpp" "${LIBS[@]}"

# --- 2, 3. rid_tx.ino o ca 2 che do ---
"${CXX[@]}" -DUSE_GPS=0 -o "$BUILD/rid_tx_mock" "$STUBS" "$HERE/test_rid_tx.cpp" \
    "$RID/mock_source.cpp" "$RID/telemetry.cpp" "$RID/status_led.cpp" "${LIBS[@]}"
"${CXX[@]}" -DUSE_GPS=1 -o "$BUILD/rid_tx_gps" "$STUBS" "$HERE/test_rid_tx.cpp" \
    "$RID/gps_source.cpp" "$RID/telemetry.cpp" "$RID/status_led.cpp" "${LIBS[@]}"

# --- 4. Hai ban waypoints.h ---
if cmp -s "$REPO/data/waypoints.h" "$RID/waypoints.h"; then
  echo "waypoints.h: data/ va firmware/rid_tx/ giong nhau"
else
  echo "FAIL waypoints.h: data/ va firmware/rid_tx/ khac nhau, chay lai data/generate_mock_data.py"; exit 1
fi

# --- 5. Cu phap tools/gps_passthrough (sketch doc lap, khong dung rid_tx/) ---
printf '#include <Arduino.h>\n#include "%s"\n' "$FW/tools/gps_passthrough/gps_passthrough.ino" \
  > "$BUILD/passthrough_check.cpp"
"${CXX[@]}" -fsyntax-only "$BUILD/passthrough_check.cpp"
echo "gps_passthrough.ino: cu phap dung"

# --- Chay ---
status=0
for t in modules rid_tx_mock rid_tx_gps; do
  out="$("$BUILD/$t")" || status=1
  grep -E "FAIL|kiem tra dat" <<< "$out" || true
done
[ $status -eq 0 ] && echo "TAT CA DAT" || echo "CO TEST THAT BAI"
exit $status
