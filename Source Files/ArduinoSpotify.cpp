// =========================================================
// Constants & Configuration
// =========================================================
#define SENSORS    4
#define THRESHOLD  200

enum Action : int {
    NEXT,
    PREV,
    PLAY,
    PAUSE,
    VOLUP,
    VOLDW,
    INVLD
};

// =========================================================
// Global Variables
// =========================================================
unsigned long previousMillis = 0;
const    long interval       = 15000;

int sensorsTrig[SENSORS]     = { 9, 10, 11, 12 };
int sensorsGestures[SENSORS] = { 0, 0, 0, 0 };
int idx                      = 0;

// =========================================================
// Function Prototypes
// =========================================================
bool   readSensor(int sensorNumber);
Action getAction(void);
void   checkAction(void);
void   reset(void);

// =========================================================
// Setup
// =========================================================
void setup(void) {
    Serial.begin(9600);

    for (int i = 0; i < SENSORS; ++i) {
        pinMode(sensorsTrig[i], OUTPUT);
    }
}

// =========================================================
// Main Loop
// =========================================================
void loop(void) {
    for (int sensorNumber = 1; sensorNumber <= SENSORS; sensorNumber++) {
        int lastTriggered = sensorsGestures[(idx + SENSORS - 1) % SENSORS];

        if (sensorNumber == lastTriggered) {
            continue;
        }

        if (readSensor(sensorsTrig[sensorNumber - 1])) {
            sensorsGestures[idx] = sensorNumber;
            idx                  = (idx + 1) % SENSORS;
            previousMillis       = millis();

            checkAction();
        }
    }

    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
        Serial.println("No action detected during the last 15 seconds... RESET");
        previousMillis = currentMillis;
        reset();
    }
}

// =========================================================
// Sensor Reading
// =========================================================
bool readSensor(int sensorPin) {
    long duration, distance;

    // Send trigger pulse
    pinMode(sensorPin, OUTPUT);
    digitalWrite(sensorPin, LOW);
    delayMicroseconds(2);
    digitalWrite(sensorPin, HIGH);
    delayMicroseconds(5);
    digitalWrite(sensorPin, LOW);

    // Read echo
    pinMode(sensorPin, INPUT);
    duration = pulseIn(sensorPin, HIGH);

    // Convert to distance
    distance = duration / 58;

    return (distance < THRESHOLD);
}

// =========================================================
// Determine Action
// =========================================================
Action getAction(void) {
    int trigs[SENSORS] = {
        sensorsGestures[idx % SENSORS],
        sensorsGestures[(idx + 1) % SENSORS],
        sensorsGestures[(idx + 2) % SENSORS],
        sensorsGestures[(idx + 3) % SENSORS]
    };

    if (trigs[2] == 1 && trigs[3] == 3) return PAUSE;
    if (trigs[2] == 3 && trigs[3] == 1) return PLAY;
    if (trigs[2] == 2 && trigs[3] == 4) return PREV;
    if (trigs[2] == 4 && trigs[3] == 2) return NEXT;
    if (trigs[0] == 1 && trigs[1] == 2 && trigs[2] == 3 && trigs[3] == 4) return VOLUP;
    if (trigs[0] == 1 && trigs[1] == 4 && trigs[2] == 3 && trigs[3] == 2) return VOLDW;

    return INVLD;
}

// =========================================================
// Action Handling
// =========================================================
void checkAction(void) {
    switch (getAction()) {
        case PAUSE: 
            Serial.println("STOP");
            break;
        case PLAY: 
            Serial.println("PLAY");
            break;
        case PREV: 
            Serial.println("PREV");
            break;
        case NEXT: 
            Serial.println("NEXT");
            break;
        case VOLUP: 
            Serial.println("VOLUP");
            break;
        case VOLDW: 
            Serial.println("VOLDOWN");
            break;
        default:
            return;
    }
    reset();
}

// =========================================================
// Reset Gestures
// =========================================================
void reset(void) {
    for (int i = 0; i < SENSORS; ++i) {
        sensorsGestures[i] = 0;
    }
    idx = 0;
    delay(1000);
}
