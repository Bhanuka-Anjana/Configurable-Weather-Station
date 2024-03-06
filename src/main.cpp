#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "FS.h"
#include "SPIFFS.h"

#define TRIGGER_PIN 0
#define LED_PIN 2

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

#define FieldLength 40 // add a custom input field

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

HTTPClient http;
int httpCode;
String payload;

char mylat[FieldLength];
char mylon[FieldLength];
char myappid[FieldLength];

unsigned long currentMillis;
unsigned long previousMillis;
unsigned long period = 5000;
bool ledState;

// WiFiManager, Glabal intialization.
WiFiManager wm;

String config_path = "/config.json";

void checkButton();
String getParam(String name);
void saveParamCallback();
void getCurrentWeatherData();
void readSPIFFS(String path);

void setup()
{
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  Serial.begin(115200);
  Serial.println("\n Starting");

  // pinMode(TRIGGER_PIN, INPUT);
  //  pinMode(LED_PIN, OUTPUT);

  // read configuration from FS json
  readSPIFFS(config_path);

  WiFiManagerParameter appid_field("appid", "Open-Weather API-KEY :", myappid, FieldLength, "bb2edd0d067b3b1052f6ce65a4908b05");
  WiFiManagerParameter longitude_field("lonid", "Your location(Longitude): ", mylon, FieldLength);
  WiFiManagerParameter latitude_field("latid", "Your location(Latitude): ", mylat, FieldLength);
  // test custom html input type(checkbox)
  // new (&custom_field) WiFiManagerParameter("customfieldid", "Custom Field Label", "Custom Field Value", customFieldLength,"placeholder=\"Custom Field Placeholder\" type=\"checkbox\"");
  // wm.addParameter(&custom_field);

  wm.addParameter(&appid_field);
  wm.addParameter(&latitude_field);
  wm.addParameter(&longitude_field);

  wm.setSaveParamsCallback(saveParamCallback);

  // custom menu via array or vector
  std::vector<const char *> menu = {"wifi", "info", "param", "sep", "restart", "exit"};
  wm.setMenu(menu);

  // set dark theme
  wm.setClass("invert");

  // wm.setConfigPortalTimeout(60); // auto close configportal after n seconds
  wm.setAPClientCheck(true); // avoid timeout if client connected to softap

  // reset settings - wipe stored credentials for testing
  // these are stored by the esp library
  // wm.resetSettings();

  // turning on the oled
  display.clearDisplay();
  display.setTextColor(WHITE, BLACK);
  display.setTextSize(1);
  display.setCursor(20, 0);
  display.println("Weather Station");
  display.setCursor(0, 14);
  display.println("Connect AutoConnectAP");
  display.setCursor(0, 24);
  display.println("password-password");
  display.display();
  delay(1000);

  bool res;
  // res = wm.autoConnect(); // auto generated AP name from chipid
  // res = wm.autoConnect("AutoConnectAP"); // anonymous ap
  res = wm.autoConnect("AutoConnectAP", "password"); // password protected ap

  if (!res)
  {
    Serial.println("Failed to connect");
    // ESP.restart();
  }
  else
  {
    // if you get here you have connected to the WiFi
    Serial.println("connected...yeey :)");
    display.clearDisplay();
    display.setTextColor(WHITE, BLACK);
    display.setTextSize(1);
    display.setCursor(20, 0);
    display.println("Weather Station");
    display.setCursor(0, 14);
    display.println("Connected to WiFi");
    display.display();
    delay(1000);
  }
}

void loop()
{
  currentMillis = millis();
  checkButton();
  getCurrentWeatherData();
}

// check button press
void checkButton()
{
  // check for button press
  if (digitalRead(TRIGGER_PIN) == LOW)
  {
    // poor mans debounce/press-hold, code not ideal for production
    delay(50);
    if (digitalRead(TRIGGER_PIN) == LOW)
    {
      Serial.println("Button Pressed");
      // still holding button for 3000 ms, reset settings, code not ideaa for production
      delay(3000); // reset delay hold
      if (digitalRead(TRIGGER_PIN) == LOW)
      {
        Serial.println("Button Held");
        Serial.println("Erasing Config, restarting");
        wm.resetSettings();
        ESP.restart();
      }

      // start portal w delay
      Serial.println("Starting config portal");
      wm.setConfigPortalTimeout(120);

      if (!wm.startConfigPortal("OnDemandAP", "password"))
      {
        Serial.println("failed to connect or hit timeout");
        delay(3000);
        // ESP.restart();
      }
      else
      {
        // if you get here you have connected to the WiFi
        Serial.println("connected...yeey :)");
      }
    }
  }
}

// read custom parameters
String getParam(String name)
{
  // read parameter from server, for customhmtl input
  String value;
  if (wm.server->hasArg(name))
  {
    value = wm.server->arg(name);
  }
  return value;
}

