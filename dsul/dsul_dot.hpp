// DSUL - Disturb State USB Light : DsulDot class
#include "Adafruit_NeoPixel.hpp"

enum pattern { NONE, RAINBOW, SOLID, BLINK, PULSE };
enum direction { FORWARD, REVERSE };

class DsulDot : public Adafruit_NeoPixel {
public:
  pattern ActivePattern;
  direction Direction;

  uint32_t Interval;
  uint32_t lastUpdate;

  uint32_t Color1;
  uint16_t BrightnessHigh, BrightnessLow;
  uint16_t TotalSteps;
  uint16_t Index;

  bool Dim;

  void (*OnComplete)();

  DsulDot(uint16_t pixels, uint8_t pin, uint8_t type, void (*callback)())
      : Adafruit_NeoPixel(pixels, pin, type) {
    OnComplete = callback;
    Dim = false;
  }

  // Update the pattern
  void Update() {
    int millis = to_ms_since_boot (get_absolute_time());

    if ((millis - lastUpdate) > Interval) {
      lastUpdate = to_ms_since_boot (get_absolute_time());
      switch (ActivePattern) {
        case RAINBOW:
          RainbowUpdate();
          break;
        case SOLID:
          SolidUpdate();
          break;
        case BLINK:
          BlinkUpdate();
          break;
        case PULSE:
          PulseUpdate();
          break;
        default:
          break;
      }
    }
  }

  // Increment the Index and reset at the end
  void Increment() {
    if (Direction == FORWARD) {
      Index++;
      if (Index >= TotalSteps) {
        Index = 0;
        if (OnComplete != NULL) {
          OnComplete();
        }
      }
    } else {
      --Index;
      if (Index <= 0) {
        Index = TotalSteps - 1;
        if (OnComplete != NULL) {
          OnComplete();
        }
      }
    }
  }

  // Reverse the index direction
  void Reverse() {
    if (Direction == FORWARD) {
      Direction = REVERSE;
      Index = TotalSteps - 1;
    } else {
      Direction = FORWARD;
      Index = 0;
    }
  }

  // Initialize for a RainbowCycle
  void Rainbow(uint8_t interval) {
    ActivePattern = RAINBOW;
    Interval = interval;
    TotalSteps = 255;
    Index = 0;
    Direction = FORWARD;
  }

  // Update the Rainbow pattern
  void RainbowUpdate() {
    setBrightness(20);
    for (int i = 0; i < numPixels(); i++) {
      uint32_t color = Wheel(((i * 256 / numPixels()) + Index) & 255);
      if (Dim) {
        color = DimColor(color);
      }
      setPixelColor(i, color);
    }
    show();
    Increment();
  }

  // Initialize for Solid pattern
  void Solid(uint32_t color1, uint16_t brightness) {
    ActivePattern = SOLID;
    Color1 = color1;
    BrightnessHigh = brightness;
  }

  // Update the Solid pattern
  void SolidUpdate() {
    setBrightness(BrightnessHigh);
    ColorSet(Color1);
    show();
  }

  // Initialize for Blink pattern
  void Blink(uint32_t color, uint16_t interval) {
    ActivePattern = BLINK;
    Interval = interval;
    TotalSteps = 2;
    Color1 = color;
    BrightnessLow = 0;
    BrightnessHigh = 100;
    Index = 0;
    Direction = FORWARD;
  }

  // Update the Blink pattern
  void BlinkUpdate() {
    uint16_t brightness = 0;

    if (Index == 0) {
      brightness = BrightnessLow;
    } else {
      brightness = BrightnessHigh;
    }

    setBrightness(brightness);
    ColorSet(Color1);
    show();
    Increment();
  }

  // Initialize for Pulse pattern
  void Pulse(uint32_t color1, uint8_t interval, uint16_t brightnessMax) {
    ActivePattern = PULSE;
    Interval = interval;
    TotalSteps = brightnessMax;
    Color1 = color1;
    Index = 0;
    Direction = FORWARD;
  }

  // Update the Pulse pattern
  void PulseUpdate() {
    setBrightness(Index);
    ColorSet(Color1);
    show();
    Increment();
  }

  // Calculate 50% dimmed version of a color (used by ScannerUpdate)
  uint32_t DimColor(uint32_t color) {
    // Shift R, G and B components one bit to the right
    uint32_t dimColor =
        Color(Red(color) >> 1, Green(color) >> 1, Blue(color) >> 1);
    return dimColor;
  }

  // Set all pixels to a color (synchronously)
  void ColorSet(uint32_t color) {
    if (Dim) {
      color = DimColor(color);
    }
    for (int i = 0; i < numPixels(); i++) {
      setPixelColor(i, color);
    }
    show();
  }

  // Returns the Red component of a 32-bit color
  uint8_t Red(uint32_t color) { return (color >> 16) & 0xFF; }

  // Returns the Green component of a 32-bit color
  uint8_t Green(uint32_t color) { return (color >> 8) & 0xFF; }

  // Returns the Blue component of a 32-bit color
  uint8_t Blue(uint32_t color) { return color & 0xFF; }

  // Input a value 0 to 255 to get a color value.
  // The colours are a transition r - g - b - back to r.
  uint32_t Wheel(uint8_t WheelPos) {
    WheelPos = 255 - WheelPos;
    if (WheelPos < 85) {
      return Color(255 - WheelPos * 3, 0, WheelPos * 3);
    } else if (WheelPos < 170) {
      WheelPos -= 85;
      return Color(0, WheelPos * 3, 255 - WheelPos * 3);
    } else {
      WheelPos -= 170;
      return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
    }
  }
};
