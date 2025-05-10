#include <Servo.h>
Servo SG90;
const int ENUL = 1;
const int ENLL = 2;
const int ENUR = 23;
const int ENLR = 24;
const int MUR_F = 3;
const int MUR_B = 4;
const int MLR_F = 5;
const int MLR_B = 6;
const int MUL_F = 7;
const int MUL_B = 8;
const int MLL_F = 9;
const int MLL_B = 10;

const int SF1_TRIGGER_PIN = 11;
const int SF1_ECHO_PIN = 12;
const int SF2_TRIGGER_PIN = 13;
const int SF2_ECHO_PIN = 14;
const int SL_TRIGGER_PIN = 15;
const int SL_ECHO_PIN = 16;
const int SR_TRIGGER_PIN = 17;
const int SR_ECHO_PIN = 18;
const int SB1_TRIGGER_PIN = 19;
const int SB1_ECHO_PIN = 20;
const int SB2_TRIGGER_PIN = 21;
const int SB2_ECHO_PIN = 22;

bool end = false;

void setup() {
  pinMode(ENUR, OUTPUT);
  pinMode(ENLR, OUTPUT);
  pinMode(ENUL, OUTPUT);
  pinMode(ENLL, OUTPUT);
  pinMode(MUR_F, OUTPUT);
  pinMode(MUR_B, OUTPUT);
  pinMode(MLR_F, OUTPUT);
  pinMode(MLR_B, OUTPUT);
  pinMode(MUL_F, OUTPUT);
  pinMode(MUL_B, OUTPUT);
  pinMode(MLL_F, OUTPUT);
  pinMode(MLL_B, OUTPUT);
  
  pinMode(SF1_TRIGGER_PIN, OUTPUT);
  pinMode(SF1_ECHO_PIN, INPUT);
  pinMode(SF2_TRIGGER_PIN, OUTPUT);
  pinMode(SF2_ECHO_PIN, INPUT);
  pinMode(SL_TRIGGER_PIN, OUTPUT);
  pinMode(SL_ECHO_PIN, INPUT);
  pinMode(SR_TRIGGER_PIN, OUTPUT);
  pinMode(SR_ECHO_PIN, INPUT);
  pinMode(SB1_TRIGGER_PIN, OUTPUT);
  pinMode(SB1_ECHO_PIN, INPUT);
  pinMode(SB2_TRIGGER_PIN, OUTPUT);
  pinMode(SB2_ECHO_PIN, INPUT);

  digitalWrite(MUR_F, HIGH);
  digitalWrite(MUR_B, LOW);
  digitalWrite(MUL_F, HIGH);
  digitalWrite(MUL_B, LOW);
  digitalWrite(MLL_F, HIGH);
  digitalWrite(MLL_B, LOW);
  digitalWrite(MLR_F, HIGH);
  digitalWrite(MLR_B, LOW);

  SG90.attach(3);
}

long duration;
long SF1_distance, SF2_distance, SL_distance, SR_distance, SB1_distance, SB2_distance;

void readDistance() {
  // SF1
  digitalWrite(SF1_TRIGGER_PIN, LOW);
  delay(5);
  digitalWrite(SF1_TRIGGER_PIN, HIGH);
  delay(5);
  digitalWrite(SF1_TRIGGER_PIN, LOW);
  delay(5);

  duration = pulseIn(SF1_ECHO_PIN, HIGH);
  SF1_distance = (duration/2)/29.1;

  // SF2
  digitalWrite(SF2_TRIGGER_PIN, LOW);
  delay(5);
  digitalWrite(SF2_TRIGGER_PIN, HIGH);
  delay(5);
  digitalWrite(SF2_TRIGGER_PIN, LOW);
  delay(5);

  duration = pulseIn(SF2_ECHO_PIN, HIGH);
  SF2_distance = (duration/2)/29.1;

  // SL
  digitalWrite(SL_TRIGGER_PIN, LOW);
  delay(5);
  digitalWrite(SL_TRIGGER_PIN, HIGH);
  delay(5);
  digitalWrite(SL_TRIGGER_PIN, LOW);
  delay(5);

  duration = pulseIn(SL_ECHO_PIN, HIGH);
  SL_distance = (duration/2)/29.1;

  // SR
  digitalWrite(SR_TRIGGER_PIN, LOW);
  delay(5);
  digitalWrite(SR_TRIGGER_PIN, HIGH);
  delay(5);
  digitalWrite(SR_TRIGGER_PIN, LOW);
  delay(5);

  duration = pulseIn(SR_ECHO_PIN, HIGH);
  SR_distance = (duration/2)/29.1;

  // SB1
  digitalWrite(SB1_TRIGGER_PIN, LOW);
  delay(5);
  digitalWrite(SB1_TRIGGER_PIN, HIGH);
  delay(5);
  digitalWrite(SB1_TRIGGER_PIN, LOW);
  delay(5);

  duration = pulseIn(SB1_ECHO_PIN, HIGH);
  SB1_distance = (duration/2)/29.1;

  // SB2
  digitalWrite(SB2_TRIGGER_PIN, LOW);
  delay(5);
  digitalWrite(SB2_TRIGGER_PIN, HIGH);
  delay(5);
  digitalWrite(SB2_TRIGGER_PIN, LOW);
  delay(5);

  duration = pulseIn(SB2_ECHO_PIN, HIGH);
  SB2_distance = (duration/2)/29.1;
}

