// mock_source.cpp - phat lan luot cac diem cua waypoints.h
#include "mock_source.h"
#include "config.h"
#include "waypoints.h"

struct Scenario {
  const char       *name;
  const waypoint_t *wp;
  size_t            len;
  bool              loop;   // true: quay ve diem 0, false: dung o diem cuoi
};

static const Scenario SCN[] = {
  { "stationary", WP_STATIONARY, WP_STATIONARY_LEN, true  },
  { "straight",   WP_STRAIGHT,   WP_STRAIGHT_LEN,   false },
  { "rectangle",  WP_RECTANGLE,  WP_RECTANGLE_LEN,  true  },
};
static const int N_SCN = sizeof(SCN) / sizeof(SCN[0]);

static int    cur      = 0;       // kich ban dang chay
static size_t idx      = 0;       // diem se phat ke tiep
static bool   finished = false;   // da qua diem cuoi (kich ban khong lap)

void mock_select(int s) {
  if (s < 0 || s >= N_SCN) return;
  cur = s; idx = 0; finished = false;
  Serial.printf("# scenario=%d name=%s len=%u\n", s, SCN[s].name, (unsigned) SCN[s].len);
}

int    mock_scenario(void) { return cur; }
size_t mock_index(void)    { return idx; }

void mock_step(UTM_data *d) {
  const Scenario   &s = SCN[cur];
  const waypoint_t &w = s.wp[idx];

  // Doi don vi waypoints.h sang don vi UTM_data
  d->latitude_d  = w.lat_e7 / 1e7;
  d->longitude_d = w.lon_e7 / 1e7;
  d->alt_msl_m   = w.alt_msl_dm / 10.0f;
  d->alt_agl_m   = w.alt_agl_dm / 10.0f;
  d->speed_kn    = finished ? 0 : w.speed_kn;
  d->heading     = w.heading_deg;
  d->satellites  = MOCK_SATS;

  if (idx + 1 < s.len)  idx++;
  else if (s.loop)      idx = 0;
  else                  finished = true;
}
