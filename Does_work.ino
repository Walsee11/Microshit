#include <WiFi.h>
#include "DHT.h"
#include "time.h"
#include <ESP_Google_Sheet_Client.h>

#define WIFI_SSID ""
#define WIFI_PASSWORD ""

#define DHTPIN 21
#define DHTTYPE DHT11

DHT dht(DHTPIN,DHTTYPE);

// Google Project ID
#define PROJECT_ID "microprocessor-425215"

// Service Account's client email
#define CLIENT_EMAIL "datalogging@microprocessor-425215.iam.gserviceaccount.com"

// Service Account's private key
const char PRIVATE_KEY[] PROGMEM = "-----BEGIN PRIVATE KEY-----\nMIIEvAIBADANBgkqhkiG9w0BAQEFAASCBKYwggSiAgEAAoIBAQDqz7Lew1nRfkog\nzueKoPrqgS5BLDrJIeSq7elvUtDgZ62BZ06qcetO3gAxnpmZifAy"
 "+BMBGZGJZOPL\nwCG2UHH4OC6fP+mdVwJr7/TjhlfMBZOiEnkyQVW/xiF944vU9ToHEX2oBPGL4jrF\nbYLbdZHGL8+onZoHFDu8dSTTr0lZxyZUQHKtjbat9jjmlDfjZABXs7U5LQZ9a4pF\nV"
 "+ecQnw3I0c3U9fv0YIylpQu/VpdjpWBZXbDqkt7BDNNRf5SDIMgh7F9oLvIkBuH\n6ZN9T61yaLBUdzbwj8gB5/TR1iYOnVAXRaIZ/0Fpk0xEa71/wm/8ApIoJofWXEkv\nXAHJaZA9AgMBAAECggEAL"
 "+5PPyElyblrhMhvY7EqHunZKQUQB+QDAzA21RpU2GXZ\neT9/A5Rm+c2fwQE0re6GCUVxzajsOB1P0ZTJ3OcNaKOBQtEfTRaIgEN2W+ZdsCAY\nS35VXoCPMisNxZ6H+ePH4U789SnzK4B1yjypOEaGx"
 "+uVCkCHgzfqXLUV3B8dDDXS\nXQOd2t5SIC/6g/RduZAko44Mi2+pscfOoilbeYcyyrEDeHDrZjrjs7z7Pzr5sWTK\nC47Uf9BCOaTwLisvDJzdf5dJNJ7DxjYL/mlnYUHUZGvB341ZIvzNQVYmKCj3qsvV"
 "\nbDDDiBMAil5CDzXt+xvRDg/TuXJMUURxZdPCjlUueQKBgQD5kVR46xMS9Qx0HJKh\nPudQWaaS9roAb6AFKNFYpNCZQg73YFzj6WYbL+jMHnYCKtzjeS+KB4c23itDHqpb"
 "\nUPl3RzveSs4Yfz3eKuJIXtXkJnM0H/0xIT6furEV2dCgv4iXCMCjToZTeuzdaJTv\nE2zRMPrWIqQMDjbJ4LAHi1VMNQKBgQDw3QE/gb9Qm89I+CB4jpGqoCuHpMqUPfrk"
 "\ntkL8WgHynOc5SKGeP63jR2JQLTqTfUrYErWScBDEMcbmnxfsKn3eFJz2OXV4YOyp\nGe3ewI0NvM1zh8SKWcTcK7BM2RrFIzrvDJe+nDIUfU4gGqY2AJAd3qRwxv70e4Ph\nAJVSikjk6QKBgEurbABpzlO"
 "+JzOBglgPuZUcSaxf0M/XCJ7n3GnTN0hwxhbtidy0\n5PezVBOn3MIVevohwW9Jjw7s2BM21hJv/+PRnm5tIgY9dmXJfOjSkGnn2qBgXqWp\n3dISiiUE8QP8bw7UYYxif8oKCI2taxL"
 "+Qc0/255XzIO3P5eAX/85mPhNAoGARmUx\nQjBXi+Wd1YO/abyxUh7x2fkNw/nBZCrotPxRaWj2I0XZBnkyByUL1McDQavyvefp\n7VyXad/qc+i3gyyAEPIRVd1BOeeKA3oyjVHVZ9hwa32YsZ6+7GJQaoVhT5CWXttd"
 "\nVjYsZNSiirt/LHVJqZEymktupGGdD5uuCKul1lECgYBMrSwWJg1MWMtW9JH1881U\nIBkTx+fvYOBL1ah5dChG02Dlah9JF8NN2bHwA29BgXt7wFZ8e8Ujj9PBaUBusFfc\neLgwFX7vJ/ixJ464cXm"
 "+y/L08Mc8ovOXsrR945HWL/ZHc/0txp8ihGf6k2TgU48+\nrOZs9YQygAqsiXkX7ka33g==\n-----END PRIVATE KEY-----\n";

