#ifndef FILTER_H_
#define FILTER_H_

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

#define FILTER_TAP_NUM 32
#define FILTER_PRECISION 20

typedef struct {
  int32_t history[FILTER_TAP_NUM];
  unsigned int last_index;
} Filter;

void Filter_init(Filter* f);
void Filter_put(Filter* f, int32_t input);
int32_t Filter_get(Filter* f);

#endif