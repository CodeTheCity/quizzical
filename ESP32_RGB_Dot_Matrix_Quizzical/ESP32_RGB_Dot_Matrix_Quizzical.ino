// This is how many color levels the display shows - the more the slower the update
//#define PxMATRIX_COLOR_DEPTH 8

// Defines the buffer height / the maximum height of the matrix
//#define PxMATRIX_MAX_HEIGHT 64

// Defines the buffer width / the maximum width of the matrix
//#define PxMATRIX_MAX_WIDTH 64

// Defines how long we display things by default
//#define PxMATRIX_DEFAULT_SHOWTIME 30

// Defines the speed of the SPI bus (reducing this may help if you experience noisy images)
//#define PxMATRIX_SPI_FREQUENCY 20000000

// Creates a second buffer for backround drawing (doubles the required RAM)
#define PxMATRIX_double_buffer true

#include <PxMatrix.h>
#include <ESP32Time.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "Question.h"

// WiFi and Webserver
const char* ssid     = "Quizzical";
const char* password = "123456789";

AsyncWebServer server(80);

// Variable to store the HTTP request
String header;

TaskHandle_t Task1;
TaskHandle_t Task2;

#define ESP32

// Pins for LED MATRIX
#ifdef ESP32

#define P_LAT 22
#define P_A 19
#define P_B 23
#define P_C 18
#define P_D 5
#define P_E 15
#define P_OE 16
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

#endif

#ifdef ESP8266

#include <Ticker.h>
Ticker display_ticker;
#define P_LAT 16
#define P_A 5
#define P_B 4
#define P_C 15
#define P_D 12
#define P_E 0
#define P_OE 2

#endif

#define matrix_width 64
#define matrix_height 32

// This defines the 'on' time of the display is us. The larger this number,
// the brighter the display. If too large the ESP will crash
uint8_t display_draw_time = 60; //30-70 is usually fine

//PxMATRIX display(32,16,P_LAT, P_OE,P_A,P_B,P_C);
PxMATRIX display(matrix_width, matrix_height, P_LAT, P_OE, P_A, P_B, P_C, P_D);
//PxMATRIX display(64,64,P_LAT, P_OE,P_A,P_B,P_C,P_D,P_E);

// Some standard colors
uint16_t myRED = display.color565(255, 0, 0);
uint16_t myGREEN = display.color565(0, 255, 0);
uint16_t myGREEN50 = display.color565(0, 255, 0);
uint16_t myBLUE = display.color565(0, 0, 255);
uint16_t myWHITE = display.color565(255, 255, 255);
uint16_t myYELLOW = display.color565(255, 255, 0);
uint16_t myCYAN = display.color565(0, 255, 255);
uint16_t myMAGENTA = display.color565(255, 0, 255);
uint16_t myBLACK = display.color565(0, 0, 0);

uint16_t myCOLORS[8] = {myRED, myGREEN, myBLUE, myWHITE, myYELLOW, myCYAN, myMAGENTA, myBLACK};

char row[4][64];
bool scrolling[4] = { false, false, false, false };
uint8_t mSelected = 5; // set to row beyond the end of the display
uint8_t mCurrentQuestion = 0;
Question* mQuestion = NULL;

Question questions[] = {
  Question( "How much is that doggie in the window?", "not for sale", "One million dollars", "free"),
  Question( "When was the first CTC?", "2014", "1875", "2020"),
  Question( "Ohm's Law is", "V = IR", "R = VI", "I = VR")
};

#define numQuestions (sizeof(questions)/sizeof(Question)) //array size

IPAddress local_IP(192, 168, 4, 22);
IPAddress gateway(192, 168, 4, 9);
IPAddress subnet(255, 255, 255, 0);

#ifdef ESP8266
// ISR for display refresh
void display_updater()
{
  display.display(display_draw_time);
}
#endif

#ifdef ESP32
void IRAM_ATTR display_updater() {
  // Increment the counter and set the time of ISR
  portENTER_CRITICAL_ISR(&timerMux);
  display.display(display_draw_time);
  portEXIT_CRITICAL_ISR(&timerMux);
}
#endif


