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

void init_logs();
void adjust_speed();
void full_stop();
void move_forward();
void move_backward();

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

  if (GamePad.isUpPressed()) {
    move_forward();
    Serial.println("Forward");
  }

  if (GamePad.isDownPressed()) {
    move_backward();
    Serial.println("Rear");
  }

  delay(20); 
}

void init_logs() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("Logs init");
}

void adjust_speed() {
  ledcWrite(CHANNEL_A, 255);
  ledcWrite(CHANNEL_B, 255);
}

void full_stop() {
  ledcWrite(CHANNEL_A, 0);
  ledcWrite(CHANNEL_B, 0);
}

void move_forward() {
  digitalWrite(A_IN_1, HIGH);
  digitalWrite(A_IN_2, LOW);

  digitalWrite(B_IN_1, HIGH);
  digitalWrite(B_IN_2, LOW);

  adjust_speed();
}

void move_backward() {
  digitalWrite(A_IN_1, LOW);
  digitalWrite(A_IN_2, HIGH);

  digitalWrite(B_IN_1, LOW);
  digitalWrite(B_IN_2, HIGH);

  adjust_speed();
}