// ===============================
// Rover Project Code
// ===============================

#include <Servo.h>   // For servo control (grabber)
#include <Wire.h>    // For IMU (MPU6050)
#include <SD.h>      // For SD card
#include <SPI.h>     // For SD card SPI communication
#include <MPU6050.h>

// --- Global Variables ---

const int chipSelect = 10;  // SD card CS pin
String missionData = "";    // Store sensor logs

MPU6050 imu;
float initialYaw = 0;

// Motor pins (now using PWM-capable pins)
const int motorLeftForward = 5;
const int motorLeftBackward = 3;
const int motorRightForward = 6;
const int motorRightBackward = 9;

// Ultrasonic sensor pins
const int trigPin = 7;
const int echoPin = 8;

// Servo pin
const int grabberServoPin = 4;
Servo grabberServo;

// --- Setup ---

void setup() {
  Serial.begin(9600);

  // Initialize motors
  pinMode(motorLeftForward, OUTPUT);
  pinMode(motorLeftBackward, OUTPUT);
  pinMode(motorRightForward, OUTPUT);
  pinMode(motorRightBackward, OUTPUT);

  // Initialize ultrasonic
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Initialize servo
  grabberServo.attach(grabberServoPin);

  // Initialize SD card
  if (!SD.begin(chipSelect)) {
    Serial.println("SD card initialization failed!");
  } else {
    Serial.println("SD card ready.");
  }

  // Initialize IMU
  Wire.begin();
  imu.initialize();
  if (!imu.testConnection()) {
    Serial.println("MPU6050 connection failed!");
  } else {
    Serial.println("MPU6050 connected.");
  }

  // Capture initial heading
  initialYaw = getYaw();
  Serial.print("Initial Yaw: ");
  Serial.println(initialYaw);

  Serial.println("Setup complete!");
}

// --- Main Mission Sequence ---

void loop() {
  navigateStraightaway();
  searchForTarget();
  grabTarget();
  goToDropZone();
  dropObject();
  endMission();
}

// --- Mission Functions ---

void navigateStraightaway() {
  Serial.println("Navigating straightaway...");
  while (true) {
    if (!obstacleDetected()) {
      moveForward();
    } else {
      stopMotors();
      int leftDistance = checkLeft();
      int rightDistance = checkRight();
      if (leftDistance > rightDistance) turnLeft();
      else turnRight();
    }
    delay(100);
  }
}

void searchForTarget() {
  Serial.println("Searching for target...");
  bool targetFound = false;
  int sweepCount = 0;
  while (!targetFound && sweepCount < 10) {
    moveForward();
    delay(1000);
    stopMotors();
    delay(500);
    if (tallObjectDetected()) {
      targetFound = true;
      break;
    }
    if (sweepCount % 2 == 0) turnLeftSlight();
    else turnRightSlight();
    sweepCount++;
  }
}

void grabTarget() {
  Serial.println("Centering to grab target...");
  float yawError = getYaw() - initialYaw;
  while (abs(yawError) > 5) {
    if (yawError > 0) turnLeftSlight();
    else turnRightSlight();
    delay(300);
    stopMotors();
    delay(200);
    yawError = getYaw() - initialYaw;
  }
  int distance = getDistance();
  while (distance > 6) {
    moveForwardSlow();
    delay(500);
    stopMotors();
    delay(200);
    distance = getDistance();
  }
  grabberServo.write(90);
  delay(1000);
}

void goToDropZone() {
  Serial.println("Heading to drop zone...");
  float targetHeading = initialYaw + 90; // Example, tune this at trial
  turnToHeading(targetHeading);
  moveForwardWithCorrectionTimed(3000); // Tune travel time
  stopMotors();
}

void dropObject() {
  Serial.println("Dropping object...");
  grabberServo.write(0);
  delay(1000);
  moveBackwardSlow();
  delay(1000);
  stopMotors();
}

void moveForwardWithCorrectionTimed(unsigned long duration_ms) {
  unsigned long startTime = millis();
  while (millis() - startTime < duration_ms) {
    moveForward(); // Reuses your existing heading-corrected move
    delay(100);    // Briefly wait to allow corrections to update
  }
}

