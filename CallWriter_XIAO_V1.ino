// PE0FKO, CallWriter, V2.0
//
#include <TimerTC3.h>
#include "Font.h"

const     float     SecondsOneChar            = 2.4;          // the number of seconds for TX one charracter
const     uint32_t  ToneLines                 = 16;           // Number of tone carriers TX at the same time
const     float     ToneStart                 = 1000;          // Tone start Hz
const     float     ToneBand                  = 2000;          // Bandbreete Hz
const     float     ToneStep                  = ToneBand / ToneLines;
const     uint32_t  SampleRate                = 22000;        // 8KHz sample rate
const     uint32_t  SineTableLength           = 1 << 9;       // Length of sine table
const     uint32_t  OscFraction               = 1 << 16;      // Oscilator 16bit fraction
const     uint32_t  FilterTableLength         = 32;           // Length of filter table

static    uint32_t  SineTable[SineTableLength];
static    uint32_t  OSCreg[ToneLines]         = { 0 };
static    uint32_t  OSCincr[ToneLines]        = { 0 };
static    uint32_t  FilterTable[FilterTableLength];
static    int       FilterCounter[ToneLines]  = { 0 };

static    uint32_t  NextLineCount             = SecondsOneChar * SampleRate / FONT_LENGTH;
static    uint32_t  CharLine                  = 0;          // 16 bits of char line
static    uint32_t  CharNextCount             = 0;          // SR count for next char line load.

volatile  bool      calcNext                  = true;
volatile  uint64_t  signal                    = 0;

void setup()
{
  pinMode(D5, OUTPUT);
  pinMode(D6, OUTPUT);
  pinMode(13, OUTPUT);    

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

  for(uint32_t i = 0; i < FilterTableLength; i++)
  {
    float s = 0.5 + cos(  M_PI
                        + M_PI
                          / (FilterTableLength + 1)
                          * (i+1)
                        ) / 2;
   
    FilterTable[i] = s * (1U<<16);        // 0 ... 65xxx
  }

//  for(uint32_t i = 0; i < FilterTableLength; i++)
//    Serial.printf("Filter[%2d]: %5d\n", i, FilterTable[i]);

  for(uint32_t i = 0; i < ToneLines; i++)
  {
    // Start with shifted phase signals, more flat power pattern.
    OSCreg[i]  = random(0, OscFraction * SineTableLength);

    uint32_t I = ToneLines - 1 - i;
//    uint32_t I = i;

    OSCincr[i] = (uint32_t)(
       (uint64_t)                   // Need >36 bits
        (ToneStart + I * ToneStep)  // 12bits (< 4096)
       * OscFraction                // 16bits
       * SineTableLength            // 8bits
       / SampleRate
       );
  }

  fontInit();

#if 1
  if (Serial) 
  {
    Serial.println("\n-- CallWriter V2.00 ----------------------");
    Serial.printf("Sizeof int      : %d\n", sizeof(int));
    Serial.printf("Sizeof float    : %d\n", sizeof(float));
    Serial.printf("Sizeof double   : %d\n", sizeof(double));

    Serial.printf("SampleRate      : %d\n", SampleRate);
    Serial.printf("ToneStart       : %.3f\n", ToneStart);
    Serial.printf("ToneStep        : %.3f\n", ToneStep);
    Serial.printf("ToneLines       : %d\n", ToneLines);
    Serial.printf("SineTableLength : %d\n", SineTableLength);
    Serial.printf("OscFraction     : %d\n", OscFraction);

#if 0
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
#endif

    Serial.println("----------------------------------------------");
  }
#endif

//  TimerTc3.initialize(1000000UL / SampleRate);
  TimerTc3.initialize(1000000UL / 4);
  TimerTc3.attachInterrupt(timerIsr);
}


