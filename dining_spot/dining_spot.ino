#include <Arduino_JSON.h>
#include <WiFi.h>
#include <HTTPClient.h>

#include "private.h"
//const char* ssid = "********";
//const char* keyphrase = "********";
//const char* authorization = "********"
//const char* device_id = "********";

#define CHARLEN 100

hw_timer_t * timer = NULL;

const char* base_url = "https://api.switch-bot.com/v1.0";
const int button = 4;

int state;
int flag = 1;

void IRAM_ATTR on_timer() {
  Serial.println("on_timer");
  flag = 1;
}

void setup() {
  Serial.begin(9600);
  
  WiFi.begin(ssid, keyphrase);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
  }

  pinMode(button, INPUT_PULLUP);

  timer = timerBegin(0, 8000, true);
  timerAttachInterrupt(timer, &on_timer, true);
  timerAlarmWrite(timer, 6000000, true);
  timerAlarmEnable(timer);
}

void check() {
  if (flag) {
    flag = 0;
    state = get_state();
  }
}

void loop() {
  int count;
  count = wait_for_press(button);  
  if (count >= 10) {
    toggle();
  }
  wait_for_release(button);
}

void toggle() {
  if (state)
    set_state(0);
  else
    set_state(1);
}

void set_state(int st) {
  HTTPClient http;
  char url[CHARLEN];
  sprintf(url, "%s/devices/%s/commands", base_url, device_id);
  http.begin(url);
  http.addHeader("Authorization", authorization);
  char message[CHARLEN];
  if (st)
    sprintf(message, "{\"command\": \"turnOn\"}");
  else
    sprintf(message, "{\"command\": \"turnOff\"}");
  int status_code = http.POST(message);
  state = st;
  timerWrite(timer, 0);
}

int get_state() {
  Serial.println("get");
  HTTPClient http;
  char url[CHARLEN];
  sprintf(url, "%s/devices/%s/status", base_url, device_id);
  http.begin(url);
  http.addHeader("Authorization", authorization);
  int status_code = http.GET();
  String res = http.getString().c_str();
  JSONVar obj = JSON.parse(res);
  const char* power = obj["body"]["power"];
  if (strcmp(power, "on") == 0)
    return 1;
  else
    return 0;
}

int is_pressed(int pin) {
  if (digitalRead(pin) == HIGH)
    return 0;
  else
    return 1;
}

int wait_for_press(int pin) {
  int count = 0;
  while (!is_pressed(pin)) {
    check();
    count += 1;
    delay(1);
  }
  return count;
}

int wait_for_release(int pin) {
  int count = 0;
  while (is_pressed(pin)) {
    check();
    count += 1;
    delay(1);
  }
  return count;
}