void endMission() {
  Serial.println("Ending mission, saving data...");
  stopMotors();
  File dataFile = SD.open("mission.txt", FILE_WRITE);
  if (dataFile) {
    dataFile.println("Time(ms),Heading(degrees),Distance(cm)");
    dataFile.print(missionData);
    dataFile.close();
    Serial.println("Data saved to SD card.");
  } else {
    Serial.println("Error opening file!");
  }
}

// --- Movement and Sensor Helpers ---

void moveForward() {
  float currentYaw = getYaw();
  int distance = getDistance();
  unsigned long currentTime = millis();
  missionData += String(currentTime) + "," + String(currentYaw) + "," + String(distance) + "\n";

  const int baseSpeed = 150;
  const int correctionGain = 2;
  int leftMotorSpeed = baseSpeed;
  int rightMotorSpeed = baseSpeed;

  float yawError = currentYaw - initialYaw;
  if (yawError > 5) leftMotorSpeed -= correctionGain * yawError;
  else if (yawError < -5) rightMotorSpeed += correctionGain * yawError;

  analogWrite(motorLeftForward, constrain(leftMotorSpeed, 0, 255));
  digitalWrite(motorLeftBackward, LOW);
  analogWrite(motorRightForward, constrain(rightMotorSpeed, 0, 255));
  digitalWrite(motorRightBackward, LOW);
}

void stopMotors() {
  digitalWrite(motorLeftForward, LOW);
  digitalWrite(motorLeftBackward, LOW);
  digitalWrite(motorRightForward, LOW);
  digitalWrite(motorRightBackward, LOW);
}

void moveForwardSlow() {
  analogWrite(motorLeftForward, 100);
  digitalWrite(motorLeftBackward, LOW);
  analogWrite(motorRightForward, 100);
  digitalWrite(motorRightBackward, LOW);
}

void moveBackwardSlow() {
  analogWrite(motorLeftBackward, 100);
  digitalWrite(motorLeftForward, LOW);
  analogWrite(motorRightBackward, 100);
  digitalWrite(motorRightForward, LOW);
}

bool obstacleDetected() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH);
  float distance = duration * 0.034 / 2;
  return (distance < 20);
}

int getDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH);
  float distance = duration * 0.034 / 2;
  return (int) distance;
}

float getYaw() {
  int16_t ax, ay, az;
  int16_t gx, gy, gz;
  imu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  return gz / 131.0; // Approximate heading change
}

void turnLeft() {
  digitalWrite(motorLeftForward, LOW);
  digitalWrite(motorLeftBackward, HIGH);
  digitalWrite(motorRightForward, HIGH);
  digitalWrite(motorRightBackward, LOW);
  delay(400);
  stopMotors();
}

void turnRight() {
  digitalWrite(motorLeftForward, HIGH);
  digitalWrite(motorLeftBackward, LOW);
  digitalWrite(motorRightForward, LOW);
  digitalWrite(motorRightBackward, HIGH);
  delay(400);
  stopMotors();
}

void turnLeftSlight() {
  digitalWrite(motorLeftForward, LOW);
  digitalWrite(motorLeftBackward, HIGH);
  digitalWrite(motorRightForward, HIGH);
  digitalWrite(motorRightBackward, LOW);
  delay(300);
  stopMotors();
}

void turnRightSlight() {
  digitalWrite(motorLeftForward, HIGH);
  digitalWrite(motorLeftBackward, LOW);
  digitalWrite(motorRightForward, LOW);
  digitalWrite(motorRightBackward, HIGH);
  delay(300);
  stopMotors();
}

void turnToHeading(float targetYaw) {
  float yawError = targetYaw - getYaw();
  while (abs(yawError) > 5) {
    if (yawError > 0) turnRightSlight();
    else turnLeftSlight();
    delay(300);
    stopMotors();
    delay(200);
    yawError = targetYaw - getYaw();
  }
}

bool tallObjectDetected() {
  int distance = getDistance();
  return (distance > 6 && distance < 15);  // Example values, tune if needed
}

int checkLeft() {
  turnLeft();
  delay(300);
  stopMotors();
  delay(100);
  int distance = getDistance();
  turnRight();
  delay(300);
  stopMotors();
  return distance;
}

int checkRight() {
  turnRight();
  delay(300);
  stopMotors();
  delay(100);
  int distance = getDistance();
  turnLeft();
  delay(300);
  stopMotors();
  return distance;
}
