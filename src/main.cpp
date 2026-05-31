#define CUSTOM_SETTINGS
#define INCLUDE_GAMEPAD_MODULE
#include <DabbleESP32.h>

#define A_IN_1 5
#define A_IN_2 6
#define PWM_A 4
#define CHANNEL_A 0

#define B_IN_1 15
#define B_IN_2 16
#define PWM_B 7
#define CHANNEL_B 1

#define FREQ 1000
#define RESOLUTION 8
#define MAX_DUTY 255

#define MAX_SPEED_PERCENTAGE 0.8 // 1 is max (100%)
#define MAX_CONTROL_VALUE 7

const int set_control_value = MAX_CONTROL_VALUE / MAX_SPEED_PERCENTAGE;

int prev_speed = 0;
char* direction_name = "STOP";

void init_logs();

void adjust_speed(int speed);
void slow_braking(int speed);
void full_stop();

void move_forward(int speed);
void move_backward(int speed);
void turn_right(int speed);
void turn_left(int speed);

void setup() {
    init_logs();

    // Init output pins
    pinMode(A_IN_1, OUTPUT);
    pinMode(A_IN_2, OUTPUT);
    pinMode(B_IN_1, OUTPUT);
    pinMode(B_IN_2, OUTPUT);

    // PWM setup
    ledcSetup(CHANNEL_A, FREQ, RESOLUTION);
    ledcSetup(CHANNEL_B, FREQ, RESOLUTION);

    ledcAttachPin(PWM_A, CHANNEL_A);
    ledcAttachPin(PWM_B, CHANNEL_B);
    
    // Set bluetooth name of your device
    Dabble.begin("S3-Mobile-Pad"); 
}

void loop() {
  // Refresh data obtained from smartphone. Calling this function is mandatory in order to get data properly from your mobile.
  Dabble.processInput();

  int direction = GamePad.getAngle(); // 60-120 forward, 240-300 backward
  int radius = GamePad.getRadius(); // 0-7
  int speed = radius != 0 ? (MAX_DUTY / set_control_value) * radius : prev_speed;
  
  if (radius > 0) {
    if (direction >= 60 && direction <= 120) {
      move_forward(speed);
      direction_name= "FORWARD";
    }
  
    if (direction >= 150 && direction <= 210) {
      turn_right(speed);
      direction_name= "RIGHT";
    }
  
    if (direction >= 240 && direction <= 300) {
      move_backward(speed);
      direction_name= "BACKWARD";
    }
  
    if (direction >= 330 && direction <= 360 || direction >= 0 && direction <= 30) {
      turn_left(speed);
      direction_name= "LEFT";
    }
  } else if (prev_speed != 0) {
    slow_braking(speed);
    direction_name= "STOP";

    speed = 0;
  }

  Serial.print("Speed: ");
  Serial.print(speed);
  Serial.print(" | Direction: ");
  Serial.println(direction_name);

  prev_speed = speed;

  delay(20); 
}

void init_logs() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("Logs init");
}

void adjust_speed(int speed) {
  ledcWrite(CHANNEL_A, speed);
  ledcWrite(CHANNEL_B, speed);
}

void slow_braking(int speed) {
  for (int i = speed; i > 0; i -= 2) {
    adjust_speed(i);
    
    Serial.print("Speed: ");
    Serial.print(i);
    Serial.println(" | Direction: BRAKING");

    delay(10);
  }
}

void full_stop() {
  ledcWrite(CHANNEL_A, 0);
  ledcWrite(CHANNEL_B, 0);
}

void move_forward(int speed) {
  digitalWrite(A_IN_1, HIGH);
  digitalWrite(A_IN_2, LOW);

  digitalWrite(B_IN_1, HIGH);
  digitalWrite(B_IN_2, LOW);

  adjust_speed(speed);
}

void move_backward(int speed) {
  digitalWrite(A_IN_1, LOW);
  digitalWrite(A_IN_2, HIGH);

  digitalWrite(B_IN_1, LOW);
  digitalWrite(B_IN_2, HIGH);

  adjust_speed(speed);
}

void turn_right(int speed) {
  digitalWrite(A_IN_1, HIGH);
  digitalWrite(A_IN_2, LOW);

  digitalWrite(B_IN_1, LOW);
  digitalWrite(B_IN_2, HIGH);

  adjust_speed(speed);
}

void turn_left(int speed) {
  digitalWrite(A_IN_1, LOW);
  digitalWrite(A_IN_2, HIGH);

  digitalWrite(B_IN_1, HIGH);
  digitalWrite(B_IN_2, LOW);

  adjust_speed(speed);
}