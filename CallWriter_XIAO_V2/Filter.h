#ifndef FILTER_H_
#define FILTER_H_

/*
FIR filter designed with
http://t-filter.appspot.com
*/

#define FILTER_PRECISION    16

//int32_t FilterTaps[FILTER_TAP_NUM] = {
int32_t FilterTaps[] = {

#if 1
/* OK
sampling frequency: 16000 Hz
fixed point precision: 16 bits

* 0 Hz - 500 Hz
  gain = 0
  desired attenuation = -40 dB
  actual attenuation = n/a

* 900 Hz - 1600 Hz
  gain = 1
  desired ripple = 4 dB
  actual ripple = n/a

* 2000 Hz - 8000 Hz
  gain = 0
  desired attenuation = -40 dB
  actual attenuation = n/a

*/

#define FILTER_TAP_NUM 55   // 5,78 bits

  -158,
  -294,
  -178,
  -211,
  -46,
  57,
  177,
  180,
  114,
  -9,
  -78,
  -12,
  245,
  644,
  1048,
  1249,
  1059,
  390,
  -676,
  -1880,
  -2846,
  -3206,
  -2732,
  -1439,
  390,
  2274,
  3682,
  4203,  // 12,037 bits
  3682,
  2274,
  390,
  -1439,
  -2732,
  -3206,
  -2846,
  -1880,
  -676,
  390,
  1059,
  1249,
  1048,
  644,
  245,
  -12,
  -78,
  -9,
  114,
  180,
  177,
  57,
  -46,
  -211,
  -178,
  -294,
  -158

#elif 1
/* OK

FIR filter designed with
http://t-filter.appspot.com

sampling frequency: 16000 Hz

fixed point precision: 16 bits

* 0 Hz - 600 Hz
  gain = 0
  desired attenuation = -40 dB
  actual attenuation = n/a

* 1000 Hz - 2000 Hz
  gain = 1
  desired ripple = 5 dB
  actual ripple = n/a

* 2400 Hz - 8000 Hz
  gain = 0
  desired attenuation = -40 dB
  actual attenuation = n/a

*/

#define FILTER_TAP_NUM 51

  188,
  9,
  -142,
  -332,
  -449,
  -396,
  -165,
  142,
  360,
  371,
  205,
  46,
  124,
  539,
  1126,
  1481,
  1157,
  -35,
  -1823,
  -3491,
  -4177,
  -3312,
  -977,
  2046,
  4573,
  5553,
  4573,
  2046,
  -977,
  -3312,
  -4177,
  -3491,
  -1823,
  -35,
  1157,
  1481,
  1126,
  539,
  124,
  46,
  205,
  371,
  360,
  142,
  -165,
  -396,
  -449,
  -332,
  -142,
  9,
  188

#elif 1
/* OK
sampling frequency: 16000 Hz
fixed point precision: 16 bits

* 0 Hz - 1500 Hz
  gain = 1
  desired ripple = 5 dB
  actual ripple = 3.36

* 2000 Hz - 8000 Hz
  gain = 0
  desired attenuation = -50 dB
  actual attenuation = -51.77

*/

#define FILTER_TAP_NUM 45

  -135,
  -287,
  -499,
  -713,
  -848,
  -838,
  -630,
  -250,
  220,
  627,
  820,
  683,
  217,
  -459,
  -1099,
  -1418,
  -1160,
  -220,
  1330,
  3207,
  5009,
  6305,
  6779,
  6305,
  5009,
  3207,
  1330,
  -220,
  -1160,
  -1418,
  -1099,
  -459,
  217,
  683,
  820,
  627,
  220,
  -250,
  -630,
  -838,
  -848,
  -713,
  -499,
  -287,
  -135

#elif 1
/* OK
sampling frequency: 16000 Hz
fixed point precision: 16 bits

* 0 Hz - 1500 Hz
  gain = 1
  desired ripple = 5 dB
  actual ripple = 4.01 dB

* 1800 Hz - 8000 Hz
  gain = 0
  desired attenuation = -40 dB
  actual attenuation = -40.33 dB

*/

#define FILTER_TAP_NUM 55

  343,
  486,
  698,
  843,
  858,
  708,
  402,
  5,
  -380,
  -638,
  -679,
  -480,
  -95,
  347,
  681,
  759,
  506,
  -37,
  -711,
  -1273,
  -1462,
  -1082,
  -71,
  1459,
  3239,
  4903,
  6085,
  6512,
  6085,
  4903,
  3239,
  1459,
  -71,
  -1082,
  -1462,
  -1273,
  -711,
  -37,
  506,
  759,
  681,
  347,
  -95,
  -480,
  -679,
  -638,
  -380,
  5,
  402,
  708,
  858,
  843,
  698,
  486,
  343

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