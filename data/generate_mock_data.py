#!/usr/bin/env python3
# generate_mock_data.py - sinh du lieu gia lap 3 kich ban bay: 3 file CSV va waypoints.h
# Cach dung (tu thu muc goc repo): python data/generate_mock_data.py

import csv
import math
import re
import struct
import sys
import time
from datetime import datetime, timezone
from pathlib import Path

# --- Duong dan ---
DATA_DIR = Path(__file__).resolve().parent
REPO_DIR = DATA_DIR.parent
FW_HEADER = REPO_DIR / "firmware" / "rid_tx" / "waypoints.h"
FW_CONFIG = REPO_DIR / "firmware" / "rid_tx" / "config.h"

# --- Tham so mo phong (phai trung firmware/rid_tx/config.h) ---
HOME_LAT = 10.7725        # do, WGS84
HOME_LON = 106.6578       # do, WGS84
ALT_MSL_HOME_M = 10.0     # do cao MSL cua home (m)
ALT_AGL_M = 50.0          # do cao bay so voi home (m)
SPEED_KN = 10             # toc do bay (kn)
KN_TO_MS = 0.514444       # m/s tren 1 kn
SPEED_MS = SPEED_KN * KN_TO_MS
MOCK_SATS = 10            # so ve tinh gia lap
M_PER_DEG_LAT = 111320.0
M_PER_DEG_LON = 111320.0 * math.cos(math.radians(HOME_LAT))

# Ma kich ban (cot scn)
SCN_STATIONARY, SCN_STRAIGHT, SCN_RECTANGLE = 0, 1, 2

# Buoc ma hoa cua OpenDroneID
LATLON_STEP_DEG = 1e-7    # do
ALT_STEP_M = 0.5          # m
SPEED_STEP_MS = 0.25      # m/s
LATLON_LIMIT_DEG = 1e-6   # nguong sai so toa do T01 (do)

CSV_HEADER = ["utc", "scn", "idx", "lat", "lon", "alt_msl", "alt_agl",
              "speed_kn", "heading", "sats"]


# ---------------------------------------------------------------- kich ban
def raw_pt(lat, lon, speed, heading):
    """Diem goc truoc khi lam tron."""
    return {"lat": lat, "lon": lon,
            "alt_msl": ALT_MSL_HOME_M + ALT_AGL_M, "alt_agl": ALT_AGL_M,
            "speed_kn": speed, "heading": heading}


def stationary():          # 60 diem dung yen tai home
    return [raw_pt(HOME_LAT, HOME_LON, 0, 0) for _ in range(60)]


def straight():            # 30 diem bay huong Bac
    d = SPEED_MS / M_PER_DEG_LAT
    return [raw_pt(HOME_LAT + i * d, HOME_LON, SPEED_KN, 0) for i in range(30)]


def rectangle():           # 40 diem: 4 canh Dong, Bac, Tay, Nam
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
            out.append(raw_pt(la0 + (la1 - la0) * f, lo0 + (lo1 - lo0) * f,
                              SPEED_KN, headings[leg]))
    return out


def to_waypoint(p):
    """Lam tron ve dang luu trong waypoints.h."""
    return (round(p["lat"] * 1e7), round(p["lon"] * 1e7),
            round(p["alt_msl"] * 10), round(p["alt_agl"] * 10),
            p["speed_kn"], p["heading"])


# ---------------------------------------------------------------- kiem tra
def check_config():
    """Kiem tra hang so cua script trung config.h."""
    if not FW_CONFIG.exists():
        sys.exit("Khong tim thay %s" % FW_CONFIG)
    text = FW_CONFIG.read_text(encoding="utf-8")
    expect = {"HOME_LAT": HOME_LAT, "HOME_LON": HOME_LON,
              "HOME_ALT_MSL": ALT_MSL_HOME_M, "MOCK_SATS": MOCK_SATS}
    for name, value in expect.items():
        m = re.search(r"#define\s+%s\s+([-0-9.]+)" % name, text)
        if not m or float(m.group(1)) != value:
            sys.exit("%s trong config.h khac voi script (%s)" % (name, value))