void display_update_enable(bool is_enable)
{

#ifdef ESP8266
  if (is_enable)
    display_ticker.attach(0.004, display_updater);
  else
    display_ticker.detach();
#endif

#ifdef ESP32
  if (is_enable)
  {
    timer = timerBegin(0, 80, true);
    timerAttachInterrupt(timer, &display_updater, true);
    timerAlarmWrite(timer, 4000, true);
    timerAlarmEnable(timer);
  }
  else
  {
    timerDetachInterrupt(timer);
    timerAlarmDisable(timer);
  }
#endif
}



void setup() {
  Serial.begin(115200);

  WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  char stringIPAddress[32];
  sprintf(stringIPAddress, "%d:%d:%d:%d", IP[0], IP[1], IP[2], IP[3]);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    Serial.println("/ HTTP_GET");
    // Uncomment so that this is displayed on row 3 of the display for diagnostics
    //sprintf(row[2], "/ HTTP_GET");
    request->send_P(200, "text/html", "hello");
  });

  server.on("/button", HTTP_POST, [](AsyncWebServerRequest * request) {
    Serial.println("/button HTTP_POST");
    // Uncomment so that this is displayed on row 3 of the display for diagnostics
    //sprintf(row[2], "/button HTTP_POST");

    if (request->hasParam("button", true)) {
      uint8_t value = request->getParam("button", true)->value().toInt();
      Serial.print(value);

      // Use button 4 as skip to next question
      if (value == 4) {
        mSelected = 4;
        mCurrentQuestion = (mCurrentQuestion + 1) % numQuestions;
        askQuestion(mCurrentQuestion);
      } else {
        mSelected = value + 1;
        Serial.print("Selected ");
        Serial.println(mSelected);
      }
    }

    request->send_P(200, "text/html", "OK");
  });

  server.begin();


  // Define your display layout here, e.g. 1/8 step, and optional SPI pins begin(row_pattern, CLK, MOSI, MISO, SS)
  display.begin(16);
  //display.begin(8, 14, 13, 12, 4);

  // Define multiplex implemention here {BINARY, STRAIGHT} (default is BINARY)
  //display.setMuxPattern(BINARY);

  // Set the multiplex pattern {LINE, ZIGZAG,ZZAGG, ZAGGIZ, WZAGZIG, VZAG, ZAGZIG} (default is LINE)
  //display.setScanPattern(LINE);

  // Rotate display
  //display.setRotate(true);

  // Flip display
  //display.setFlip(true);

  // Control the minimum color values that result in an active pixel
  //display.setColorOffset(5, 5,5);

  // Set the multiplex implemention {BINARY, STRAIGHT} (default is BINARY)
  //display.setMuxPattern(BINARY);

  // Set the color order {RRGGBB, RRBBGG, GGRRBB, GGBBRR, BBRRGG, BBGGRR} (default is RRGGBB)
  //display.setColorOrder(RRGGBB);

  // Set the time in microseconds that we pause after selecting each mux channel
  // (May help if some rows are missing / the mux chip is too slow)
  //display.setMuxDelay(0,1,0,0,0);

  // Set the number of panels that make up the display area width (default is 1)
  //display.setPanelsWidth(2);

  // Set the brightness of the panels (default is 255)
  //display.setBrightness(50);

  // Set driver chip type
  //display.setDriverChip(FM6124);

  display.clearDisplay();
  display.setTextColor(myCYAN);
  display.setCursor(2, 0);
  display.print("Quizzical");
  display.setTextColor(myMAGENTA);
  display.setCursor(2, 8);
  display.print("a Fun");
  display.setTextColor(myBLUE);
  display.setCursor(2, 16);
  display.print("Educational");
  display.setTextColor(myYELLOW);
  display.setCursor(2, 24);
  display.print("Quiz");
  display_update_enable(true);

  display.showBuffer();

  askQuestion(mCurrentQuestion);

  //create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
    TaskUpdateDisplay,   /* Task function. */
    "Task1",     /* name of task. */
    10000,       /* Stack size of task */
    NULL,        /* parameter of the task */
    1,           /* priority of the task */
    &Task1,      /* Task handle to keep track of created task */
    0);          /* pin task to core 0 */

  delay(1000);
}
union single_double {
  uint8_t two[2];
  uint16_t one;
} this_single_double;


