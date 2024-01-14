#include <Arduino.h>
#include "Filter.h"

static int32_t filter_taps[FILTER_TAP_NUM] = {
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
};

void Filter_init(Filter* f) {
  int i;
  for(i = 0; i < FILTER_TAP_NUM; ++i)
    f->history[i] = 0;
  f->last_index = 0;
}

void Filter_put(Filter* f, int32_t input) {
  f->history[(f->last_index++) & 31] = input;
}

int32_t Filter_get(Filter* f) {
  int32_t acc = 0;
  int index = f->last_index, i;
  for(i = 0; i < FILTER_TAP_NUM; ++i) {
    acc += f->history[(index--) & 31] * filter_taps[i];
  };
  return acc >> FILTER_PRECISION;
}