def check_waypoints(name, wps, expected_len):
    """Kiem tra so diem va mien gia tri cua waypoint_t."""
    if len(wps) != expected_len:
        sys.exit("%s: co %d diem, can %d" % (name, len(wps), expected_len))
    for i, (lat, lon, msl, agl, spd, hdg) in enumerate(wps):
        where = "%s[%d]" % (name, i)
        if not -90e7 <= lat <= 90e7 or not -180e7 <= lon <= 180e7:
            sys.exit("%s: lat/lon ngoai mien WGS84" % where)
        if not -10000 <= msl <= 32767 or not -10000 <= agl <= 32767:
            sys.exit("%s: do cao ngoai mien -1000.0..3276.7 m" % where)
        if msl % 5 or agl % 5:
            sys.exit("%s: do cao khong chia het buoc 0.5 m cua OpenDroneID" % where)
        if not 0 <= spd <= 255:
            sys.exit("%s: toc do vuot kieu uint8_t (0..255 kn)" % where)
        if not 0 <= hdg < 360:
            sys.exit("%s: huong ngoai mien 0..359 do" % where)
    if MOCK_SATS < 7:
        sys.exit("MOCK_SATS < 7: thu vien se khong phat toa do (SATS_LEVEL_2)")


def check_closed(rect):
    """Kiem tra kich ban hinh chu nhat khep kin."""
    last, first = rect[-1], rect[0]
    step_back = last[0] - first[0]
    step_leg = rect[38][0] - rect[39][0]
    if first[1] != last[1] or abs(step_back - step_leg) > 1:
        sys.exit("rectangle: diem cuoi khong noi ve diem xuat phat")


# ---------------------------------------- mo phong ma hoa cua firmware
def f32(x):
    """Lam tron ve float 32 bit."""
    return struct.unpack("<f", struct.pack("<f", x))[0]


def odid_latlon(lat_e7):
    # Ma hoa roi giai ma lat/lon nhu firmware
    enc = int((lat_e7 / 1e7) * 10000000)
    return enc / 1e7


def odid_alt(alt_dm):
    # Ma hoa roi giai ma do cao nhu firmware
    alt = f32(f32(float(alt_dm)) / f32(10.0))
    enc = int(f32(f32(alt + 1000.0) / 0.5))
    return f32(enc * 0.5) - 1000.0


def odid_speed(speed_kn):
    # Ma hoa roi giai ma toc do ngang nhu firmware
    v = f32(0.514444 * float(speed_kn))
    enc = int(f32(v / 0.25))
    return enc * 0.25


def quantization(scenarios):
    """In va kiem tra sai so luong tu hoa lon nhat sau ma hoa OpenDroneID."""
    worst = {"lat/lon (do)": 0.0, "do cao (m)": 0.0, "toc do (m/s)": 0.0}
    for _, raws, wps in scenarios:
        for p, w in zip(raws, wps):
            worst["lat/lon (do)"] = max(worst["lat/lon (do)"],
                                        abs(odid_latlon(w[0]) - p["lat"]),
                                        abs(odid_latlon(w[1]) - p["lon"]))
            worst["do cao (m)"] = max(worst["do cao (m)"],
                                      abs(odid_alt(w[2]) - p["alt_msl"]),
                                      abs(odid_alt(w[3]) - p["alt_agl"]))
            worst["toc do (m/s)"] = max(worst["toc do (m/s)"],
                                        abs(odid_speed(w[4]) - p["speed_kn"] * KN_TO_MS))
    step = {"lat/lon (do)": LATLON_STEP_DEG, "do cao (m)": ALT_STEP_M,
            "toc do (m/s)": SPEED_STEP_MS}
    limit = {"lat/lon (do)": LATLON_LIMIT_DEG, "do cao (m)": ALT_STEP_M,
             "toc do (m/s)": SPEED_STEP_MS}
    print("Kiem tra luong tu hoa (sai so lon nhat | buoc ma hoa | nguong dat):")
    for k in worst:
        ok = worst[k] < limit[k]
        print("  %-14s %.4g | %g | < %g  %s" % (k, worst[k], step[k], limit[k],
                                                "DAT" if ok else "KHONG DAT"))
        if not ok:
            sys.exit("Sai so luong tu hoa vuot nguong: %s" % k)
    lost = sum(1 for _, _, wps in scenarios for w in wps for v in (w[0], w[1])
               if int((v / 1e7) * 10000000) != v)
    print("  %d gia tri lat/lon bi firmware cat mat 1e-7 do khi ma hoa (loi lam tron so thuc)" % lost)


