// =========================================================
// Arduino Spotify Controller
// Four ultrasonic sensors turn hand gestures into Spotify
// commands, printed over serial for the Python script.
// =========================================================
#include <Arduino.h>

// =========================================================
// Configuration
// =========================================================
constexpr int     SENSOR_COUNT = 4;
constexpr uint8_t SENSOR_PINS[SENSOR_COUNT] = { 9, 10, 11, 12 };

constexpr unsigned long DETECT_THRESHOLD_CM  = 200;    // 200 for TinkerCAD, ~20 on real hardware
constexpr unsigned long ECHO_TIMEOUT_US      = 30000;  // max echo wait, ~5 m range
constexpr unsigned long US_PER_CM_ROUND_TRIP = 58;
constexpr unsigned long PING_SETTLE_MS       = 30;     // let echoes die out between pings
constexpr unsigned long GESTURE_TIMEOUT_MS   = 2000;   // give up on a half-finished gesture
constexpr unsigned long ACTION_COOLDOWN_MS   = 500;    // short break after each command

// =========================================================
// Gesture Table
// =========================================================
// Sensor numbers 1-4, oldest first. The command strings are
// what the Python script listens for.
struct Gesture {
    const char* command;
    uint8_t     length;
    uint8_t     pattern[SENSOR_COUNT];
};

const Gesture GESTURES[] = {
    { "STOP",    2, { 1, 3 } },
    { "PLAY",    2, { 3, 1 } },
    { "PREV",    2, { 2, 4 } },
    { "NEXT",    2, { 4, 2 } },
    { "VOLUP",   4, { 1, 2, 3, 4 } },
    { "VOLDOWN", 4, { 1, 4, 3, 2 } },
};

constexpr int GESTURE_COUNT = sizeof(GESTURES) / sizeof(GESTURES[0]);

// =========================================================
// State
// =========================================================
uint8_t       history[SENSOR_COUNT]    = { 0 };     // last triggered sensors, oldest first, 0 = empty
bool          wasInRange[SENSOR_COUNT] = { false };
unsigned long lastTriggerMillis        = 0;
unsigned long lastActionMillis         = 0;

// =========================================================
// Function Prototypes
// =========================================================
bool           readSensor(uint8_t sensorPin);
void           recordTrigger(uint8_t sensorNumber);
const Gesture* matchGesture();
void           clearGesture();
void           expireStaleGesture();

// =========================================================
// Setup
// =========================================================
void setup() {
    Serial.begin(9600);
    // no pinMode here, readSensor() switches the shared pin itself
}

// =========================================================
// Main Loop
// =========================================================
void loop() {
    expireStaleGesture();

    for (int i = 0; i < SENSOR_COUNT; ++i) {
        bool inRange = readSensor(SENSOR_PINS[i]);
        bool cooling = (millis() - lastActionMillis < ACTION_COOLDOWN_MS);

        // trigger only when a hand first enters range, no repeats while hovering
        if (inRange && !wasInRange[i] && !cooling) {
            recordTrigger(static_cast<uint8_t>(i + 1));
        }
        wasInRange[i] = inRange;

        delay(PING_SETTLE_MS);
    }
}

// =========================================================
// Sensor Reading
// =========================================================
bool readSensor(uint8_t sensorPin) {
    // Send trigger pulse
    pinMode(sensorPin, OUTPUT);
    digitalWrite(sensorPin, LOW);
    delayMicroseconds(2);
    digitalWrite(sensorPin, HIGH);
    delayMicroseconds(5);
    digitalWrite(sensorPin, LOW);

    // Read echo
    pinMode(sensorPin, INPUT);
    unsigned long duration = pulseIn(sensorPin, HIGH, ECHO_TIMEOUT_US);

    // 0 = no echo (timeout / dead sensor), not distance 0
    if (duration == 0) {
        return false;
    }

    return (duration / US_PER_CM_ROUND_TRIP) < DETECT_THRESHOLD_CM;
}

// =========================================================
// Gesture Recording & Matching
// =========================================================
void recordTrigger(uint8_t sensorNumber) {
    // shift left, newest last
    for (int i = 0; i < SENSOR_COUNT - 1; ++i) {
        history[i] = history[i + 1];
    }
    history[SENSOR_COUNT - 1] = sensorNumber;
    lastTriggerMillis         = millis();

    const Gesture* gesture = matchGesture();
    if (gesture != nullptr) {
        Serial.println(gesture->command);
        clearGesture();
        lastActionMillis = millis();
    }
}

const Gesture* matchGesture() {
    for (int g = 0; g < GESTURE_COUNT; ++g) {
        const Gesture& gesture = GESTURES[g];
        const int offset = SENSOR_COUNT - gesture.length;

        // unfilled slots are 0 so they never match
        bool matches = true;
        for (int i = 0; i < gesture.length && matches; ++i) {
            matches = (history[offset + i] == gesture.pattern[i]);
        }
        if (matches) {
            return &gesture;
        }
    }
    return nullptr;
}

// =========================================================
// Reset Handling
// =========================================================
void clearGesture() {
    // keep wasInRange, a hovering hand must leave before triggering again
    for (int i = 0; i < SENSOR_COUNT; ++i) {
        history[i] = 0;
    }
}

void expireStaleGesture() {
    bool historyEmpty = (history[SENSOR_COUNT - 1] == 0);
    if (!historyEmpty && millis() - lastTriggerMillis >= GESTURE_TIMEOUT_MS) {
        Serial.println("# gesture timed out");  // '#' = debug line, not a command
        clearGesture();
    }
}
