// gps_passthrough.ino - kiem tra module GPS M5Stack (AT6668) noi vao board OhStem YOLO UNO.
//
// Muc dich: truoc khi viet gps_source, xac nhan 3 dieu:
//   1) day noi dung chua (RX/TX co bi dao khong)
//   2) baud va dinh dang (8N1 / 7N1) dung chua
//   3) module da "fix" (dinh vi duoc) chua
//
// Cach hoat dong:
//   Giai doan 1 (~25 giay): thu lan luot cac to hop {chan, baud, dinh dang}. Moi to hop nghe 2.5 giay,
//                           dem so cau NMEA co checksum DUNG. To hop nao nhieu nhat duoc chon.
//   Giai doan 2:            giu to hop da chon, in cau NMEA tho (5 giay dau) va 1 dong tom tat moi 2 giay.
//
// Serial Monitor: 115200 baud. Go 'v' de bat/tat in cau tho, go 'r' de do lai tu dau.
// Chan GPS lay tu BOARD_PINS.md: D3 = GPIO6 (ESP32 nhan), D4 = GPIO7 (ESP32 gui) - trung config.h.

#include <Arduino.h>
#include <stdlib.h>
#include <string.h>

#define GPS_RX_PIN   6      // D3: ESP32 nhan <- TX cua module GPS
#define GPS_TX_PIN   7      // D4: ESP32 gui  -> RX cua module GPS
#define PROBE_MS     2500   // thoi gian nghe moi to hop
#define LINE_MAX     120    // do dai toi da 1 cau NMEA

struct Combo { uint32_t baud; uint32_t cfg; const char *name; };

// Thu to hop co kha nang cao nhat truoc
static const Combo COMBOS[] = {
  { 115200, SERIAL_8N1, "115200 8N1" },   // mac dinh cua Unit GPS v1.1 (AT6668)
  { 115200, SERIAL_7N1, "115200 7N1" },   // mot so khach bao 7N1
  {   9600, SERIAL_8N1, "9600 8N1"   },   // mac dinh cua doi truoc (doi truoc: AT6558 / NEO-6M)
  {  38400, SERIAL_8N1, "38400 8N1"  },
  {   9600, SERIAL_7N1, "9600 7N1"   },
};
static const int N_COMBO = sizeof(COMBOS) / sizeof(COMBOS[0]);

// ---------------------------------------------------------------- NMEA co ban
// Cau hop le: $<noi dung>*HH, HH = XOR cua moi ky tu giua '$' va '*' (2 chu so hex).
static int hexval(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  return -1;
}

static bool nmea_valid(const char *s) {
  if (s[0] != '$') return false;
  uint8_t x = 0;
  const char *p = s + 1;
  while (*p && *p != '*') x ^= (uint8_t) *p++;
  if (*p != '*') return false;
  int h1 = hexval(p[1]), h2 = (h1 < 0) ? -1 : hexval(p[2]);
  if (h1 < 0 || h2 < 0) return false;
  return x == (uint8_t) (h1 * 16 + h2);
}

// Lay truong thu n (0 = "$GNGGA") cua cau, chep vao out. Tra false neu khong co.
static bool nmea_field(const char *s, int n, char *out, size_t outsz) {
  const char *p = s;
  for (int i = 0; i < n; i++) {
    p = strchr(p, ',');
    if (!p) return false;
    p++;
  }
  size_t k = 0;
  while (*p && *p != ',' && *p != '*' && k + 1 < outsz) out[k++] = *p++;
  out[k] = 0;
  return true;
}

// "ddmm.mmmm" (+ N/S/E/W) -> do thap phan. deg_digits = 2 (vi do) hoac 3 (kinh do).
static double nmea_deg(const char *f, char hemi, int deg_digits) {
  if (strlen(f) < (size_t) deg_digits + 1) return 0.0;
  char d[4] = {0};
  memcpy(d, f, deg_digits);
  double v = atof(d) + atof(f + deg_digits) / 60.0;
  return (hemi == 'S' || hemi == 'W') ? -v : v;
}

// ---------------------------------------------------------------- trang thai
static int  chosen = -1;            // to hop duoc chon
static bool swapped = false;        // true neu phai dao chan RX/TX
static bool verbose = true;
static uint32_t lock_ms = 0, last_sum = 0;