// The ID of the spreadsheet where you'll publish the data
const char spreadsheetId[] = "1EfVcSzhqw1yhXdVQswK3VG-3hJliPp4-YN0LaH6CXUA";

// Timer variables
unsigned long lastTime = 0;  
unsigned long timerDelay = 30000;

// Token Callback function
void tokenStatusCallback(TokenInfo info);

// Variables to hold sensor readings
float temp;
float hum;

// NTP server to request epoch time
const char* ntpServer = "vn.pool.ntp.org";

// Variable to save current epoch time
unsigned long epochTime; 
FirebaseJson response;

// Function that gets current epoch time
unsigned long getTime() {
    time_t now;
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
      Serial.println("Failed to obtain time");
      return(0);
    }
    time(&now);
    return now;
}

void setup(){

    Serial.begin(115200);
    Serial.println();

    //Configure time
    configTime(7, 0, ntpServer);

    //Serial.println(F("DHT sensor test!"));
    GSheet.printf("ESP Google Sheet Client v%s\n\n", ESP_GOOGLE_SHEET_CLIENT_VERSION);

    // Connect to Wi-Fi
    WiFi.setAutoReconnect(true);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(1000);
    }

    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    dht.begin();

    // Set the callback for Google API access token generation status (for debug only)
    GSheet.setTokenCallback(tokenStatusCallback);

    // Set the seconds to refresh the auth token before expire (60 to 3540, default is 300 seconds)
    GSheet.setPrerefreshSeconds(10 * 60);

    // Begin the access token generation for Google API authentication
    GSheet.begin(CLIENT_EMAIL, PROJECT_ID, PRIVATE_KEY);

    String value1 = "Temperature (°C)";
    String value2 = "Humidity (%)";
    String value3 = "Time";
    FirebaseJson data;
    data.add("majorDimension", "COLUMNS");
    data.set("values/[1]/[0]", value1);
    data.set("values/[2]/[0]", value2);
    data.set("values/[0]/[0]", value3);
    GSheet.values.append(&response, spreadsheetId, "Sheet1!A1", &data);
}

void loop(){

    // Call ready() repeatedly in loop for authentication checking and processing
    bool ready = GSheet.ready();

    if (ready && millis() - lastTime > timerDelay){
        lastTime = millis();
        Serial.println("\nAppend spreadsheet values...");
        Serial.println("----------------------------");

        temp = dht.readTemperature();
        hum = dht.readHumidity();
        float f = dht.readTemperature(true);

        // Get timestamp
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

        //Serial.print(F("Humidity: "));
        //Serial.print(hum);
        //Serial.print(F("% Temperature: "));
        //Serial.print(temp);
        //Serial.print(F("°C "));

        FirebaseJson valueRange;

        valueRange.add("majorDimension", "COLUMNS");
        valueRange.set("values/[0]/[0]", formatted_date);
        valueRange.set("values/[1]/[0]", temp);
        valueRange.set("values/[2]/[0]", hum);

        // Append values to the spreadsheet
        bool success = GSheet.values.append(&response, spreadsheetId, "Sheet1!A2" , &valueRange);
        if (success){
            response.toString(Serial, true);
            valueRange.clear();
        }
        else{
            Serial.println(GSheet.errorReason());
        }
        Serial.println();
    }
}

void tokenStatusCallback(TokenInfo info){
    if (info.status == token_status_error){
        GSheet.printf("Token info: type = %s, status = %s\n", GSheet.getTokenType(info).c_str(), GSheet.getTokenStatus(info).c_str());
        GSheet.printf("Token error: %s\n", GSheet.getTokenError(info).c_str());
    }
    else{
        GSheet.printf("Token info: type = %s, status = %s\n", GSheet.getTokenType(info).c_str(), GSheet.getTokenStatus(info).c_str());
    }
}
