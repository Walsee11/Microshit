#include <WiFi.h>
#include "time.h"
#include <ESP_Google_Sheet_Client.h>
#include <GS_SDHelper.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>

#define WIFI_SSID "THD"
#define WIFI_PASSWORD "123456789"

// Khai bao man hinh
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Google project ID
#define PROJECT_ID "iot-datalogging01"

// Service account's client email
#define CLIENT_EMAIL "iot-datalogging@iot-datalogging01.iam.gserviceaccount.com"

// Service account's private key
const char PRIVATE_KEY[] PROGMEM = "-----BEGIN PRIVATE KEY-----\nMIIEvQIBADANBgkqhkiG9w0BAQEFAASCBKcwggSjAgEAAoIBAQDjuNvLvtXg7KAx\ntoutW0NDtBV7z8E10z8aqX8EsP39Yzwxl22eQh1cQ7L1kW8/aKWEFp9FWqBaLcTm\n1fpJ/i8jcc0OTBbNkzgNlgC2ubc6I+GwDptuS1wzf3aFxhLGaGHMSSLhvrHazODe\npSexf60Lm2yrposdYil3w6N1JUzCPUdNmSbqdfCmdAi3ifezZX10tNj2nhDjlhss\no0paVTZTzOiU5qiN596+0oAC1p8GDoWStKHKakCy3lI33kTjsb7lqDbL/ZaKlj/i\nKiy62dTKT/L3V38foK1ULtvau3c+ZxzLOd+4l8452+8djw1IgFQ8nOUa6/Wo0M6R\nXpuGrpnlAgMBAAECggEACoEJanGIDhL8JK7l7H4sMSBKaWag4+m1nHY7DK1ZWenM\n6gCxJr6n+gyJYbhuqD8ce2ZLvHg8s0wvK/Ks0+Lsq7bhHYq29o6W5FGKAOO6Ju1m\nyKcz1jeVri5v+iWz0gEYmcK54C3aqFwQ91Qo65AXHFT0A0rwWc+gWAznKk0pKEBz\nEbPMO2r3N04EVdhTPE26qwYT8aUtfSMB/obE5T2h/7Upep4vp9JfC1vTHGDbMTzt\nu/8eQn641qNtH4/TBVVPc1BliZ3r/Z4OOCcRrpEX2vH5T5pmwmCgnyyhIKMHThhO\nIxa5RfL3yKdnp92oum9v8ZZvkfei4zDlSNLPkRFQ9QKBgQD7Q4a70nULEx4x9Y/U\naJg8bQ3hgp/jzWijgK8Ek5oXGUUEC7XSpfFiMQk6Vxx2Pch7CCm9L9PCka0v2RxX\nXLMW82Ybp9YkEyrYkSQWiOsarv9Uxf3+9x7iRkrF510/9t8CYPv0LMRhJxBP+t88\nqW9cB3fURV02CC+e/39LPlYB2wKBgQDoA7teX3XKtTBwYufW2pks+w5IsEcT6KkC\nrX+geNyNSgOvX3hJKRUnTMKK1yxD340tm1KObZ9Fh7jRf3M0/MMkNa4BkMPOpExW\nnbkfdNoc7gX9VC3bjRQ2R/aFdQGTkxpVPLGRYqYkh/txIQ0WI5U2yXqrAoHjjFLl\nv8NVtnH/PwKBgA34hzlSTme7PGaSvNiWqEBT3/22AaXs+fB9az3Z5yt5oDjFXerE\nh1NqXpT72v1IGimdHpUCX8flybdrXwQMh3izN+45utJurmc0Ssll+4yURmY0TBct\nLm5uciHm52xzEZtUQS4mfxTX0zRfQqJg2WxIjgUjhjdG5T71/eH1KVsZAoGBAMLD\ntzs+uVwEA/ikHmivMpye19vPFquwyIf160hjjQmSU0uZwhjwQtCt0DXK8V68v//D\n/maPwX8VsuEysxyM/qdb5Ilt4+GI7IhdeLTeW+ajLSX+z0wZvxjeCiT4Fu2yNA3P\nmGQqQrwx5rKOAzVNg4ntUmtR/IVvG3vAIZLHSCTzAoGABuoZn0YDYSscl6bMiAVr\n5iPUc/Y3zejNFUYdyRxisD/wEpcrjiozgmu8ko6uJBCgmgpKWqvg1juBdW90Cqbw\ncIA9RyTw5ErJZCDwGq9NeZdpM90GX3CSN3FfHm6d73d0bQLhmBscqSk3DoWebAAe\nDFGohYEp7CtfJCwUV3nTGBY=\n-----END PRIVATE KEY-----\n";

// ID cua Google Sheet
const char spreadsheetId[] = "1tnsfrbl6mn08Gb4W6vz-78IrolLklLh8ZkEIHinhKEM";

// Bien thoi gian
unsigned long lastTime = 0;
unsigned long timerDelay = 30000;

// Token callback function
void tokenStatusCallback(TokenInfo info);

// HC-SR04 sensor
const int trigPin = 5;
const int echoPin = 18;

#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701

long duration;
float  distanceCm;
// float  distanceInch;

// DHT sensor
#define DHTPIN 14
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);
float temp;
float hum;

// NTP server to request epoch time
const char* ntpServer = "vn.pool.ntp.org";

// Bien luu thoi gian thuc te
unsigned long epochTime;
FirebaseJson response;