// callback to save custom parameters
void saveParamCallback()
{
  Serial.println("[CALLBACK] saveParamCallback fired");
  getParam("appid").toCharArray(myappid, FieldLength);
  getParam("latid").toCharArray(mylat, FieldLength);
  getParam("lonid").toCharArray(mylon, FieldLength);

  Serial.println("The values in the file are: ");
  Serial.println("\tapi key : " + String(myappid));
  Serial.println("\tlongitude : " + String(mylon));
  Serial.println("\tlatitude : " + String(mylat));

  // save the custom parameters to FS
  Serial.println("saving config");

  DynamicJsonBuffer jsonBuffer;
  JsonObject &json = jsonBuffer.createObject();

  json["apikey"] = myappid;
  json["longitude"] = mylon;
  json["latitude"] = mylat;

  Serial.println("mounted file system");
  File configFile = SPIFFS.open(config_path, "w");
  if (!configFile)
  {
    Serial.println("failed to open config file for writing");
  }

  json.printTo(Serial);
  json.printTo(configFile);
  configFile.close();
  Serial.println("done saving");
}

// get current weather data
void getCurrentWeatherData()
{
  if (currentMillis - previousMillis >= period)
  {
    previousMillis = currentMillis;
    String url = (String) "https://api.openweathermap.org/data/2.5/weather?lat=" + mylat + "&lon=" + mylon + "&appid=" + myappid;
    if (http.begin(url))
    {
      Serial.print("[HTTP] GET...\n");
      // start connection and send HTTP header
      httpCode = http.GET();
      // httpCode will be negative on error
      if (httpCode > 0)
      {
        payload = http.getString();

        DynamicJsonBuffer jsonBuffer(512);

        JsonObject &root = jsonBuffer.parseObject(payload);
        if (!root.success())
        {
          Serial.println(F("Paring failed"));
          return;
        }

        float temp = (float)(root["main"]["temp"]) - 273.15;
        int humidity = root["main"]["humidity"];
        float pressure = (float)(root["main"]["pressure"]) / 1000;
        float wind_speed = root["wind"]["speed"];
        int wind_degree = root["wind"]["deg"];
        String city = root["name"];

        Serial.printf("Temperature = %f°C\r\n", temp);
        Serial.printf("Humidiy = %d %%\r\n", humidity);
        Serial.printf("Pressure = %.3f bar \r\n", pressure);
        Serial.printf("Wind Speed = %.1f m/s\r\n", wind_speed);
        Serial.printf("Wind Degree = %d`\r\n\r\n", wind_degree);

        display.clearDisplay();
        display.setCursor(0, 0);
        display.print("Station - " + city);

        display.setCursor(0, 10);
        display.print("Temperature :");
        display.setCursor(77, 10);
        display.print(temp);
        display.print("C");

        display.setCursor(0, 20);
        display.print("Humidity    :");
        display.setCursor(77, 20);
        display.print(humidity);
        display.print("%");
        display.display();
        display.startscrollright(0x00, 0x00);
        delay(2000);

        display.clearDisplay();
        display.setCursor(0, 0);
        display.print("Station - " + city);

        display.setCursor(0, 10);
        display.print("Pressure    :");
        display.setCursor(77, 10);
        display.print(pressure, 3);
        display.print("bar");

        display.setCursor(0, 20);
        display.print("Wind speed  :");
        display.setCursor(77, 20);
        display.print(wind_speed, 1);
        display.print("m/s");
        display.display();
        display.startscrollright(0x00, 0x00);
        delay(2000);

        display.clearDisplay();
        display.setCursor(0, 0);
        display.print("Station - " + city);
        display.setCursor(0, 10);
        display.print("Wind degree :");
        display.setCursor(77, 10);
        display.println(wind_degree);
        display.display();
        display.startscrollright(0x00, 0x00);
        display.clearDisplay();
      }
      else
      {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }
      http.end();
    }
    else
    {
      Serial.println("[HTTP] Unable to connect");
      delay(500);
      display.print("*");
    }
  }
}

// read configuration from FS json
void readSPIFFS(String path)
{
  // clean FS, for testing
  // SPIFFS.format();

  if (SPIFFS.begin(true))
  {
    Serial.println("mounted file system");
    if (SPIFFS.exists(path))
    {
      // file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open(path, "r");
      if (configFile)
      {
        Serial.println("opened config file");
        size_t size = configFile.size();

        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);

        DynamicJsonBuffer jsonBuffer;
        JsonObject &json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);

        if (json.success())
        {
          Serial.println("\nparsed json");
          strcpy(myappid, json["apikey"]);
          strcpy(mylon, json["longitude"]);
          strcpy(mylat, json["latitude"]);
        }
        else
        {
          Serial.println("failed to load json config");
        }
        configFile.close();
      }
    }
  }
  else
  {
    Serial.println("failed to mount FS");
  }
}