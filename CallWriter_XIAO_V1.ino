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
const     uint32_t  ToneLines                 = 16;           // Number of tone carriers TX at the same time
const     float     ToneStart                 = 2500;          // Tone start Hz
const     float     ToneBand                  = 400;          // Bandbreete Hz
const     float     ToneStep                  = ToneBand / ToneLines;
const     uint32_t  SampleRate                = 24000;        // 8KHz sample rate
const     uint32_t  SineTableLength           = 1 << 9;       // Length of sine table
const     uint32_t  OscFraction               = 1 << 16;      // Oscilator 16bit fraction
const     uint32_t  FilterTableLength         = 32;           // Length of filter table

static    uint32_t  SineTable[SineTableLength];
static    uint32_t  OSCreg[ToneLines]         = { 0 };
static    uint32_t  OSCincr[ToneLines]        = { 0 };

//static    uint16_t* pFontTable                = ((uint16_t*)FontTable);
static    uint16_t* pFontTable                = 0;

static    uint32_t  NextLineCount             = SecondsOneChar * SampleRate / FONT_LENGTH;
static    uint32_t  CharLine                  = 0;          // 16 bits of char line
static    uint32_t  CharNextCount             = 0;          // SR count for next char line load.


void setup()
{
//  pinMode(13, OUTPUT);    
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
    double s = sin( (2.0 * M_PI * i) / SineTableLength );
    SineTable[i] = (uint32_t)( ((s + 1.0)/2.0) * 1024.0);   // 0.0 ... 1024.0
  }

  for(uint32_t i = 0; i < ToneLines; i++)
  {
    // Start with shifted phase signals, more flat power pattern.
    OSCreg[i]  = random(0, OscFraction * SineTableLength);

    uint32_t I = ToneLines - 1 - i;
//    uint32_t I = i;

    OSCincr[i] = (uint32_t)(
       (uint64_t)                   // Need >37 bits
        (ToneStart + I * ToneStep)  // 12bits (< 4096)
       * OscFraction                // 16bits
       * SineTableLength            // 9bits
       / SampleRate
       );
  }

  fontInit();

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
    Serial.printf("OscFraction     : %d\n", OscFraction);

    for(uint32_t i = 0; i < ToneLines; ++i)
    {
      float tone = ToneStart + i * ToneStep;
      Serial.printf(  "Tone  %2d = %3.2fHz, +%2.4f (%d) Incr, %.4f samples.\n"
                    , i, tone
                    , (float)OSCincr[i] / OscFraction 
                    , OSCincr[i]
                    , (float)SampleRate / tone
                    );
    }

    Serial.printf("----------------------------------------------\n\n");
  }
#endif

  TimerTc3.initialize(1000000UL / SampleRate);
  TimerTc3.attachInterrupt(timerIsr);
}

static bool       GetNewSample = true;
static uint64_t   signal = 0;

void loop() 
{
  if (GetNewSample)
  {
    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED off by making the voltage LOW

    signal = 0;
    for(uint32_t i = 0; i < ToneLines; i++)
    {
      OSCreg[i] += OSCincr[i];        // Calc the next oscilator value

      if (CharLine & (1 << i))        // Check if dot is needed.
        signal += SineTable[ (OSCreg[i] / OscFraction) % SineTableLength ];
    }

    signal /= ToneLines;

    if (CharNextCount++ == NextLineCount) 
    {
      CharNextCount = 0;
      fontGetNextLine();
    }
    GetNewSample = false;
  }
}

void timerIsr()
{
  if (GetNewSample)
    digitalWrite(LED_BUILTIN, LOW);  // turn the LED on XIAO

  // Only 10bits DAC
  analogWrite(A0, signal & 0x3FF);
  GetNewSample = true;
}

static	uint32_t	indx_ch, indx_ln;	// char and bit-line index

void
fontGetNextLine()
{
//	if (FontEndOfMessageText())
//		fontInit();

	if (indx_ch >= sizeof(FontTable) / 1)
		fontInit();

	if (indx_ln < FONT_LENGTH)
	{
#if 1
		CharLine = FontTable[indx_ch+1] << 8 | FontTable[indx_ch+0];
#elif 1

//		CharLine = FontTable[indx_ch+1] << 8 | FontTable[indx_ch+0];
//    uint32_t x = *pFontTable;
    CharLine = *pFontTable++;

//    uint32_t x = *pFontTable++;
//		CharLine = x;

//	  Serial.printf("Font: Line %04x -- %04x\n", CharLine, x);
//	  Serial.printf("Font: Line %04x\n", CharLine);

#else
		CharLine = pFontTable[indx_ch / 2];
#endif
		indx_ch += 2;
		indx_ln += 1;
	}
	else if (indx_ln <= FONT_LENGTH)
	{
		indx_ln += 1;
  	CharLine = 0;
	}
	else
	{
		indx_ln = 0;
		CharLine = 0;
	  Serial.printf("Font: Next char (%d).\n", indx_ch);
	}

//  CharLine |= 0x8000;     // Underline the text
}

void
fontInit( void )
{
  Serial.printf("\nFont: initialize (%d).\n\n", sizeof(FontTable));

  pFontTable = (uint16_t*) &FontTable[0];
	indx_ch = 0;
	indx_ln = 0;
//	fontGetNextLine();
}

bool
xFontEndOfMessageText( void )
{
	return indx_ch >= sizeof(FontTable) / 2;
}
