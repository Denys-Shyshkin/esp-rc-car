#define CUSTOM_SETTINGS
#define INCLUDE_GAMEPAD_MODULE
#include "secrets.h"
#include <DabbleESP32.h>
#include <NewPing.h>
#include <WiFi.h>
#include <ESP32Servo.h>

const char *ssid = WIFI_SSID;
const char *password = WIFI_PASS;

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

#define TRIGGER 14
#define ECHO 13

#define FREQ 1000
#define RESOLUTION 8
#define MAX_DUTY 255

#define MAX_SPEED_PERCENTAGE 0.8 // 1 is max (100%)
#define MAX_CONTROL_VALUE 7

#define SERVO_PIN 17

const int set_control_value = MAX_CONTROL_VALUE / MAX_SPEED_PERCENTAGE;

int startup_notes[4] = {330, 440, 554, 659};  // E4, A4, C#5, E5
int note_durations[4] = {150, 150, 150, 300}; // Duration in milliseconds

int direction = 0;
int radius = 0;
int speed = 0;

uint32_t last_trigger = 0;
uint16_t trigger_delay = 50;
int distance_to_speed_multiplier = 3;
bool is_obstacle_detected = false;

int servo_pos = 90;
int current_servo_pos = servo_pos;

NewPing sonar(TRIGGER, ECHO);

Servo my_servo;

void init_serial();
void init_telnet_server();

void telnet_server();
void logging(const char *fmt, ...);
void adjust_speed(int speed_a, int speed_b);
void full_stop();
void play_startup_tone();
void obstacle_detection();
void inputs_reading();
void movement_control();
void servo_control();

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

    // Init servo signal pin
    my_servo.attach(SERVO_PIN);
    my_servo.write(current_servo_pos);

    // PWM setup
    ledcSetup(CHANNEL_A, FREQ, RESOLUTION);
    ledcSetup(CHANNEL_B, FREQ, RESOLUTION);

    ledcAttachPin(PWM_A, CHANNEL_A);
    ledcAttachPin(PWM_B, CHANNEL_B);

    // Set bluetooth name of your device
    Dabble.begin("S3-Mobile-Pad");

    play_startup_tone();
}

void loop() {
    telnet_server();

    inputs_reading();
    obstacle_detection();
    movement_control();

    servo_control();

    logging("Servo pos: %d", servo_pos);
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
    // on macos terminal commnand
    // nc {local ip address} {port}
    // e.g. nc 192.168.0.188 23

    if (telnetServer.hasClient()) {
        if (!telnetClient || !telnetClient.connected()) {
            if (telnetClient) {
                telnetClient.stop();
            } else {
                telnetClient = telnetServer.available();
                telnetClient.println("Connected to ESP32");
            }
        } else {
            telnetServer.available().stop();
        }
    }
}

void logging(const char *fmt, ...) {
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

void adjust_speed(int speed_a, int speed_b) {
    ledcWrite(CHANNEL_A, speed_a);
    ledcWrite(CHANNEL_B, speed_b);
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
    digitalWrite(A_IN_1, HIGH);
    digitalWrite(A_IN_2, LOW);

    digitalWrite(B_IN_1, LOW);
    digitalWrite(B_IN_2, HIGH);
}

void turn_left() {
    digitalWrite(A_IN_1, LOW);
    digitalWrite(A_IN_2, HIGH);

    digitalWrite(B_IN_1, HIGH);
    digitalWrite(B_IN_2, LOW);
}

void play_startup_tone() {
    int volume = 50;

    digitalWrite(A_IN_1, HIGH);
    digitalWrite(A_IN_2, LOW);

    digitalWrite(B_IN_1, HIGH);
    digitalWrite(B_IN_2, LOW);

    for (int i = 0; i < 4; i++) {
        ledcWriteTone(CHANNEL_A, startup_notes[i]);
        ledcWriteTone(CHANNEL_B, startup_notes[i]);

        ledcWrite(CHANNEL_A, volume);
        ledcWrite(CHANNEL_B, volume);

        delay(note_durations[i]);

        ledcWrite(CHANNEL_A, 0);
        ledcWrite(CHANNEL_B, 0);

        delay(30);
    }

    full_stop();
    delay(50);

    ledcSetup(CHANNEL_A, FREQ, 8);
    ledcSetup(CHANNEL_B, FREQ, 8);
}

void obstacle_detection() {
    uint32_t now = millis();

    if (now - last_trigger >= trigger_delay) {
        if (direction >= 20 && direction < 160) {
            last_trigger = now;

            unsigned long distance = sonar.ping_cm();
            is_obstacle_detected =
                distance > 0 && distance <= radius * distance_to_speed_multiplier;

            logging("Obstacle distance = %d cm", distance);
            logging("Is obstacle detected: %s", is_obstacle_detected ? "TRUE" : "FALSE");
        } else {
            is_obstacle_detected = false;
        }
    }

    if (is_obstacle_detected) {
        speed = 0;
    }
}

void inputs_reading() {
    // Refresh data obtained from smartphone. Calling this function is mandatory in order to get
    // data properly from your mobile.
    Dabble.processInput();

    direction = GamePad.getAngle(); // 0-360
    radius = GamePad.getRadius();   // 0-7
    speed = (MAX_DUTY / set_control_value) * radius;

    if (GamePad.isTrianglePressed()) {
        servo_pos = 90;
    }
    if (GamePad.isSquarePressed()) {
        servo_pos = 180;
    }
    if (GamePad.isCirclePressed()) {
        servo_pos = 0;
    }
}

void movement_control() {
    if (direction >= 20 && direction < 70) {
        move_forward();
        adjust_speed(speed / 2, speed);
    }

    if (direction >= 70 && direction < 110) {
        move_forward();
        adjust_speed(speed, speed);
    }

    if (direction >= 110 && direction < 160) {
        move_forward();
        adjust_speed(speed, speed / 2);
    }

    if (direction >= 160 && direction < 200) {
        turn_left();
        adjust_speed(speed, speed);
    }

    if (direction >= 200 && direction < 250) {
        move_backward();
        adjust_speed(speed, speed / 2);
    }

    if (direction >= 250 && direction < 290) {
        move_backward();
        adjust_speed(speed, speed);
    }

    if (direction >= 290 && direction < 340) {
        move_backward();
        adjust_speed(speed / 2, speed);
    }

    if (direction > 340 && direction <= 360 || direction >= 0 && direction < 20) {
        turn_right();
        adjust_speed(speed, speed);
    }
}

void servo_control() {
    if (current_servo_pos != servo_pos) {
        current_servo_pos = servo_pos;

        my_servo.write(current_servo_pos);
    }
}