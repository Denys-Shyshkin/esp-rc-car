#define CUSTOM_SETTINGS
#define INCLUDE_GAMEPAD_MODULE
#include <DabbleESP32.h>
#include <WiFi.h>
#include "secrets.h"

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASS;

WiFiServer telnetServer(23);
WiFiClient telnetClient;

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

void init_serial();
void init_telnet_server();

void telnet_server();
void logging(const char* fmt, ...);
void adjust_speed(int speed);
void slow_braking(int speed);
void full_stop();

void move_forward();
void move_backward();
void turn_right();
void turn_left();

void setup() {
  init_serial();
  init_telnet_server();

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
  telnet_server();
  
  // Refresh data obtained from smartphone. Calling this function is mandatory in order to get data properly from your mobile.
  Dabble.processInput();

  int direction = GamePad.getAngle(); // 60-120 forward, 240-300 backward
  int radius = GamePad.getRadius(); // 0-7
  int speed = radius != 0 ? (MAX_DUTY / set_control_value) * radius : prev_speed;
  
  if (radius > 0) {
    if (direction >= 60 && direction <= 120) {
      move_forward();
    }
  
    if (direction >= 150 && direction <= 210) {
      turn_left();
    }
    
    if (direction >= 240 && direction <= 300) {
      move_backward();
    }
    
    if (direction >= 330 && direction <= 360 || direction >= 0 && direction <= 30) {
      turn_right();
    }
  } else if (prev_speed != 0) {
    slow_braking(speed);

    speed = 0;
  }

  adjust_speed(speed);

  logging("Direction: %d", direction);

  prev_speed = speed;
}

void init_serial() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("Logs init");
}

void init_telnet_server() {
  WiFi.begin(ssid, password);

  Serial.print("Connecting");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP()); // 192.168.0.188

  telnetServer.begin();
  telnetServer.setNoDelay(true);
}

void telnet_server() {
  if (telnetServer.hasClient()) {
    if (!telnetClient || !telnetClient.connected()) {
      if (telnetClient) {
        telnetClient.stop();
      } else {
        telnetClient = telnetServer.available();
        telnetClient.println("Connected to ESP32");
      }
    }
    else {
      telnetServer.available().stop();
    }
  }
}

void logging(const char* fmt, ...) {
  char buffer[128];

  va_list args;
  va_start(args, fmt);
  vsnprintf(buffer, sizeof(buffer), fmt, args);
  va_end(args);

  Serial.println(buffer);

  if (telnetClient && telnetClient.connected()) {
    telnetClient.println(buffer);
  }
}

void adjust_speed(int speed) {
  ledcWrite(CHANNEL_A, speed);
  ledcWrite(CHANNEL_B, speed);
}

void slow_braking(int speed) {
  for (int i = speed; i > 0; i -= 2) {
    adjust_speed(i);

    delay(10);
  }
}

void full_stop() {
  ledcWrite(CHANNEL_A, 0);
  ledcWrite(CHANNEL_B, 0);
}

void move_forward() {
  digitalWrite(A_IN_1, LOW);
  digitalWrite(A_IN_2, HIGH);

  digitalWrite(B_IN_1, LOW);
  digitalWrite(B_IN_2, HIGH);
}

void move_backward() {
  digitalWrite(A_IN_1, HIGH);
  digitalWrite(A_IN_2, LOW);

  digitalWrite(B_IN_1, HIGH);
  digitalWrite(B_IN_2, LOW);
}

void turn_right() {
  digitalWrite(A_IN_1, LOW);
  digitalWrite(A_IN_2, HIGH);

  digitalWrite(B_IN_1, HIGH);
  digitalWrite(B_IN_2, LOW);
}

void turn_left() {
  digitalWrite(A_IN_1, HIGH);
  digitalWrite(A_IN_2, LOW);

  digitalWrite(B_IN_1, LOW);
  digitalWrite(B_IN_2, HIGH);
}