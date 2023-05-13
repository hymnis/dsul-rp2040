// DSUL - Disturb State USB Light : DsulTimer class

class DsulTimer {
public:
  uint32_t Interval;
  uint32_t lastUpdate;

  void (*OnComplete)();

  DsulTimer(uint32_t setTime, void (*callback)()) {
    Interval = setTime;
    OnComplete = callback;
  }

  // Update the timer and check if we should fire callback
  void Update() {
    int millis = to_ms_since_boot (get_absolute_time());

    if ((millis - lastUpdate) > Interval) {
      lastUpdate = to_ms_since_boot (get_absolute_time());
      OnComplete();
    }
  }

  // Reset timer
  void Reset() { lastUpdate = to_ms_since_boot (get_absolute_time()); }
};
