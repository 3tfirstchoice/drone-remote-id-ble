// mock_source.cpp - doc waypoints.h, moi lan goi mock_step() phat 1 diem (1 giay)
#include "mock_source.h"
#include "config.h"
#include "waypoints.h"

struct Scenario { const char *name; const waypoint_t *wp; size_t len; bool loop; };

static const Scenario SCN[] = {
  { "stationary", WP_STATIONARY, WP_STATIONARY_LEN, true  },
  { "straight",   WP_STRAIGHT,   WP_STRAIGHT_LEN,   false },
  { "rectangle",  WP_RECTANGLE,  WP_RECTANGLE_LEN,  true  },
};
static const int N_SCN = sizeof(SCN) / sizeof(SCN[0]);

static int    cur      = 0;
static size_t idx      = 0;
static bool   finished = false;
static char   dop_str[] = "1.0";

void mock_select(int s) {
  if (s < 0 || s >= N_SCN) return;
  cur = s; idx = 0; finished = false;
  Serial.printf("# scenario=%d name=%s len=%u\n", s, SCN[s].name, (unsigned) SCN[s].len);
}

int         mock_scenario(void) { return cur; }
size_t      mock_index(void)    { return idx; }
const char *mock_name(void)     { return SCN[cur].name; }

void mock_step(UTM_data *d) {
  const Scenario   &s = SCN[cur];
  const waypoint_t &w = s.wp[idx];

  d->latitude_d  = w.lat_e7 / 1e7;
  d->longitude_d = w.lon_e7 / 1e7;
  d->alt_msl_m   = w.alt_msl_dm / 10.0f;
  d->alt_agl_m   = w.alt_agl_dm / 10.0f;
  d->speed_kn    = finished ? 0 : w.speed_kn;   // het kich ban thang: dung yen o diem cuoi
  d->heading     = w.heading_deg;
  d->satellites  = MOCK_SATS;
  d->hdop_s = d->vdop_s = dop_str;

  if (idx + 1 < s.len)  idx++;
  else if (s.loop)      idx = 0;
  else                  finished = true;
}