unsigned long last_draw = 0;
int scroll_xpos[4] = { matrix_width, matrix_width, matrix_width, matrix_width };

// General text scrolling with each row a differnt piece of text
// Not currently used but kept for future use
void scroll_text(unsigned long scroll_delay, uint8_t colorR, uint8_t colorG, uint8_t colorB)
{
  uint16_t text_length = 0;
  display.setTextWrap(false);  // we don't wrap text so it scrolls nicely
  display.setTextSize(1);
  display.setRotation(0);
  display.setTextColor(display.color565(colorR, colorG, colorB));
  display.clearDisplay();

  for (int i = 0; i < 4; i++) {
    text_length = strlen(row[i]) + 1;
    if (scrolling[i]) {
      scroll_xpos[i]--;
      if (scroll_xpos[i] < -(matrix_width + text_length * 5)) {
        scroll_xpos[i] = matrix_width;
      }
    } else {
      scroll_xpos[i] = 2;
    }

    display.setTextColor(myMAGENTA);
    display.setCursor(scroll_xpos[i], i * 8);
    display.print(row[i]);
  }

  delay(scroll_delay);
  yield();

  delay(scroll_delay / 5);
  yield();
}

// Special version of scroll_text that shows fixed options at start of scrolling
void display_question(unsigned long scroll_delay, uint8_t colorR, uint8_t colorG, uint8_t colorB)
{
  uint16_t text_length = 0;
  display.setTextWrap(false);  // we don't wrap text so it scrolls nicely
  display.setTextSize(1);
  display.setRotation(0);
  display.setTextColor(display.color565(colorR, colorG, colorB));
  display.clearDisplay();

  // Hiighlight the row that has been selected
  // will display out of the bounds of the display if mSelected is set to 5 or above
  for (int i = 0; i < 8; i++) {
    display.drawLine(0, (mSelected * 8) + i, matrix_width, (mSelected * 8) + i, mQuestion->isCorrect(mSelected) ? myGREEN : myRED);
  }

  for (int i = 0; i < 4; i++) {
    text_length = strlen(row[i]) + 1;
    if (scrolling[i]) {
      scroll_xpos[i]--;
      if (scroll_xpos[i] < -(matrix_width + text_length * 5)) {
        scroll_xpos[i] = matrix_width;
      }
    } else {
      scroll_xpos[i] = 0;
    }

    display.setTextColor(myYELLOW);
    display.setCursor(scroll_xpos[i] + 6, i * 8);
    display.print(row[i]);
  }

  // Blank the first 6 pixels to hide any scrolling text
  for (int i = 0; i < 6; i++) {
    display.drawLine(i, 0, i, 32, display.color565(0, 8, 0));
  }

  display.setTextColor(myMAGENTA);
  display.setCursor(0, 0);
  display.print("Q");

  for (int i = 1; i < 4; i++) {
    display.setTextColor(myMAGENTA);
    display.setCursor(0, i * 8);
    display.print(i);
  }

  // Display a line under the question
  display.drawLine(0, 7, matrix_width, 7, display.color565(128, 128, 128));

  display.showBuffer();

  delay(scroll_delay);
  yield();
}


void TaskUpdateDisplay( void * pvParameters ) {

  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());

  for (;;) {

    if (mQuestion != NULL) {
      display_question(50, 96, 96, 250);
    }
  }
}


void askQuestion(int question) {

  mQuestion = &questions[question];

  mQuestion->randomise();

  for (int i = 0; i < 4; i++)
  {
    scrolling[i] = false;
    strcpy(row[i], "");
  }

  strcpy(row[0], mQuestion->getQuestion());
  delay(1000);
  strcpy(row[1], mQuestion->getOption(0));
  delay(1000);
  strcpy(row[2], mQuestion->getOption(1));
  delay(1000);
  strcpy(row[3], mQuestion->getOption(2));

  for (int i = 0; i < 4; i++)
  {
    scrolling[i] = strlen(row[i]) > 9;
  }
}

void loop() {

}
