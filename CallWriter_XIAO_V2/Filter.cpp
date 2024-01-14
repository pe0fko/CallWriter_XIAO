#include <Arduino.h>
#include "Filter.h"

static int32_t filter_taps[FILTER_TAP_NUM] = {
#if 1
/*

FIR filter designed with
 http://t-filter.appspot.com

sampling frequency: 22000 Hz

fixed point precision: 16 bits

* 0 Hz - 1800 Hz
  gain = 1
  desired ripple = 3 dB
  actual ripple = 2.29 dB

* 2300 Hz - 11000 Hz
  gain = 0
  desired attenuation = -20 dB
  actual attenuation = -20.28 dB

*/
  -357,
  1787,
  994,
  745,
  397,
  -112,
  -677,
  -1120,
  -1255,
  -941,
  -126,
  1125,
  2629,
  4124,
  5326,
  5996,   // Max! is 12,55 bits
  5996,
  5326,
  4124,
  2629,
  1125,
  -126,
  -941,
  -1255,
  -1120,
  -677,
  -112,
  397,
  745,
  994,
  1787,
  -357
#else
/*
FIR filter designed with
 http://t-filter.appspot.com

sampling frequency: 22000 Hz

fixed point precision: 20 bits

* 0 Hz - 1800 Hz
  gain = 1
  desired ripple = 3 dB
  actual ripple = 2.96 dB

* 2200 Hz - 11000 Hz
  gain = 0
  desired attenuation = -20 dB
  actual attenuation = -18.09 dB
*/
  -1161,
  39334,
  14514,
  11038,
  4681,
  -4004,
  -12751,
  -18924,
  -19746,
  -13519,
  177,
  20046,
  43215,
  65852,
  83831,
  93789,
  93789,
  83831,
  65852,
  43215,
  20046,
  177,
  -13519,
  -19746,
  -18924,
  -12751,
  -4004,
  4681,
  11038,
  14514,
  39334,
  -1161
#endif
};

void Filter_init(Filter* f) {
  int i;
  for(i = 0; i < FILTER_TAP_NUM; ++i)
    f->history[i] = 0;
  f->last_index = 0;
}

void Filter_put(Filter* f, int32_t input) {
  f->history[(f->last_index++) & (FILTER_TAP_NUM-1)] = input;
}

int32_t Filter_get(Filter* f) {
  int32_t acc = 0;   // acc => 5b (0..32) + 10b sample + 14b taps
  int index = f->last_index, i;
  for(i = 0; i < FILTER_TAP_NUM; ++i) {
    acc += (int32_t)(f->history[(index--) & (FILTER_TAP_NUM-1)]) * filter_taps[i];  // [10.0] * [0.14]
  };
  return acc >> FILTER_PRECISION;
}