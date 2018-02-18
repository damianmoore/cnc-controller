#define LED_BLUE 6
#define LED_GREEN 5
#define LED_RED 3

#define BUTTON 9      // E-stop button
#define FEED_HOLD 10    // A1 on GRBL
#define CYCLE_START 11  // A2 on GRBL


#define STATUS_BOOTING 'B'
#define STATUS_IDLE 'I'
#define STATUS_RUNNING 'R'
#define STATUS_PAUSED 'P'
#define STATUS_DATA 'D'

char grblStatus = 'B';
unsigned long buttonStartTime;
bool paused = false;
String data;


void setStatus(char status) {
  int red = 0, green = 0, blue = 0;
  
  if (status != 'D') {
    grblStatus = status;
  }
  
  if (status == STATUS_BOOTING) {
    red = 255; green = 204; blue = 0;
  }
  else if (status == STATUS_IDLE) {
    red = 0; green = 255; blue = 7;
  }
  else if (status == STATUS_RUNNING) {
    red = 7; green = 0; blue = 255;
  }
  else if (status == STATUS_PAUSED) {
    red = 255; green = 0; blue = 0;
  }
  else if (status == STATUS_DATA) {
    red = 0; green = 0; blue = 0;
  }
  
  analogWrite(LED_RED, red/4);
  analogWrite(LED_GREEN, green/4);
  analogWrite(LED_BLUE, blue/4);
}

void sendPulse(int pin) {
  digitalWrite(pin, LOW);
  delay(100);
  digitalWrite(pin, HIGH);
}


void setup() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);
  pinMode(FEED_HOLD, OUTPUT);
  pinMode(CYCLE_START, OUTPUT);
  
  digitalWrite(FEED_HOLD, HIGH);
  digitalWrite(CYCLE_START, HIGH);
  
  setStatus(STATUS_BOOTING);
  Serial.begin(115200);
}

void loop() {
  if (Serial.available()) {
    data = Serial.readStringUntil('\n');
    if (data.startsWith("<Run")) {
      setStatus(STATUS_RUNNING);
    }
    else if (data.startsWith("<Idle")) {
      setStatus(STATUS_IDLE);
    }
    else if (data.startsWith("<Hold")) {
      setStatus(STATUS_PAUSED);
    }
    else if (data.startsWith("ok")) {
      setStatus(STATUS_DATA);
      delay(5);
      setStatus(grblStatus);
    }
    else if (data.startsWith("Grbl")) {
      setStatus(STATUS_IDLE);
    }
  }
  
  if (digitalRead(BUTTON) == LOW) {
    buttonStartTime = millis();
    paused = false;
    while (millis() < buttonStartTime + 1000) {
      if (digitalRead(BUTTON) == HIGH) {
        setStatus(STATUS_PAUSED);
        paused = true;
        sendPulse(FEED_HOLD);
      }
    }
    if (!paused) {
      setStatus(STATUS_RUNNING);
      sendPulse(CYCLE_START);
      while (digitalRead(BUTTON) == LOW) {}
    }
  }
}
