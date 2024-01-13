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
//** History......: https://github.com/pe0fko/CallWriter_XIAO_V1.git
//**
//**************************************************************************

#include <TimerTC3.h>
#include "Font.h"

const     float     SecondsOneChar            = 0.8;          // the number of seconds for TX one charracter
const     float     ToneStart                 = 1000;         // Tone start Hz
const     float     ToneBand                  = 250;          // Bandbreete Hz
const     uint32_t  ToneLines                 = 16;           // Number of tone carriers TX at the same time
const     uint32_t  SampleRate                = 22000;        // 22KHz sample rate
const     float     ToneStep                  = ToneBand / ToneLines;
const     uint32_t  NextLineCount             = SecondsOneChar * SampleRate / FONT_LENGTH;
const     uint32_t  SineTableLength           = 1 << 9;       // Length of sine table
const     uint32_t  DDSFraction               = 1 << 16;      // DDS oscilator 16bit fraction
const     uint32_t  FilterTableLength         = 32;           // Length of filter table

static    int64_t   Signal                    = 0;
static    int32_t   SineTable[SineTableLength];
static    uint32_t  DDSPhaseAcc[ToneLines]    = { 0 };        // DDS phase register
static    uint32_t  DDSFreqReg[ToneLines]     = { 0 };        // DDS frequency register increment the phase reg.
static    uint8_t   const *pFontTable         = FontTable;    // The font table to TX
static    uint32_t  CharLine                  = 0;            // 16 bits of font char, 16x11 font used
static    uint32_t  NextCharLineCounter       = 0;            // Samplerate counter for next char to load
static    bool      GetNewSample              = true;


//=====================================================================
//---- SETUP....  SETUP....  SETUP....  SETUP....  SETUP....    
//=====================================================================
void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  analogWriteResolution(10);        // Set analog out resolution to max, 10-bits

#if 1
  Serial.begin(115200);
  uint32_t millisStart = millis();
  while(!Serial) 
    if (millis() - millisStart > 10000)
      break;
#endif

  for(uint32_t i = 0; i < SineTableLength; i++)
  {
    float s = sin( (2.0 * M_PI * i) / SineTableLength );
    SineTable[i] = (int32_t)(s * 512.0);      // [.10] -512.0 ... 512.0
  }

  for(uint32_t i = 0; i < ToneLines; i++)
  {
    // Start with random phase signals for more flat power pattern.
    DDSPhaseAcc[i]  = random(0, DDSFraction * SineTableLength);  // [.25]

    // Miror the text, or not
    uint32_t I = ToneLines - 1 - i;
//    uint32_t I = i;

    DDSFreqReg[i] = (uint32_t)(     // [7.25] [7.16+9]
       (uint64_t)                   // Need >37 bits
        (ToneStart + I * ToneStep)  // 12 bits (< 4096)
       * DDSFraction                // 16 bits
       * SineTableLength            // 9 bits
       / SampleRate
       );
  }

#if 1
  if (Serial) 
  {
    Serial.printf("\n-- CallWriter V2.00 ----------------------\n");

    Serial.printf("Sizeof int      : %d\n", sizeof(int));
    Serial.printf("Sizeof float    : %d\n", sizeof(float));
    Serial.printf("Sizeof double   : %d\n", sizeof(double));

    Serial.printf("SampleRate      : %d\n", SampleRate);
    Serial.printf("ToneStart       : %.3f\n", ToneStart);
    Serial.printf("ToneStep        : %.3f\n", ToneStep);
    Serial.printf("ToneLines       : %d\n", ToneLines);
    Serial.printf("SineTableLength : %d\n", SineTableLength);
    Serial.printf("DDSFraction     : %d\n", DDSFraction);

    for(uint32_t i = 0; i < ToneLines; ++i)
    {
      float tone = ToneStart + i * ToneStep;
      Serial.printf(  "Tone  %2d = %3.2fHz, +%2.4f (%d) Incr, %.4f samples.\n"
                    , i, tone
                    , (float)DDSFreqReg[i] / DDSFraction 
                    , DDSFreqReg[i]
                    , (float)SampleRate / tone
                    );
    }

    Serial.printf("----------------------------------------------\n\n");
  }
#endif

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
    Signal = 0;
    for(uint32_t i = 0; i < ToneLines; i++)
    {
      DDSPhaseAcc[i] += DDSFreqReg[i];        // Calc the next oscilator value

      if (CharLine & (1u << i))        // Check if dot is needed.
        Signal += SineTable[ (DDSPhaseAcc[i] / DDSFraction) % SineTableLength ]; // [4.10]
    }

    Signal /= ToneLines;                // [4.10] => [.10]

    if (Signal < -512) Signal = -512;   // Only 10 bits DAC in SAMD21
    if (Signal >  511) Signal =  511;   // Range -513 ... 511

    Signal += 512;                      // Uplift negative value
    Signal &= 0x3FF;                    // Hard set 10 bits

    if (NextCharLineCounter++ == NextLineCount) 
    {
      NextCharLineCounter = 0;
      fontGetNextLine();
      digitalWrite(LED_BUILTIN, HIGH);   // turn the overflow LED off, XIAO
    }

    GetNewSample = false;
  }
}

void timerIsr()
{
  if (GetNewSample)                   // Check overrun
    digitalWrite(LED_BUILTIN, LOW);   // turn the LED on, XIAO

  analogWrite(A0, Signal);            // Output the analog signal
  GetNewSample = true;
}

void
fontGetNextLine()
{
  static int indx_char=0;

  if (pFontTable == &FontTable[sizeof FontTable])
  {
    Serial.printf("\nFont: initialize (%d).\n", sizeof(FontTable));
    pFontTable = &FontTable[0];
    indx_char = 0;
    digitalWrite(LED_BUILTIN, HIGH);   // turn the samplerate overflow LED off
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
