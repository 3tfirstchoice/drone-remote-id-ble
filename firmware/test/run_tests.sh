#!/usr/bin/env bash
# run_tests.sh - bien dich va chay test firmware tren may tinh (g++), khong can board ESP32.
# Yeu cau: g++, gcc, git, mang de tai TinyGPSPlus lan dau (ve thu muc .deps/, da .gitignore).
# Chay:  bash firmware/test/run_tests.sh
set -euo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
FW="$(dirname "$HERE")"
DEPS="$HERE/.deps"
BUILD="$HERE/.build"
TINYGPS_COMMIT=0a20575      # TinyGPSPlus 1.1.0 (mikalhart), ban Arduino Library Manager cung phien ban

if [ ! -d "$DEPS/TinyGPSPlus/src" ]; then
  mkdir -p "$DEPS"
  git clone -q https://github.com/mikalhart/TinyGPSPlus.git "$DEPS/TinyGPSPlus"
  git -C "$DEPS/TinyGPSPlus" checkout -q "$TINYGPS_COMMIT"
fi
mkdir -p "$BUILD"

INC=(-I "$HERE/stubs" -I "$FW/rid_tx" -I "$FW/libraries/utm" -I "$FW/libraries/id_open" -I "$DEPS/TinyGPSPlus/src")
# settimeofday doi ten de test khong bao gio dung vao dong ho that cua may
DEFS=(-DUSE_GPS=1 -DGPS_SOURCE_TESTING -Dsettimeofday=stub_settimeofday)

gcc -std=c11 -Wall -Wextra -Wno-unused-parameter -Wno-array-bounds -Wno-stringop-truncation \
    -c "$FW/libraries/id_open/opendroneid.c" -o "$BUILD/opendroneid.o"

# Thu vien ben thu ba: bien dich rieng, tat canh bao (khong phai code cua du an). -DARDUINO de no dung millis() cua stub
g++ -std=c++17 -w -DARDUINO=10819 "${INC[@]}" -c "$DEPS/TinyGPSPlus/src/TinyGPS++.cpp" -o "$BUILD/TinyGPS++.o"

g++ -std=c++17 -Wall -Wextra -Wno-unused-parameter -DARDUINO=10819 "${DEFS[@]}" "${INC[@]}" \
    "$HERE/run_tests.cpp" "$HERE/test_gps_source.cpp" "$HERE/test_telemetry.cpp" \
    "$HERE/test_status_led.cpp" "$HERE/test_mock_waypoints.cpp" "$HERE/test_operator_id.cpp" \
    "$FW/rid_tx/gps_source.cpp" "$FW/rid_tx/telemetry.cpp" "$FW/rid_tx/status_led.cpp" \
    "$FW/rid_tx/mock_source.cpp" "$BUILD/TinyGPS++.o" "$BUILD/opendroneid.o" \
    -lm -o "$BUILD/run_tests"

"$BUILD/run_tests"