void loop() 
{
  if (calcNext)
  {
    digitalWrite(D6, HIGH);

    signal = 0;

    for(uint32_t i = 0; i < ToneLines; i++)
    {
      OSCreg[i] += OSCincr[i];        // Calc the next oscilator value

      if (!(i==0))
        continue;


#if 0
      if (CharLine & (1 << i))        // Check if dot is needed.
        signal += SineTable[ (OSCreg[i] / OscFraction) % SineTableLength ];
#else
      uint32_t SineTableIndex, Sine, Multiply;

      SineTableIndex = OSCreg[i] / OscFraction;
      SineTableIndex %= SineTableLength;
      Sine = SineTable[ SineTableIndex ];
      
//      Serial.printf("[%-2d] SineTable[%d]=%u\n", i, SineTableIndex, Sine);

      if (CharLine & bit(i))      // Ga naar de 1 toe!
      {
        if (FilterCounter[i] < FilterTableLength)        // 0 .. 2
        {
          Multiply = FilterTable[ FilterCounter[i] ];
//          signal += (s * FilterTable[ FilterCounter[i] ]) / (1<<16);
          signal += (Sine * Multiply) / 65536;    // 10.0 * 16.0 / 16.0

//          Serial.printf("signal=%d\n", Signe);
//          Serial.printf("signal=%ld\n", signal);

          Serial.printf("[%-2d] To one : SineTable[%d]=%u FilterTable[%-2d]=%-5d .. "
                      , i
                      , SineTableIndex
                      , SineTable[SineTableIndex]
                      , FilterCounter[i]
                      , FilterTable[ FilterCounter[i] ]
                      );
          Serial.printf("signal=%ld\n", signal);

          FilterCounter[i] += 1;
        }
        else 
        {
//          Serial.printf("[%2d] Full signal.\n", i);
          // Full signal add
          signal += Sine;
        }
      }
      else
      {                         // Ga naar de 0 toe!
        if (FilterCounter[i] > 0)      // 3 .. 0
        {
          FilterCounter[i] -= 1;
          signal += (Sine * FilterTable[ FilterCounter[i] ]) / (1<<16);
//          Serial.printf("[%2d] To zero: %5d < %d\n", i, FilterTable[ FilterCounter[i] ], signal);
        }
        else 
        {
//          Serial.printf("[%2d] No signal.\n", i);
          // No signal add
        }
      }
#endif
    }
    signal /= ToneLines;

    digitalWrite(D6, LOW);

    if (CharNextCount++ == NextLineCount) 
    {
      CharNextCount = 0;
      fontGetNextLine();
    }

    calcNext = false;
  }

#if 0
    // Generate a voltage between 0 and 3.3V.
    // 0= 0V, 1023=3.3V, 512=1.65V, etc.

    // Now read A1 (connected to A0), and convert that
    // 12-bit ADC value to a voltage between 0 and 3.3.
    float voltage = analogRead(A1) * 3.3 / 4096.0;
    Serial.println(voltage); // Print the voltage.
#endif
}

void timerIsr()
{
//  digitalWrite(D5, HIGH);
  if (!calcNext)
  {
    // Only 10bits DAC
    analogWrite(A0, signal & 0x3FF);
    calcNext = true;
  } else
    digitalWrite(13, HIGH);
//  digitalWrite(D5, LOW);
}

static	uint32_t	indx_ch, indx_ln;	// char and bit-line index

void
fontGetNextLine()
{
	if (FontEndOfMessageText()) {
		fontInit();
	}

	if (indx_ln < FONT_LENGTH)
	{
		CharLine = FontTable[indx_ch+1] << 8 | FontTable[indx_ch+0];
		indx_ch += 2;
		indx_ln += 1;
	}
	else if (indx_ln <= FONT_LENGTH)
	{
//		indx_ch += 2;     // del
		indx_ln += 1;
//		indx_ln = 0;      // weg
//		CharLine = 0x5555;
		CharLine = 0;
	}
	else
	{
		indx_ln = 0;
//		CharLine = 0xaaaa;
		CharLine = 0;
	}

  CharLine |= 0x8000;     // Underline the text
//  CharLine |= 0x4000;     // Underline the text
}

void
fontInit( void )
{
//  Serial.printf("text_init\n");
	indx_ch = 0;
	indx_ln = 0;
	fontGetNextLine();
}

bool
FontEndOfMessageText( void )
{
	return indx_ch >= sizeof(FontTable);
}
