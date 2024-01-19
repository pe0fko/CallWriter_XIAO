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
const     float     ToneStart                 = 1000;         // Tone start in Hz
const     float     ToneBand                  =  400;         // 250 Bandbreete Hz
const     uint32_t  ToneLines                 = 16;           // Number of tone carriers TX at the same time
const     uint32_t  SineTableLength           = 1 << 9;       // Length of sine table
const     uint32_t  DDSFractionBits           = 16;           // Oscilator 16bit fraction

const     float     ToneStep                  = ToneBand / ToneLines; // Tone steps in Hz
const     uint32_t  SamplesOneChar            = SecondsOneChar * FILTER_SAMPLE_FREQ / FONT_LENGTH;

volatile  uint32_t  SampleDAC                 = 0;            // New sample for the DAC output
static    int32_t   SineTable[SineTableLength];               // Sinus table to use in DDS
static    uint32_t  DDSToneAcc[ToneLines]     = { 0 };        // DDS accumulator for the tones
static    uint32_t  DDSTonePhase[ToneLines]   = { 0 };        // DDS phase register to index the sine table
static    uint8_t   const *pFontTable         = FontTable;    // Pointer to the running char/line
static    uint32_t  CharLine                  = 0;            // 16 bits of char/line
static    uint32_t  CharLineNextCount         = 0;            // Samplerate count for next char line load.
static    bool      GetNewSample              = true;         // Flag to get a new/next sample for the DAC
static    int16_t   FilterHistory[FILTER_TAP_NUM] = { 0 };    // The bandpass filter history sample array
static    uint32_t  FilterLastIndex           = 0;            // Index in the FilterHistory array
static    uint32_t  StartTimerOverflow        = 0;            // Interrupt DAC overflow LED timer

#ifdef DEBUG
#define   printf(...)    { if (Serial) Serial.printf(__VA_ARGS__); }
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
  StartTimerOverflow = millis();
  while(!Serial) 
    if (millis() - StartTimerOverflow > 10000)
      break;
  StartTimerOverflow = 0;
#endif

  printf("\n-- CallWriter V2.00 ------ PE0FKO --------\n");

  printf("Generate sine table, length=%d\n", SineTableLength);
  for(uint32_t i = 0; i < SineTableLength; i++)
  {
    float s = sin( (2.0 * M_PI * i) / SineTableLength );
    SineTable[i] = (int32_t)(s * 2000.0);         // [.12] -2000.0 ... 2000.0
  }

  printf("Generate tone tables, length=%d\n", ToneLines);
  for(uint32_t i = 0; i < ToneLines; i++)
  {
    // Start with all shifted phase signals for more flat power spectrum.
    DDSToneAcc[i]  = random(0, (SineTableLength << DDSFractionBits)-1);  // [.25]

    // Miror the text, or not
    uint32_t I = ToneLines - 1 - i;
//    uint32_t I = i;

    DDSTonePhase[i] = (uint32_t)(       // [7.25] [7.16+9]
       (uint64_t)                       // Need >37 bits
        (ToneStart + I * ToneStep)      // 12 bits (< 4096)
       * (1 << DDSFractionBits)         // 16 bits
       * SineTableLength                // 9 bits
       / FILTER_SAMPLE_FREQ
       );
  }

  printf("Sizeof int         : %d\n", sizeof(int));
  printf("Sizeof float       : %d\n", sizeof(float));
  printf("Sizeof double      : %d\n", sizeof(double));
  printf("FILTER_SAMPLE_FREQ : %d\n", FILTER_SAMPLE_FREQ);
  printf("ToneStart          : %.3f\n", ToneStart);
  printf("ToneBand           : %.3f\n", ToneBand);
  printf("ToneStep           : %.3f\n", ToneStep);
  printf("SecondsOneChar     : %.3f\n", SecondsOneChar);
  printf("SamplesOneChar     : %d\n", SamplesOneChar);
  printf("ToneLines          : %d\n", ToneLines);
  printf("SineTableLength    : %d\n", SineTableLength);
  printf("DDSFractionBits    : %d\n", DDSFractionBits);
  printf("FILTER_TAP_NUM     : %d\n", FILTER_TAP_NUM);
  printf("FILTER_PRECISION   : %d\n", FILTER_PRECISION);

  for(uint32_t i = 0; i < ToneLines; ++i)
  {
    float tone = ToneStart + i * ToneStep;
    printf( "Tone  %2d = %3.2fHz, +%2.4f (0x%08x) Incr, %.4f samp/tone.\n"
            , i, tone
            , (float)DDSTonePhase[i] / (1 << DDSFractionBits) 
            , DDSTonePhase[i]
            , (float)FILTER_SAMPLE_FREQ / tone
          );
  }

  printf("\n-- Starting the loop --------\n");

  TimerTc3.initialize(1000000UL / FILTER_SAMPLE_FREQ);
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
      DDSToneAcc[i] += DDSTonePhase[i];   // Calc the next oscilator value
      if (CharLine & (1u << i))           // Check if dot is needed.
        Signal += SineTable[ (DDSToneAcc[i] >> DDSFractionBits) % SineTableLength ]; // [.14]
    }

    Signal /= 1 << 8;                     // [4.14] => [.10]
    Signal = FilterLP(Signal);            // Do the filter convolution

    if (Signal < -512) Signal = -512;     // Only 10 bits DAC in SAMD21
    if (Signal >  511) Signal =  511;     // Range -512 ... 511

    SampleDAC = (Signal+512) & 0x3FF;     // Uplist signal and (hard) set to 10 bits
    GetNewSample = false;

    if (CharLineNextCount++ == SamplesOneChar) 
    {
      CharLineNextCount = 0;
      fontGetNextLine();
    }
  }

  if (StartTimerOverflow != 0 && millis() - StartTimerOverflow > 500)
  {
    StartTimerOverflow = 0;
    printf("Error: Sample-rate overflow!\n");
    digitalWrite(LED_BUILTIN, HIGH);      // turn the samplerate overflow LED OFF
  }
}

void timerIsr()
{
  analogWrite(A0, SampleDAC);                     // Output the DAC analog signal

  if (GetNewSample) {                             // Check overrun
    if (StartTimerOverflow == 0) {                // LED timer is off then set LED on
      StartTimerOverflow = millis();              // Set the LED on timer
      digitalWrite(LED_BUILTIN, LOW);             // Turn the LED on, XIAO=LOW
    }
  }

  GetNewSample = true;                            // Generate a new sample in the loop()
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

#if 0
  CharLine <<= 1;           // Shift char one bit
  CharLine ^= 0xFFFF;       // Inverse
#elif 0
  CharLine |= 0x8000;     // Underline the text
#endif
}

// TODO Taps[] only half use
int FilterLP(int input)                         // Input 10bits signed
{
  // int32_t: 6,34b (0..81) + 10b sample + 11.18b taps = 28bits
  //          FILTER_TAP_NUM  0..1024 DAC  High FilterTaps[]
  int32_t FilterACC = 0;

  FilterHistory[FilterLastIndex++] = input;    // [.10]
  if(FilterLastIndex == FILTER_TAP_NUM)
    FilterLastIndex = 0;

  int index = FilterLastIndex;
  for(int i = 0; i < FILTER_TAP_NUM; ++i) 
  {
    index = index != 0 ? index-1 : FILTER_TAP_NUM-1;
    FilterACC += (int32_t)FilterHistory[index] * FilterTaps[i];
  };

  return FilterACC >> FILTER_PRECISION;
}
