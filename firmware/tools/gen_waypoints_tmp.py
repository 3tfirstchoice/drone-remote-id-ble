#!/usr/bin/env python3
# gen_waypoints_tmp.py - sinh waypoints.h cho 3 kich ban (1 phan tu = 1 giay)
# Cach dung (chay trong firmware/tools):  python gen_waypoints_tmp.py ../rid_tx/waypoints.h
import math
import sys

HOME_LAT = 10.7725        # phai trung HOME_LAT trong config.h
HOME_LON = 106.6578
ALT_MSL_HOME_M = 10.0     # do cao mat dat tai home (m, so voi muc nuoc bien)
ALT_AGL_M = 50.0          # bay cao 50 m so voi mat dat
SPEED_KN = 10             # 10 kn = 5.144 m/s
SPEED_MS = SPEED_KN * 0.514444
M_PER_DEG_LAT = 111320.0
M_PER_DEG_LON = 111320.0 * math.cos(math.radians(HOME_LAT))


def pt(lat, lon, speed, heading):
    return (round(lat * 1e7), round(lon * 1e7),
            round((ALT_MSL_HOME_M + ALT_AGL_M) * 10), round(ALT_AGL_M * 10),
            speed, heading)


def stationary():          # 60 giay dung yen tai home
    return [pt(HOME_LAT, HOME_LON, 0, 0) for _ in range(60)]


def straight():            # 30 giay bay huong Bac, 10 kn
    d = SPEED_MS / M_PER_DEG_LAT
    return [pt(HOME_LAT + i * d, HOME_LON, SPEED_KN, 0) for i in range(30)]


def rectangle():           # 4 canh x 10 giay, dong -> bac -> tay -> nam, 41 diem
    dlat = SPEED_MS * 10 / M_PER_DEG_LAT
    dlon = SPEED_MS * 10 / M_PER_DEG_LON
    corners = [(HOME_LAT, HOME_LON),
               (HOME_LAT, HOME_LON + dlon),
               (HOME_LAT + dlat, HOME_LON + dlon),
               (HOME_LAT + dlat, HOME_LON),
               (HOME_LAT, HOME_LON)]
    headings = [90, 0, 270, 180]
    out = []
    for leg in range(4):
        (la0, lo0), (la1, lo1) = corners[leg], corners[leg + 1]
        for i in range(10):
            f = i / 10.0
            out.append(pt(la0 + (la1 - la0) * f, lo0 + (lo1 - lo0) * f,
                          SPEED_KN, headings[leg]))
    out.append(pt(HOME_LAT, HOME_LON, SPEED_KN, 180))   # diem cuoi = diem dau
    return out


def emit(f, name, pts):
    f.write("static const waypoint_t %s[] = {\n" % name)
    for p in pts:
        f.write("  { %d, %d, %d, %d, %d, %d },\n" % p)
    f.write("};\n")
    f.write("#define %s_LEN (sizeof(%s) / sizeof(%s[0]))\n\n" % (name, name, name))


def main():
    if len(sys.argv) != 2:
        sys.exit("usage: python gen_waypoints_tmp.py <output waypoints.h>")
    with open(sys.argv[1], "w", newline="\n") as f:
        f.write("// waypoints.h - SINH TU DONG boi tools/gen_waypoints_tmp.py, KHONG sua tay\n")
        f.write("// Moi phan tu = 1 giay. lat/lon don vi 1e-7 do, do cao don vi 0.1 m,\n")
        f.write("// speed_kn la hai ly (knot), heading la do (0 = Bac, 90 = Dong).\n")
        f.write("#pragma once\n#include <stdint.h>\n#include <stddef.h>\n\n")
        f.write("typedef struct {\n  int32_t  lat_e7, lon_e7;\n  int16_t  alt_msl_dm, alt_agl_dm;\n")
        f.write("  uint8_t  speed_kn;\n  uint16_t heading_deg;\n} waypoint_t;\n\n")
        emit(f, "WP_STATIONARY", stationary())
        emit(f, "WP_STRAIGHT", straight())
        emit(f, "WP_RECTANGLE", rectangle())


main()
