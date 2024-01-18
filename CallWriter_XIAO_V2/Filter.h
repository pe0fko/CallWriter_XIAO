//************************************************************************
//**
//** Project......: CallWrite, Write some text in the waterfall display.
//**
//** Platform.....: Seeeduino XIAO, SAMD board
//**
//** Licence......: NU GENERAL PUBLIC LICENSE, Version 3, 29 June 2007
//**
//** Programmer...: F.W. Krom, PE0FKO
//**
//** Description..: Filter to smood the DAC output freq from 1kHz 1.4KHz.
//**                Filter design with http://t-filter.appspot.com
//**
//**************************************************************************

#define FILTER_TAP_NUM      81
#define FILTER_PRECISION    16
#define FILTER_SAMPLE_FREQ  20000

/*
sampling frequency: 20000 Hz
fixed point precision: 16 bits

* 0 Hz - 600 Hz
  gain = 0
  desired attenuation = -40 dB
  actual attenuation = -40.34

* 1000 Hz - 1400 Hz
  gain = 1
  desired ripple = 3 dB
  actual ripple = 2.28

* 1800 Hz - 10000 Hz
  gain = 0
  desired attenuation = -40 dB
  actual attenuation = -40.34
*/

int32_t FilterTaps[FILTER_TAP_NUM] = 
{
  163,
  6,
  -6,
  -21,
  -35,
  -41,
  -37,
  -23,
  -4,
  11,
  10,
  -16,
  -71,
  -152,
  -245,
  -327,
  -370,
  -348,
  -242,
  -46,
  223,
  529,
  822,
  1038,
  1120,
  1027,
  742,
  285,
  -290,
  -899,
  -1441,
  -1819,
  -1951,
  -1795,
  -1352,
  -677,
  135,
  961,
  1671,
  2149,
  2317,       // 11,1780423289 bits (plus sign)
  2149,
  1671,
  961,
  135,
  -677,
  -1352,
  -1795,
  -1951,
  -1819,
  -1441,
  -899,
  -290,
  285,
  742,
  1027,
  1120,
  1038,
  822,
  529,
  223,
  -46,
  -242,
  -348,
  -370,
  -327,
  -245,
  -152,
  -71,
  -16,
  10,
  11,
  -4,
  -23,
  -37,
  -41,
  -35,
  -21,
  -6,
  6,
  163
};
