// check.h - khung kiem tra toi gian cho firmware/test
#pragma once
#include <stdio.h>
#include <math.h>
extern int g_pass, g_fail;
#define CHECK(cond) do { if (cond) { g_pass++; } else { g_fail++; \
    printf("  FAIL %s:%d  %s\n", __FILE__, __LINE__, #cond); } } while (0)
#define CHECK_NEAR(a, b, eps) do { double _a = (a), _b = (b); if (fabs(_a - _b) <= (eps)) { g_pass++; } else { g_fail++; \
    printf("  FAIL %s:%d  %s = %.9g, expected %.9g (+-%g)\n", __FILE__, __LINE__, #a, _a, _b, (double)(eps)); } } while (0)
#define CHECK_STR(a, b) do { const char *_a = (a), *_b = (b); if (_a && !strcmp(_a, _b)) { g_pass++; } else { g_fail++; \
    printf("  FAIL %s:%d  %s = \"%s\", expected \"%s\"\n", __FILE__, __LINE__, #a, _a ? _a : "(null)", _b); } } while (0)
#define RUN(fn) do { printf("[%s]\n", #fn); fn(); } while (0)
