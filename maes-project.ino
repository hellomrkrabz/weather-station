#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include <DHT.h>
#include <Adafruit_Sensor.h>
#include "sensirion_common.h"
#include "sgp30.h"
#include <TFT_eSPI.h>

const char* ssid = "Multiplay_EAB1";
const char* password = "ZTEEQCEE8R01978";

#define OLED_SDA 21
#define OLED_SCL 22
Adafruit_SH1106 display(21, 22);

#define DHTPIN 18

#define DHTTYPE    DHT11
DHT dht(DHTPIN, DHTTYPE);

AsyncWebServer server(80);

String readDHTTemperature() {
  float t = dht.readTemperature();
  if (isnan(t)) {
    Serial.println("Temperature measurement error");
    return "--";
  }
  else {
    Serial.println(t);
    return String(t);
  }
}

String readDHTHumidity() {
  float h= dht.readHumidity();
  if (isnan(h)) {
    Serial.println("Humidity measurement error");
    return "--";
  }
  else {
    Serial.println(h);
    return String(h);
  }
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <script src="https://code.iconify.design/iconify-icon/1.0.1/iconify-icon.min.js"></script>
  <style>
    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    h2 { font-size: 3.0rem; }
    p { font-size: 3.0rem; }
    .units { font-size: 1.2rem; }
    .dht-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
    .sgp-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
  </style>
</head>
<body>
  <h2>AIR QUALITY</h2>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <span class="dht-labels">Temperature</span> 
    <span id="temperature">%TEMPERATURE%</span>
    <sup class="units">&deg;C</sup>
  </p>
  <p>
    <i class="fas fa-tint" style="color:#00add6;"></i> 
    <span class="dht-labels">Humidity</span>
    <span id="humidity">%HUMIDITY%</span>
    <sup class="units">&percnt;</sup>
  </p>
</body>
<script>
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperature").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperature", true);
  xhttp.send();
}, 10000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("humidity").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/humidity", true);
  xhttp.send();
}, 10000 ) ;
</script>
</html>)rawliteral";

String processor(const String& var) {
  if (var == "TEMPERATURE") {
    return readDHTTemperature();
  }
  else if (var == "HUMIDITY") {
    return readDHTHumidity();
  }
  return String();

}

void setup() {
  s16 err;

  u32 ah = 0;

  u16 scaled_ethanol_signal, scaled_h2_signal;

  Serial.begin(115200);

  Serial.println("serial start!!");
  while (!Serial) { delay(10); }
  display.begin(SH1106_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.display();
  Wire.begin(OLED_SDA, OLED_SCL);
  dht.begin();
  while (sgp_probe() != STATUS_OK) {

      Serial.println("SGP failed");

      while (1);

  }

    err = sgp_measure_signals_blocking_read(&scaled_ethanol_signal,

                                            &scaled_h2_signal);

    if (err == STATUS_OK) {

        Serial.println("get ram signal!");

    } else {

        Serial.println("error reading signals");

    }

    sgp_set_absolute_humidity(13000);

    err = sgp_iaq_init();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting WiFi..." );
  }
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", readDHTTemperature().c_str());
  });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", readDHTHumidity().c_str());
  });
  server.begin();
}

void loop() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  s16 err = 0;

  u16 tvoc_ppb, co2_eq_ppm;

  err = sgp_measure_iaq_blocking_read(&tvoc_ppb, &co2_eq_ppm);

  if (err == STATUS_OK) {

      Serial.print("tVOC  Concentration:");

      Serial.print(tvoc_ppb);

      Serial.println("ppb");
      Serial.print("CO2eq Concentration:");

      Serial.print(co2_eq_ppm);

      Serial.println("ppm");

    } else {

      Serial.println("error reading IAQ values\n");

    }
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.setTextSize(1);
  display.println("eCO2:");
  display.setTextSize(2);
  display.print(co2_eq_ppm);
  display.println(" ppm");
  display.display();
  display.setCursor(0, 35);
  display.setTextSize(1);
  display.println("TVOC");
  display.setTextSize(2);
  display.print(tvoc_ppb);
  display.println(" ppb\t");
  display.display();
  display.clearDisplay();
  delay(5000);
}