void loop() {
  readDistance();
  if (SL_distance + SR_distance > 100) { 
    end = true;
  } 

  // move to the right side if left sensor detect an obstacle
  if (SF1_distance <= 50 && SF2_distance > 50 && !end) { 
    analogWrite(ENUL, 50);
    analogWrite(ENLL, 50);
    analogWrite(ENLR, 30);
    analogWrite(ENUR, 30);
    delay(1000); 

    // move to the left to make it go straight
    if (SF1_distance > 50) { 
      analogWrite(ENLR, 50);
      analogWrite(ENUR, 50);
      analogWrite(ENUL, 30);
      analogWrite(ENLL, 30);
      delay(100);
    }

    // go straight
    analogWrite(ENUL, 50);
    analogWrite(ENLL, 50);
    analogWrite(ENLR, 50);
    analogWrite(ENUR, 50);
    delay(50); 
  } 

  // move to the left side if right sensor detect an obstacle
  else if (SF2_distance <= 50 && SF1_distance > 50 && !end) {
    analogWrite(ENUR, 50);
    analogWrite(ENLR, 50);
    analogWrite(ENLL, 30);
    analogWrite(ENUL, 30);
    delay(1000); 

    // move to the right to make it go straight
    if (SF2_distance > 50) {
      analogWrite(ENLL, 50);
      analogWrite(ENUL, 50);
      analogWrite(ENUR, 30);
      analogWrite(ENLR, 30);
      delay(100);
    }

    // go straight
    analogWrite(ENUL, 50);
    analogWrite(ENLL, 50);
    analogWrite(ENLR, 30);
    analogWrite(ENUR, 30);
    delay(50); 
  }
  else {
    digitalWrite(MUL_F, HIGH);
    digitalWrite(MUL_B, LOW);
    analogWrite(ENUL, 50);
    digitalWrite(MLL_F, HIGH);
    digitalWrite(MLL_B, LOW);
    analogWrite(ENLL, 50);
    digitalWrite(MUR_F, HIGH);
    digitalWrite(MUR_B, LOW);
    analogWrite(ENUR, 50);
    digitalWrite(MLR_F, HIGH);
    digitalWrite(MLR_B, LOW);
    analogWrite(ENLR, 50);
  }

  if (end) {
    // turn left if left sensor detect the cylinder
    if (SL_distance <= 10 || SR_distance <= 10) {
      SG90.write(180);
    }
    if (SL_distance < SR_distance) {
      digitalWrite(MUR_F, HIGH);
      digitalWrite(MUR_B, LOW);
      digitalWrite(MLR_F, HIGH);
      digitalWrite(MLR_B, LOW);
      digitalWrite(MUL_F, LOW);
      digitalWrite(MUL_B, HIGH);
      digitalWrite(MLL_F, LOW);
      digitalWrite(MLL_B, HIGH);
      delay(1000);
    } 
    // turn right if right sensor detect the cylinder
    else if (SR_distance < SL_distance) {
      digitalWrite(MUR_B, HIGH);
      digitalWrite(MUR_F, LOW);
      digitalWrite(MLR_B, HIGH);
      digitalWrite(MLR_F, LOW);
      digitalWrite(MUL_B, LOW);
      digitalWrite(MUL_F, HIGH);
      digitalWrite(MLL_B, LOW);
      digitalWrite(MLL_F, HIGH);
      delay(1000); // adjust accordingly
    }
  }
}