// Ham lay thoi gian thuc te
unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return (0);
  }
  time(&now);
  return now;
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Config time
  configTime(7, 0, ntpServer);

  GSheet.printf("ESP Google Sheet Client v%s\n\n", ESP_GOOGLE_SHEET_CLIENT_VERSION);

  // Ket noi wifi
  WiFi.setAutoConnect(true);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wifi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  
  // Cai dat va ket noi OLED I2C
  dht.begin();

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 alllocation failed"));
    for(;;);
  }
  delay(2000);
  display.clearDisplay();
  display.setTextColor(WHITE);

  // Set the callback for Google API access token generation status (for debug only)
  GSheet.setTokenCallback(tokenStatusCallback);

    // Set the seconds to refresh the auth token before expire (60 to 3540, default is 300 seconds)
  GSheet.setPrerefreshSeconds(10 * 60);

    // Begin the access token generation for Google API authentication
  GSheet.begin(CLIENT_EMAIL, PROJECT_ID, PRIVATE_KEY);

  String value1 = "Temperature (°C)";
  String value2 = "Humidity (%)";
  String value3 = "Distance (cm)";
  String value4 = "Time";
  FirebaseJson data;
  data.add("majorDimension", "COLUMNS");
  data.set("values/[1]/[0]", value1);
  data.set("values/[2]/[0]", value2);
  data.set("values/[3]/[0]", value3);
  data.set("values/[0]/[0]", value4);
  GSheet.values.append(&response, spreadsheetId, "Sheet1!A1", &data);
}

void loop() {

  bool ready = GSheet.ready();

  if (ready && millis() - lastTime > timerDelay) {
    lastTime = millis();

    FirebaseJson response;
    
    Serial.println("\n Append spreadsheet values...");
    Serial.println("------------------------------");

    // DHT-PMD playing :p
    temp = dht.readTemperature();
    hum = dht.readHumidity();
    float f = dht.readTemperature(true);
    if (isnan(hum) || isnan(temp)) {
      Serial.println("Khong doc duoc du lieu tu cam bien DHT");
    }
    display.clearDisplay();

    // HC-SR04 playing :))
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);

    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    duration = pulseIn(echoPin, HIGH);

    distanceCm = duration *SOUND_SPEED/2;
    // distanceInch = duration *CM_TO_INCH; Khong quan trong

  Serial.print("Distance (cm): ");
  Serial.println(distanceCm);
//  Serial.print("Distance (inch): ");
//  Serial.println(distanceInch);
  
  delay(1000);
    //Get timestamp
    epochTime = getTime();
    time_t epoch_time_t = epochTime;

    // Gọi hàm localtime() với 1 tham số
    struct tm *time_info = localtime(&epoch_time_t);

    // Truy cập các thành phần trong cấu trúc tm
    int year = time_info->tm_year + 1900; // Chuyển đổi năm từ 0 sang năm thực tế
    int month = time_info->tm_mon + 1; // Tháng bắt đầu từ 0
    int day = time_info->tm_mday;
    int hour = time_info->tm_hour +7;
    int minute = time_info->tm_min;
    int second = time_info->tm_sec;

    // Định dạng timestamp thành chuỗi
    String formatted_date = String(year) + "-" + String(month) + "-" + String(day) + " " + String(hour) + ":" + String(minute) + ":" + String(second);

    if(isnan(hum)||isnan(temp)||isnan(f)){
      Serial.println(F("Failed to read from DHT sensor!"));
      return;
    }

    FirebaseJson valueRange;

    valueRange.add("majorDimension", "COLUMNS");
    valueRange.set("values/[0]/[0]",  formatted_date);
    valueRange.set("values/[1]/[0]", temp);
    valueRange.set("values/[2]/[0]", hum);
    valueRange.set("values/[3]/[0]", distanceCm);

    // Append values to the sheet
    bool success = GSheet.values.append(&response, spreadsheetId,"Sheet1!A2", &valueRange);
    if (success){
      response.toString(Serial, true);
      valueRange.clear();
    }
    else{
      Serial.println(GSheet.errorReason());
    }
    Serial.println();
    Serial.println(ESP.getFreeHeap());
  }

  // Hien thi Nhiet do len man hinh OLED
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print("Temperature: ");
  display.setTextSize(1);
  display.setCursor(0,10);
  display.print(temp);
  display.print(" ");
  display.setTextSize(0.5);
  display.cp437(true);
  display.write(167);
  display.setTextSize(1.5);
  display.print("C");

  // Hien thi Do am len man hinh OLED
  display.setTextSize(1);
  display.setCursor(0, 20);
  display.print("Humidity: ");
  display.setTextSize(1);
  display.setCursor(0, 30);
  display.print(hum);
  display.print(" %");

  // Hien thi khoang cach len man hinh OLED
  display.setTextSize(1);
  display.setCursor(0, 40);
  display.print(distanceCm);
  display.print(" cm");

  display.display();
     
}

void tokenStatusCallback(TokenInfo info){
  if (info.status == token_status_error) {
    GSheet.printf("Token info: type = %s, status = %\n", GSheet.getTokenType(info).c_str(), GSheet.getTokenStatus(info).c_str());
    GSheet.printf("Token error: %s\n", GSheet.getTokenError(info).c_str());
  }
  else{
    GSheet.printf("Token info: type = %s, status = %s\n", GSheet.getTokenType(info).c_str(), GSheet.getTokenStatus(info).c_str());
  }
}
