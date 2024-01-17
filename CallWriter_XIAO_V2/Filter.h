#ifndef FILTER_H_
#define FILTER_H_

/*
FIR filter designed with
http://t-filter.appspot.com
*/

#define FILTER_PRECISION    16

//int32_t FilterTaps[FILTER_TAP_NUM] = {
int32_t FilterTaps[] = {

#if 0

/*
sampling frequency: 16000 Hz
fixed point precision: 16 bits

* 0 Hz - 1600 Hz
  gain = 1
  desired ripple = 2 dB
  actual ripple = n/a

* 2000 Hz - 8000 Hz
  gain = 0
  desired attenuation = -30 dB
  actual attenuation = n/a

*/

#define FILTER_TAP_NUM      64

  6,
  315,
  227,
  203,
  97,
  -55,
  -193,
  -249,
  -182,
  -9,
  202,
  345,
  337,
  151,
  -148,
  -428,
  -541,
  -394,
  -11,
  461,
  796,
  790,
  360,
  -382,
  -1142,
  -1528,
  -1200,
  -15,
  1885,
  4085,
  6018,
  7148,
  7148,
  6018,
  4085,
  1885,
  -15,
  -1200,
  -1528,
  -1142,
  -382,
  360,
  790,
  796,
  461,
  -11,
  -394,
  -541,
  -428,
  -148,
  151,
  337,
  345,
  202,
  -9,
  -182,
  -249,
  -193,
  -55,
  97,
  203,
  227,
  315,
  6

#elif 1

/*
sampling frequency: 16000 Hz
fixed point precision: 16 bits

* 0 Hz - 1600 Hz
  gain = 1
  desired ripple = 2 dB
  actual ripple = n/a

* 2000 Hz - 8000 Hz
  gain = 0
  desired attenuation = -30 dB
  actual attenuation = n/a

*/

#define FILTER_TAP_NUM 32

  -914,
  246,
  605,
  1012,
  1248,
  1124,
  582,
  -249,
  -1067,
  -1484,
  -1172,
  5,
  1898,
  4091,
  6016,
  7142,
  7142,
  6016,
  4091,
  1898,
  5,
  -1172,
  -1484,
  -1067,
  -249,
  582,
  1124,
  1248,
  1012,
  605,
  246,
  -914

#endif
};

#endif