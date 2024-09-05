#define SENSORS 4
#define THRESHOLD 200

enum Action : int {
  NEXT,
  PREV,
  PLAY,
  PAUSE,
  VOLUP,
  VOLDW,
  INVLD
};

unsigned long previousMillis = 0;
const long interval = 15000;

int sensorsTrig[SENSORS] = {9, 10, 11, 12};
int sensorsGestures[SENSORS] = {0, 0, 0, 0};
int idx = 0;

bool readSensor(int sensorNumber);
Action getAction();
void checkAction();
void reset();

void setup() {
  Serial.begin(9600);

  for (int i = 0; i < SENSORS; ++i)
    pinMode(sensorsTrig[i], OUTPUT);
}

void loop() {
  for (int sensorNumber = 1; sensorNumber <= SENSORS; sensorNumber++) {
    int lastTrigeredSensorNumber = sensorsGestures[(idx + SENSORS - 1) % SENSORS];

    if (sensorNumber == lastTrigeredSensorNumber)
      continue;

    if (readSensor(sensorsTrig[sensorNumber - 1])) {
      sensorsGestures[idx] = sensorNumber;
      idx = (idx + 1) % SENSORS;
      previousMillis = millis();
      
      checkAction();
    }
  }

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    Serial.println("No action was detected, during the last 15 seconds...RESET");
    previousMillis = currentMillis;
    reset();
  }
}

bool readSensor(int sensorPin) {
  long duration, distance;

  pinMode(sensorPin, OUTPUT); // Set pin as OUTPUT
  digitalWrite(sensorPin, LOW);  // Clear the trigger
  delayMicroseconds(2);
  
  // Sets the trigger pin to HIGH state for 5 microseconds
  digitalWrite(sensorPin, HIGH);  
  delayMicroseconds(5);
  digitalWrite(sensorPin, LOW);

  // Set pin as INPUT
  pinMode(sensorPin, INPUT); 
  
  // Read the signal from the sensor: a HIGH pulse whose
  // duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object
  duration = pulseIn(sensorPin, HIGH);  
  
  // Convert the time into a distance
  distance = duration / 58; 
  
  return distance < THRESHOLD;
}

Action getAction() {
  int trigs[SENSORS] = {
    sensorsGestures[idx % SENSORS],
    sensorsGestures[(idx + 1) % SENSORS],
    sensorsGestures[(idx + 2) % SENSORS],
    sensorsGestures[(idx + 3) % SENSORS]
  };

  if (trigs[2] == 1 && trigs[3] == 3)
    return Action::PAUSE;
  else if (trigs[2] == 3 && trigs[3] == 1)
    return Action::PLAY;
  else if (trigs[2] == 2 && trigs[3] == 4)
    return Action::PREV;
  else if (trigs[2] == 4 && trigs[3] == 2)
    return Action::NEXT;
  else if (trigs[0] == 1 && trigs[1] == 2 && trigs[2] == 3 && trigs[3] == 4)
    return Action::VOLUP;
  else if (trigs[0] == 1 && trigs[1] == 4 && trigs[2] == 3 && trigs[3] == 2)
    return Action::VOLDW;
  
  return Action::INVLD;
}

void checkAction() {
  switch (getAction()) {
    case Action::PAUSE:
      Serial.println("STOP");
      break;
    case Action::PLAY:
      Serial.println("PLAY");
      break;
    case Action::PREV:
      Serial.println("PREV");
      break;
    case Action::NEXT:
      Serial.println("NEXT");
      break;
    case Action::VOLUP:
      Serial.println("VOLUP");
      break;
    case Action::VOLDW:
      Serial.println("VOLDOWN");
      break;
    default:
      return;
  }
  reset();
}

void reset() {
  for (int i = 0; i < SENSORS; ++i)
    sensorsGestures[i] = 0;
  idx = 0;
  delay(1000);
}
