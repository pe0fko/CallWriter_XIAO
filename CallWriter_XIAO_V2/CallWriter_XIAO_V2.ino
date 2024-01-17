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
//** Description..: 16 Software oscilators generating 16x11 matrix char.
//**
//** History......: V1, https://github.com/pe0fko/CallWriter_XIAO_V2.git
//**                V2, Output filter, http://t-filter.engineerjs.com/
//**
//**************************************************************************

#include <TimerTC3.h>
#include "Font.h"
#include "Filter.h"

#define   DEBUG

const     float     SecondsOneChar            = 0.8;          // The number of seconds for TX of one charracter
const     float     ToneBand                  =  400;         // 250 Bandbreete Hz
const     float     ToneLPFilter              = 1500;         // Lowpass filter cutoff frequency (100Hz below)
const     uint32_t  ToneLines                 = 16;           // Number of tone carriers TX at the same time
const     uint32_t  SampleRate                = 16000;        // 22KHz (max) sample rate
const     float     ToneStep                  = ToneBand / ToneLines;     // Tone steps in Hz
const     float     ToneStart                 = ToneLPFilter - ToneBand;  // Tone start in Hz
const     uint32_t  NextLineCount             = SecondsOneChar * SampleRate / FONT_LENGTH;
const     uint32_t  SineTableLength           = 1 << 9;       // Length of sine table
const     uint32_t  OscFraction               = 1 << 16;      // Oscilator 16bit fraction
const     uint32_t  FilterTableLength         = 32;           // Length of filter table
volatile  uint32_t  SampleDAC                 = 0;            // New sample for the DAC output
static    int32_t   SineTable[SineTableLength];               // Sinus table to use in DDS
static    uint32_t  DDSToneAcc[ToneLines]     = { 0 };      
static    uint32_t  DDSTonePhase[ToneLines]   = { 0 };
static    uint8_t   const *pFontTable         = &FontTable[0];
static    uint32_t  CharLine                  = 0;          // 16 bits of char line
static    uint32_t  CharNextCount             = 0;          // SR count for next char line load.
static    bool      GetNewSample              = true;
static    bool      SampleOverflow            = false;
static    int16_t   FilterHistory[FILTER_TAP_NUM] = { 0 };
static    uint32_t  FilterLastIndex           = 0;

#ifdef DEBUG
#define   printf    Serial.printf
#else
#define   printf(...)
#endif


//=====================================================================
//---- SETUP....  SETUP....  SETUP....  SETUP....  SETUP....    
//=====================================================================
void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);     // PIN 13, PIN_LED[23]
  analogWriteResolution(10);        // Set analog out resolution to max, 10-bits

#ifdef DEBUG
  Serial.begin(115200);
  uint32_t millisStart = millis();
  while(!Serial) 
    if (millis() - millisStart > 10000)
      break;
#endif

  printf("\n-- CallWriter V2.00 ------ PE0FKO --------\n");

  printf("Generate sine table, length=%d\n", SineTableLength);
  for(uint32_t i = 0; i < SineTableLength; i++)
  {
    float s = sin( (2.0 * M_PI * i) / SineTableLength );
//    SineTable[i] = (int32_t)(s * 512.0);         // [.10] -512.0 ... 512.0
    SineTable[i] = (int32_t)(s * 2000.0);         // [.12] -2000.0 ... 2000.0
  }

  printf("Generate tone tables, length=%d\n", ToneLines);
  for(uint32_t i = 0; i < ToneLines; i++)
  {
    // Start with shifted phase signals, more flat power pattern.
    DDSToneAcc[i]  = random(0, OscFraction * SineTableLength);  // [.25]

    // Miror the text, or not
    uint32_t I = ToneLines - 1 - i;
//    uint32_t I = i;

    DDSTonePhase[i] = (uint32_t)(        // [7.25] [7.16+9]
       (uint64_t)                   // Need >37 bits
        (ToneStart + I * ToneStep)  // 12 bits (< 4096)
       * OscFraction                // 16 bits
       * SineTableLength            // 9 bits
       / SampleRate
       );
  }

  printf("Sizeof int      : %d\n", sizeof(int));
  printf("Sizeof float    : %d\n", sizeof(float));
  printf("Sizeof double   : %d\n", sizeof(double));

  printf("SampleRate      : %d\n", SampleRate);
  printf("ToneStart       : %.3f\n", ToneStart);
  printf("ToneStep        : %.3f\n", ToneStep);
  printf("ToneLines       : %d\n", ToneLines);
  printf("SineTableLength : %d\n", SineTableLength);
  printf("OscFraction     : %d\n", OscFraction);

  for(uint32_t i = 0; i < ToneLines; ++i)
  {
    float tone = ToneStart + i * ToneStep;
    printf( "Tone  %2d = %3.2fHz, +%2.4f (0x%08x) Incr, %.4f samp/tone.\n"
            , i, tone
            , (float)DDSTonePhase[i] / OscFraction 
            , DDSTonePhase[i]
            , (float)SampleRate / tone
          );
  }

  printf("\n-- Starting the loop --------\n");

  TimerTc3.initialize(1000000UL / SampleRate);
  TimerTc3.attachInterrupt(timerIsr);
}


