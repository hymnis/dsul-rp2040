// DSUL - Disturb State USB Light : RP2040 Firmware
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdio.h"
#include "pico/time.h"
#include "hardware/gpio.h"
#include <tusb.h>
#include "dsul_patterns.h"
#include "dsul_timer.h"

// Version //
#define MAJOR 0
#define MINOR 1
#define PATCH 0

// Fixed values //
#ifdef PICO_DEFAULT_WS2812_PIN
#define NEOPIN     PICO_DEFAULT_WS2812_PIN       // Pin where NeoPixel is connected
#else
#define NEOPIN     2                             // Pin where NeoPixel is connected
#endif

#ifdef ADAFRUIT_QTPY_RP2040
#define NEOPOWER   PICO_DEFAULT_WS2812_POWER_PIN // Pin providing power to the NeoPixel
#endif

#define NUMPIXELS  1   // Number of NeoPixels
#define BRIGHT_MIN 10  // Minimum brightness level
#define BRIGHT_MAX 120 // Maximum brightness level

// Macro for constraining numerical values from input
#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))

uint32_t show_color = 0;       // default color, 0 = black/off
uint16_t show_brightness = 50; // default brightness (1-255)
uint8_t show_mode = 1;         // default mode, 1 = solid
bool show_dim = false;         // default dime state, false = no dimming
uint16_t host_timeout = 60;    // default timeout, 60 seconds
uint8_t input_count = 0;
char input_string[13] = "";
bool input_state = false;
bool wait_state = true;
bool color_reset = false;

void dotComplete();
void heartbeatComplete();
DsulPatterns Dot(NUMPIXELS, NEOPIN, NEO_GRB + NEO_KHZ800, &dotComplete);
DsulTimer Heartbeat(1000 * host_timeout, &heartbeatComplete);

void dotSetup() {
#ifdef ADAFRUIT_QTPY_RP2040
  gpio_init(NEOPOWER);
  gpio_set_dir(NEOPOWER, GPIO_OUT);
  gpio_put(NEOPOWER, 1);
#endif

  Dot.begin();
  Dot.setBrightness(50);
  Dot.Rainbow(45);
  show_color = Dot.Color(0, 0, 0); // set color to black/off as default
}

// Set the current mode
void setShowMode() {
  if (show_mode == 0) { // off mode
    Dot.ActivePattern = NONE;
    Dot.Color1 = Dot.Color(0, 0, 0);
  } else if (show_mode == 1) { // solid mode
    Dot.Solid(show_color);
  } else if (show_mode == 2) { // blink mode
    Dot.Blink(show_color, 500);
  } else if (show_mode == 3) { // flash mode
    Dot.Blink(show_color, 250);
  } else if (show_mode == 4) { // pulse mode
    Dot.Pulse(show_color, 20, BRIGHT_MAX);
  }
}

// Set the current dim mode
void setDimMode() {
  if (show_dim) {
    Dot.Dim = true;
  } else {
    Dot.Dim = false;
  }
}

// Send OK command
void sendOk() {
  printf("+!#");
  wait_state = false;
  color_reset = true;
  Heartbeat.Reset();
}

// Send NOK command
void sendNOK() { printf("-!#"); }

// Send a ping command
void sendPing() { printf("-?#"); }

// Run once pattern is complete (if needed)
void dotComplete() {
  // reverse direction for pulse on completion
  if (Dot.ActivePattern == PULSE) {
    Dot.Reverse();
  }
}

// No heartbeat felt, send ping and wait for host to reply
void heartbeatComplete() {
  wait_state = true;
  sendPing();
}

// Reset input variables
void resetInput() {
  input_state = false;
  input_count = 0;
  memset(input_string, 0, sizeof input_string);
}

