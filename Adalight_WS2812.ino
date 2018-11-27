/*
 * Project forked from Wifsimster/adalight_ws2812
 * 
 * The fork provides the functionality of smooth color transitions
 */
 
#include "FastLED.h"
#include "CircularBuffer.h"

/* 
 *  Configure according to your hardware setup
 */
#define NUM_LEDS    53
#define DATA_PIN    5
#define serialRate  115200 // Baudrate, higher rate allows faster refresh rate and more LEDs (defined in /etc/boblight.conf)

/*
 * Configure the smooth transition settings
 */
#define SMOOTH_TRANSITION_STEPS             32    // number of intermediate colors
#define SMOOTH_TRANSITION_STEP_SIZE         8     // how big each step of intermediate colors is. Recommended: SMOOTH_TRANSITION_STEPS * SMOOTH_TRANSITION_STEP_SIZE > 255
#define SMOOTH_TRANSITION_WAIT_INTERMEDIATE 3000  // wait that many µs after each intermediate color. Should be a mutliple of 100.

/* 
 * Constants
 */
#define FRAME_SIZE (3*NUM_LEDS + 6) // 3 byte per LED + 6 bytes header; can hold up to one complete frame

// Adalight sends a "Magic Word" (defined in /etc/boblight.conf) before sending the pixel data
uint8_t prefix[] = {'A', 'd', 'a'}, hi, lo, chk, i;

CRGB leds[NUM_LEDS];                            // current LED color to be displayed
CRGB leds_target_color[NUM_LEDS];               // target color for smoothing
CircularBuffer<uint8_t, FRAME_SIZE> read_buf;   // stores up to one frame of data


void delay_microseconds_and_read(int microsecs)
{
  while(microsecs)
  {
    fill_read_buf();
    delayMicroseconds(100);
    microsecs -= 100;
  }
}

/*
 * Read one byte from our read_buf
 * if needed, read one byte from serial
 */
uint8_t read_byte()
{
  int read_val = -1;
  if(read_buf.isEmpty())
  {
    while(read_val == -1)
    {
      read_val = Serial.read();
    }
    read_buf.push(read_val);
  }
  
  return read_buf.shift();
}

void fill_read_buf()
{
  while(Serial.available())
  {
    read_buf.push(Serial.read());
  }
}

int smooth_step(int from, int to, int step_size)
{
  int result; 
  
  if(from == to)
  {
    return to;
  }
  
  if(from > to)
  {
    result = from - step_size;
    if(result < to)
    {
      return to;
    }
  }
  else
  {
    result = from + step_size;
    if(result > to)
    {
      return to;
    }
  }

  return result;
}

void show_colors_smooth()
{
  int intermediate_colors_wait_time_micros = SMOOTH_TRANSITION_WAIT_INTERMEDIATE; 
  int intermediate_colors_step_size = SMOOTH_TRANSITION_STEP_SIZE;
  int steps = SMOOTH_TRANSITION_STEPS;
  while(steps--)
  {
    for(int i = 0; i < NUM_LEDS; i++)
    {
      leds[i].r = smooth_step(leds[i].r, leds_target_color[i].r, intermediate_colors_step_size);
      leds[i].g = smooth_step(leds[i].g, leds_target_color[i].g, intermediate_colors_step_size);
      leds[i].b = smooth_step(leds[i].b, leds_target_color[i].b, intermediate_colors_step_size);
      //fill_read_buf(); // Empty the serial buffer and make sure we have the most current frame saved
    }
    FastLED.show();
    //delay_microseconds_and_read(intermediate_colors_wait_time_micros);
    delayMicroseconds(intermediate_colors_wait_time_micros);
  }
}

void setup() {
  // Use NEOPIXEL to keep true colors
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  
  // Initial RGB flash
  LEDS.showColor(CRGB(255, 0, 0));
  delay(500);
  LEDS.showColor(CRGB(0, 255, 0));
  delay(500);
  LEDS.showColor(CRGB(0, 0, 255));
  delay(500);
  LEDS.showColor(CRGB(0, 0, 0));
  
  Serial.begin(serialRate);
  // Send "Magic Word" string to host
  Serial.print("Ada\n");
}

void read_ada_header()
{
  while(true)
  {
    for(i = 0; i < sizeof prefix; i++) 
    {
      // Check next byte in Magic Word
      if(prefix[i] == read_byte()) continue;
      // otherwise, start over
      i = 0;
    }
    
    // Hi, Lo, Checksum  
    hi = read_byte();
    lo = read_byte();
    chk = read_byte();
    
    // If checksum does not match go back to wait
    if (chk == (hi ^ lo ^ 0x55)) 
    {
      break;
    }
  }
}

void loop() { 
  read_ada_header();

  // Read the transmission data and set LED values
  for (uint8_t i = 0; i < NUM_LEDS; i++) 
  {
    leds_target_color[i].r = read_byte();
    leds_target_color[i].g = read_byte();
    leds_target_color[i].b = read_byte();
  }

  // Shows new values
  show_colors_smooth();
}