//=====================================================================
//---- LOOP....  LOOP....  LOOP....  LOOP....  LOOP....  
//=====================================================================
void loop() 
{
  if (GetNewSample)
  {
    int Signal = 0;

    for(uint32_t i = 0; i < ToneLines; i++)
    {
      DDSToneAcc[i] += DDSTonePhase[i];          // Calc the next oscilator value
      if (CharLine & (1u << i))         // Check if dot is needed.
        Signal += SineTable[ (DDSToneAcc[i] / OscFraction) % SineTableLength ]; // [.14]
    }

    Signal /= 1 << 8;                   // [4.14] => [.10]
    Signal = FilterLP(Signal);          // Do the filter convolution

    if (Signal < -512) Signal = -512;   // Only 10 bits DAC in SAMD21
    if (Signal >  511) Signal =  511;   // Range -512 ... 511

    SampleDAC = (Signal+512) & 0x3FF;   // Uplist signal and (hard) set to 10 bits
    GetNewSample = false;

    if (CharNextCount++ == NextLineCount) 
    {
      CharNextCount = 0;
      fontGetNextLine();
    }
  }

  static uint32_t millisStart = 0;
  if (millis() - millisStart > 200)
  {
    millisStart = millis();

    if (SampleOverflow)
    {
      printf("Error: Sample-rate overflow!\n");
      digitalWrite(LED_BUILTIN, HIGH);   // turn the samplerate overflow LED OFF
      SampleOverflow = false;
    }
  }
}

void timerIsr()
{
  analogWrite(A0, SampleDAC);           // Output the analog signal

  if (GetNewSample && !SampleOverflow)  // Check overrun
  {
    SampleOverflow = true;              // Remember overrun
    digitalWrite(LED_BUILTIN, LOW);     // turn the LED on, XIAO=LOW
  }
  GetNewSample = true;
}

void
fontGetNextLine()
{
  static int indx_char=0;

  if (pFontTable == &FontTable[sizeof FontTable])
  {
    pFontTable = &FontTable[0];
    indx_char = 0;
  }

  if (indx_char++ < FONT_LENGTH)
  {
    CharLine = *pFontTable++ | *pFontTable++ << 8;
  }
  else
  {
    CharLine = 0;           // Blank line
    if (indx_char == FONT_LENGTH+2)
      indx_char = 0;
  }

//  CharLine <<= 1;           // Shift char one bit
//  CharLine ^= 0xFFFF;       // Inverse
//  CharLine |= 0x8000;     // Underline the text
}


#if FILTER_TAP_NUM & 0x01

void Filter_put(Filter* f, int input) {
  f->history[f->last_index++] = input;
  if(f->last_index == FILTER_TAP_NUM)
    f->last_index = 0;
}

int Filter_get(Filter* f) {
  long long acc = 0;
  int index = f->last_index, i;
  for(i = 0; i < FILTER_TAP_NUM; ++i) {
    index = index != 0 ? index-1 : FILTER_TAP_NUM-1;
    acc += (long long)f->history[index] * filter_taps[i];
  };
  return acc >> 16;
}

#else

int FilterLP(int input)                         // Input 10bits signed
{
//  int32_t FilterACC = 0;                      // acc => 5b (0..32) + 10b sample + 16b taps
  int64_t FilterACC = 0;                      // acc => 6b (0..64) + 10b sample + 16b taps

  FilterHistory[(FilterLastIndex++) & (FILTER_TAP_NUM-1)] = input;    // [.10]

  int index = FilterLastIndex;
  for(int i = 0; i < FILTER_TAP_NUM; ++i) 
  {
//    FilterACC += (int32_t)FilterHistory[(index--) & 31] * FilterTaps[i];
    FilterACC += (int32_t)FilterHistory[(index--) & (FILTER_TAP_NUM-1)] * FilterTaps[i];
  };

  return FilterACC >> FILTER_PRECISION;
}

/*

    // Filter the signal on LP
    int32_t FilterACC = 0;                      // acc => 5b (0..32) + 10b sample + 16b taps
    int index = FilterLastIndex;
    for(int i = 0; i < FILTER_TAP_NUM; ++i) 
    {
      int32_t his = FilterHistory[(index--) % FILTER_TAP_NUM];
      FilterACC += his * FilterTaps[i];         // [10.0] * [0.14]
    };
    Signal = FilterACC >> FILTER_PRECISION;
*/
#endif