# ---------------------------------------------------------------- xuat file
def iso(t):
    return datetime.fromtimestamp(t, timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")


def write_csv(path, scn, wps, epoch, closing_row):
    rows = list(enumerate(wps))
    if closing_row:                       # them dong diem 0 o cuoi
        rows.append((0, wps[0]))
    with open(path, "w", newline="", encoding="ascii") as f:
        w = csv.writer(f, lineterminator="\n")
        w.writerow(CSV_HEADER)
        for t, (idx, (lat, lon, msl, agl, spd, hdg)) in enumerate(rows):
            w.writerow([iso(epoch + t), scn, idx, "%.7f" % (lat / 1e7), "%.7f" % (lon / 1e7),
                        "%.1f" % (msl / 10), "%.1f" % (agl / 10), spd, hdg, MOCK_SATS])
    return len(rows)


def header_text(scenarios, epoch):
    out = ["// waypoints.h - SINH TU DONG boi data/generate_mock_data.py, KHONG sua tay",
           "// Moi phan tu = 1 giay. lat/lon don vi 1e-7 do, do cao don vi 0.1 m,",
           "// speed_kn la hai ly (knot), heading la do (0 = Bac, 90 = Dong).",
           "#pragma once",
           "#include <stdint.h>",
           "#include <stddef.h>",
           "",
           "// Gio bat dau gia lap (Unix, UTC)",
           "#define SCENARIO_EPOCH   %dUL      // %s" % (epoch, iso(epoch).replace("T", " ").replace("Z", " UTC")),
           "",
           "typedef struct {",
           "  int32_t  lat_e7, lon_e7;",
           "  int16_t  alt_msl_dm, alt_agl_dm;",
           "  uint8_t  speed_kn;",
           "  uint16_t heading_deg;",
           "} waypoint_t;",
           ""]
    for name, _, wps in scenarios:
        out.append("static const waypoint_t %s[] = {" % name)
        out += ["  { %d, %d, %d, %d, %d, %d }," % w for w in wps]
        out.append("};")
        out.append("#define %s_LEN (sizeof(%s) / sizeof(%s[0]))" % (name, name, name))
        out.append("")
    return "\n".join(out)


def main():
    check_config()
    epoch = int(time.time())

    raws = {"WP_STATIONARY": stationary(), "WP_STRAIGHT": straight(), "WP_RECTANGLE": rectangle()}
    scenarios = [(name, r, [to_waypoint(p) for p in r]) for name, r in raws.items()]
    lens = {"WP_STATIONARY": 60, "WP_STRAIGHT": 30, "WP_RECTANGLE": 40}
    for name, _, wps in scenarios:
        check_waypoints(name, wps, lens[name])
    check_closed(scenarios[2][2])
    quantization(scenarios)

    n0 = write_csv(DATA_DIR / "stationary.csv", SCN_STATIONARY, scenarios[0][2], epoch, False)
    n1 = write_csv(DATA_DIR / "straight_line.csv", SCN_STRAIGHT, scenarios[1][2], epoch, False)
    n2 = write_csv(DATA_DIR / "rectangle.csv", SCN_RECTANGLE, scenarios[2][2], epoch, True)

    text = header_text(scenarios, epoch)
    for path in (DATA_DIR / "waypoints.h", FW_HEADER):
        with open(path, "w", newline="\n", encoding="ascii") as f:
            f.write(text)

    print("Moc thoi gian: %s (SCENARIO_EPOCH = %d)" % (iso(epoch), epoch))
    print("CSV: stationary=%d dong, straight_line=%d dong, rectangle=%d dong" % (n0, n1, n2))
    print("Header: data/waypoints.h, firmware/rid_tx/waypoints.h")
    print("Nho nap lai firmware de gio trong log khop voi CSV.")


if __name__ == "__main__":
    main()