static char    line[LINE_MAX];
static size_t  len = 0;
// Ban tin GGA moi nhat
static int     g_fix = 0, g_sats = 0;
static char    g_hdop[12] = "", g_alt[12] = "";
static double  g_lat = 0, g_lon = 0;
static uint32_t n_valid = 0, n_bad = 0;
// GSV = ve tinh "nhin thay" theo tung he (GP=GPS, BD/GB=BeiDou, GL=GLONASS, GA=Galileo)
static const int MAX_TALKER = 6;
static char     gsv_talker[MAX_TALKER][3];
static int      gsv_inview[MAX_TALKER];

static void gps_open(const Combo &c, bool swap) {
  Serial1.end();
  int rx = swap ? GPS_TX_PIN : GPS_RX_PIN;
  int tx = swap ? GPS_RX_PIN : GPS_TX_PIN;
  Serial1.begin(c.baud, c.cfg, rx, tx);
}

// Nghe ms mili giay, dem byte va cau hop le/khong hop le.
static void probe(const Combo &c, bool swap, uint32_t ms, uint32_t &bytes, uint32_t &ok, uint32_t &bad) {
  gps_open(c, swap);
  bytes = ok = bad = 0;
  size_t n = 0;
  char buf[LINE_MAX];
  uint32_t t0 = millis();
  while (millis() - t0 < ms) {
    while (Serial1.available()) {
      char ch = (char) Serial1.read();
      bytes++;
      if (ch == '\n' || ch == '\r') {
        if (n > 0) {
          buf[n] = 0;
          if (buf[0] == '$') { if (nmea_valid(buf)) ok++; else bad++; }
          n = 0;
        }
      } else if (n + 1 < sizeof(buf)) {
        buf[n++] = ch;
      } else {
        n = 0;                       // cau qua dai: bo
      }
    }
  }
}

static void run_detection(void) {
  Serial.println("# ---- Giai doan 1: do cau hinh ----");
  Serial.printf("# chan: RX=GPIO%d (D3), TX=GPIO%d (D4)\n", GPS_RX_PIN, GPS_TX_PIN);
  uint32_t best_ok = 0, best_bytes_any = 0;
  int best = -1; bool best_swap = false;

  for (int sw = 0; sw < 2; sw++) {
    for (int i = 0; i < N_COMBO; i++) {
      uint32_t bytes, ok, bad;
      probe(COMBOS[i], sw == 1, PROBE_MS, bytes, ok, bad);
      Serial.printf("# thu %-10s %s: bytes=%-5lu cau_dung=%-3lu cau_sai=%lu\n",
                    COMBOS[i].name, sw ? "(dao RX/TX)" : "(chan chuan) ",
                    (unsigned long) bytes, (unsigned long) ok, (unsigned long) bad);
      if (bytes > best_bytes_any) best_bytes_any = bytes;
      if (ok > best_ok) { best_ok = ok; best = i; best_swap = (sw == 1); }
    }
  }

  if (best >= 0) {
    chosen = best; swapped = best_swap;
    Serial.printf("# => CHON: %s%s (%lu cau dung)\n", COMBOS[best].name,
                  best_swap ? ", PHAI DAO day vang/trang" : "", (unsigned long) best_ok);
    if (best_swap) Serial.println("# Goi y: dao 2 day tin hieu tren board, hoac doi GPS_RX_PIN/GPS_TX_PIN trong config.h.");
    gps_open(COMBOS[best], best_swap);
    lock_ms = millis(); last_sum = lock_ms; len = 0;
    n_valid = n_bad = 0; g_fix = g_sats = 0;
    Serial.println("# ---- Giai doan 2: doc lien tuc ('v' bat/tat cau tho, 'r' do lai) ----");
    return;
  }

  chosen = -1;
  Serial.println("# => KHONG TIM THAY cau NMEA hop le nao.");
  if (best_bytes_any == 0) {
    Serial.println("# Khong nhan duoc byte nao: kiem tra nguon 5V/GND, day tin hieu da cam chua, module co sang den khong.");
  } else {
    Serial.println("# Co byte nhung khong co cau dung: sai baud/dinh dang, hoac day nhieu. Thu cap USB/nguon khac, kiem tra GND chung.");
  }
  Serial.println("# Go 'r' de do lai.");
}