// Handle the input sequence
void handleInput() {
  char type = input_string[0];
  char key = input_string[1];

  if (type == '-') {  // - = 45
    if (key == '!') { // ! = 33
      // information requested
      // respond with version, leds, color, brightness and mode etc.
      char response_v[13];
      char response_ll[6];
      char response_lb[10];
      char response_cc[9];
      char response_cb[6];
      char response_cm[6];
      char response_cd[4];

      sprintf(response_v, "v%03d.%03d.%03d", MAJOR, MINOR, PATCH);
      printf(response_v);
      sprintf(response_ll, "ll%03d", Dot.numPixels());
      printf(response_ll);
      sprintf(response_lb, "lb%03d:%03d", BRIGHT_MIN, BRIGHT_MAX);
      printf(response_lb);
      sprintf(response_cc, "cc%06d", show_color);
      printf(response_cc);
      sprintf(response_cb, "cb%03d", show_brightness);
      printf(response_cb);
      sprintf(response_cm, "cm%03d", show_mode);
      printf(response_cm);
      sprintf(response_cd, "cd%01d", show_dim);
      printf(response_cd);
      printf("#");
    } else if (key == '?') { // ? = 63
      // ping received
      sendOk();
    } else {
      sendNOK(); // unknown key, not OK
    }
  } else if (type == '+') { // + = 43
    if (key == 'l') {
      // set LED color
      // +l rrrgggbbb
      uint16_t r, g, b = 0;
      char color_value[4] = "";

      color_value[0] = input_string[2];
      color_value[1] = input_string[3];
      color_value[2] = input_string[4];
      color_value[3] = '\0';
      r = atoi(color_value);
      r = constrain(r, 0, 255);
      color_value[0] = input_string[5];
      color_value[1] = input_string[6];
      color_value[2] = input_string[7];
      color_value[3] = '\0';
      g = atoi(color_value);
      g = constrain(g, 0, 255);
      color_value[0] = input_string[8];
      color_value[1] = input_string[9];
      color_value[2] = input_string[10];
      color_value[3] = '\0';
      b = atoi(color_value);
      b = constrain(b, 0, 255);

      show_color = Dot.Color(r, g, b);
      Dot.Color1 = show_color;
      sendOk();
    } else if (key == 'b') {
      // set led BRIGHTNESS
      // +b xxx
      char bright_value[4] = "";

      bright_value[0] = input_string[2];
      bright_value[1] = input_string[3];
      bright_value[2] = input_string[4];
      bright_value[3] = '\0';
      show_brightness = atoi(bright_value);
      show_brightness = constrain(show_brightness, BRIGHT_MIN, BRIGHT_MAX);

      Dot.setBrightness(show_brightness);
      sendOk();
    } else if (key == 'm') {
      // set show MODE
      // +m xxx
      char show_value[4] = "";
      show_value[0] = input_string[2];
      show_value[1] = input_string[3];
      show_value[2] = input_string[4];
      show_value[3] = '\0';
      show_mode = atoi(show_value);

      setShowMode();
      sendOk();
    } else if (key == 'd') {
      // set led color DIM
      // +d x
      char dim_value[2] = "";
      dim_value[0] = input_string[2];
      dim_value[1] = '\0';
      show_dim = atoi(dim_value);

      setDimMode();
      sendOk();
    } else {
      sendNOK(); // unknown key, not OK
    }
  } else {
    sendNOK(); // unknown type, not OK
  }

  resetInput();
}

int main() {
    stdio_init_all();
    dotSetup();
    while (!tud_cdc_connected()) { sleep_ms(100); } // wait for usb to initialize
  #ifdef RASPBERRYPI_PICO
    printf("Board: ");
    printf("Arduino QT PY");
  #endif
  #ifdef ADAFRUIT_QTPY_RP2040
    printf("Board: ");
    printf("Arduino QT PY");
  #endif
    
    // start main loop
    while (true) {
        Heartbeat.Update();
        Dot.Update();

        if (wait_state) {
            // wait for host connection
            // cycle LED colors while waiting
            Dot.ActivePattern = RAINBOW;
            Dot.Interval = 45;
            Dot.TotalSteps = 255;
        } else {
            if (color_reset == true) {
                // wait is over. continue from last mode and color
                color_reset = false;
                setShowMode();
                setDimMode();
            }
        }

        int input;
        while ((input = tud_cdc_read_char()) != -1) {
            // reading char into 'c'
            if (input == 35) { // #
              // sequence complete
              handleInput();
            } else {
              // receiving data
              if (input_count > 13) {
                // received more chars than we should. abort
                sendNOK();
                resetInput();
              } else {
                input_string[input_count] = input;
                input_count += 1;
              }
            }
        }
    }

    return 0;
};