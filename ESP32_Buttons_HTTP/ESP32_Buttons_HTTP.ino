#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "Quizzical";
const char* password = "123456789";
String serverName = "http://192.168.4.22/button";

#define DEBOUNCE_TIME  50 // the debounce time in millisecond, increase this time if it still chatters
#define NUM_BUTTONS 5 // number of buttons

// 1 - 4 push buttons, 5 - toggle button
const byte buttonPins[] = { 27, 26, 33, 25, 4};

// Keep track of which buttons were pressed last frame
bool lastButtonState[NUM_BUTTONS];
int lastFlickerableState[NUM_BUTTONS];  // the previous flickerable state from the input pin

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime[NUM_BUTTONS];  // the last time the output pin was toggled


void setup() {
  Serial.begin(115200);

  delay(1000);

  WiFi.mode(WIFI_STA); //Optional
  WiFi.begin(ssid, password);
  Serial.println("\nConnecting");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }

  Serial.println("\nConnected to the WiFi network");
  Serial.print("Local ESP32 IP: ");
  Serial.println(WiFi.localIP());

  for (int i = 0; i < NUM_BUTTONS; i++)
  {
    // Buttons
    pinMode(buttonPins[i], INPUT_PULLUP); // config input pin and enable the internal pull-up resistor
    lastButtonState[i] = HIGH;
    lastFlickerableState[i] = HIGH;
    lastDebounceTime[i] = 0;
  }
}


void loop() {

  for (int i = 0; i < NUM_BUTTONS; i++) {
    int buttonState = digitalRead(buttonPins[i]);
    if (buttonState != lastFlickerableState[i]) {
      lastDebounceTime[i] = millis();
      lastFlickerableState[i] = buttonState;
    }
    if ((millis() - lastDebounceTime[i]) > DEBOUNCE_TIME) {
      if (lastButtonState[i] == HIGH && buttonState == LOW) {
        Serial.print("Button ");
        Serial.print(i);
        Serial.println(" pressed");

        //Check WiFi connection status
        if (WiFi.status() == WL_CONNECTED) {
          HTTPClient http;

          // Your Domain name with URL path or IP address with path
          http.begin(serverName);
          http.addHeader("Content-Type", "application/x-www-form-urlencoded");

          String httpRequestData = "button=" + String(i);
          int httpResponseCode = http.POST(httpRequestData);
          Serial.print("HTTP Response code: ");
          Serial.println(httpResponseCode);
          // Free resources
          http.end();
        }
      } else if (lastButtonState[i] == LOW && buttonState == HIGH) {
        Serial.print("Button ");
        Serial.print(i);
        Serial.println(" released");
      }
      lastButtonState[i] = buttonState;
    }
  }
}