// ---------------------------------------------------------------- xu ly 1 cau
static void handle_sentence(const char *s) {
  if (!nmea_valid(s)) { n_bad++; return; }
  n_valid++;
  if (verbose && millis() - lock_ms < 5000) Serial.println(s);

  // $GNGGA hoac $GPGGA: truong 2 = vi do, 3 = N/S, 4 = kinh do, 5 = E/W,
  //                     6 = chat luong fix, 7 = so ve tinh, 8 = hdop, 9 = do cao MSL
  if (strncmp(s + 3, "GGA", 3) == 0) {
    char f[16];
    if (nmea_field(s, 6, f, sizeof f)) g_fix  = atoi(f);
    if (nmea_field(s, 7, f, sizeof f)) g_sats = atoi(f);
    nmea_field(s, 8, g_hdop, sizeof g_hdop);
    nmea_field(s, 9, g_alt,  sizeof g_alt);
    char lat[16], lon[16], ns[4], ew[4];
    if (nmea_field(s, 2, lat, sizeof lat) && nmea_field(s, 3, ns, sizeof ns) &&
        nmea_field(s, 4, lon, sizeof lon) && nmea_field(s, 5, ew, sizeof ew)) {
      g_lat = nmea_deg(lat, ns[0], 2);
      g_lon = nmea_deg(lon, ew[0], 3);
    }
  }
  // $GxGSV: truong 3 = tong so ve tinh nhin thay cua he x (de biet anten co thay troi hay khong)
  if (strncmp(s + 3, "GSV", 3) == 0) {
    char f[8];
    if (!nmea_field(s, 3, f, sizeof f)) return;
    int slot = -1;
    for (int i = 0; i < MAX_TALKER; i++) {
      if (gsv_talker[i][0] == 0 && slot < 0) slot = i;                    // o trong dau tien
      if (strncmp(gsv_talker[i], s + 1, 2) == 0) { slot = i; break; }     // da co he nay
    }
    if (slot >= 0) { memcpy(gsv_talker[slot], s + 1, 2); gsv_talker[slot][2] = 0; gsv_inview[slot] = atoi(f); }
  }
}

static int sats_in_view(void) {
  int sum = 0;
  for (int i = 0; i < MAX_TALKER; i++) sum += gsv_inview[i];
  return sum;
}

static void print_summary(void) {
  Serial.printf("# cau_dung=%lu cau_sai=%lu | fix=%d ve_tinh=%d nhin_thay=%d hdop=%s do_cao=%s m",
                (unsigned long) n_valid, (unsigned long) n_bad, g_fix, g_sats, sats_in_view(), g_hdop, g_alt);
  if (g_fix > 0) Serial.printf(" | lat=%.6f lon=%.6f", g_lat, g_lon);
  else           Serial.print(" | CHUA FIX (nhin_thay=0: anten khong thay troi -> ra cua so/ngoai troi; >0 ma chua fix: cho them)");
  Serial.println();
}

// ---------------------------------------------------------------- Arduino
void setup() {
  Serial.begin(115200);
  uint32_t t0 = millis();
  while (!Serial && millis() - t0 < 3000) { delay(10); }   // USB native: cho may tinh mo cong COM
  delay(500);
  Serial.println("# gps_passthrough: kiem tra module GPS");
  run_detection();
}

void loop() {
  while (Serial.available()) {
    char c = (char) Serial.read();
    if (c == 'v') { verbose = !verbose; Serial.printf("# in cau tho: %s\n", verbose ? "BAT" : "TAT"); }
    if (c == 'r') { run_detection(); return; }
  }
  if (chosen < 0) return;

  while (Serial1.available()) {
    char ch = (char) Serial1.read();
    if (ch == '\n' || ch == '\r') {
      if (len > 0) { line[len] = 0; if (line[0] == '$') handle_sentence(line); len = 0; }
    } else if (len + 1 < sizeof(line)) {
      line[len++] = ch;
    } else {
      len = 0;
    }
  }

  if (millis() - last_sum >= 2000) {
    last_sum += 2000;
    print_summary();
  }
}
