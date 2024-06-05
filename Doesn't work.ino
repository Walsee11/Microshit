#include <WiFi.h>
#include <Adafruit_Sensor.h>
#include "DHT.h"
#include "time.h"
#include <ESP_Google_Sheet_Client.h>

// For SD/SD_MMC mounting helper
#include <GS_SDHelper.h>

#define WIFI_SSID "PMD"
#define WIFI_PASSWORD "Dung3011"

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

// BME280 I2C

// Variables to hold sensor readings
float temp;
float hum;


// NTP server to request epoch time
const char* ntpServer = "pool.ntp.org";

// Variable to save current epoch time
unsigned long epochTime; 

// Function that gets current epoch time
unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    //Serial.println("Failed to obtain time");
    return(0);
  }
  time(&now);
  return now;
}

void setup(){

    Serial.begin(115200);
    Serial.println();
    Serial.println();

    //Configure time
    configTime(0, 0, ntpServer);

    // Initialize BME280 sensor 

    Serial.println(F("DHT sensor test!"));

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
}

void loop(){

    // Call ready() repeatedly in loop for authentication checking and processing
    bool ready = GSheet.ready();

    if (ready && millis() - lastTime > timerDelay){
        lastTime = millis();

        FirebaseJson response;

        Serial.println("\nAppend spreadsheet values...");
        Serial.println("----------------------------");

        FirebaseJson valueRange;


        temp = dht.readTemperature();
        hum = dht.readHumidity();
        float f = dht.readTemperature(true);
        // Get timestamp
        epochTime = getTime();

        if(isnan(hum)||isnan(temp)||isnan(f)){
          Serial.println(F("Failed to read from DHT sensor!"));
          return;
        }

        Serial.print(F("Humidity: "));
        Serial.print(hum);
        Serial.print(F("% Temperature: "));
        Serial.print(temp);
        Serial.print(F("°C "));

        valueRange.add("majorDimension", "COLUMNS");
        valueRange.set("values/[0]/[0]", epochTime);
        valueRange.set("values/[1]/[0]", temp);
        valueRange.set("values/[2]/[0]", hum);

        // For Google Sheet API ref doc, go to https://developers.google.com/sheets/api/reference/rest/v4/spreadsheets.values/append
        // Append values to the spreadsheet
        bool success = GSheet.values.append(&response /* returned response */, spreadsheetId /* spreadsheet Id to append */, "Sheet1!A1" /* range to append */, &valueRange /* data range to append */);
        if (success){
            response.toString(Serial, true);
            valueRange.clear();
        }
        else{
            Serial.println(GSheet.errorReason());
        }
        Serial.println();
        //Serial.println(ESP.getFreeHeap());
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
