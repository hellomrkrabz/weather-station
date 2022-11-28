#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include "Adafruit_SGP30.h"
#include "sensirion_common.h"
#include "sgp30.h"
#include <TFT_eSPI.h> // Hardware-specific library
//Wprowadź dane sieci WiFi, doktórejchcesz się podłączyć
const char* ssid = "Multiplay_EAB1";
const char* password = "ZTEEQCEE8R01978";

//Definiujemy piny ESP, doktórychpodłączony jest wyświetlacz
#define OLED_SDA 21
#define OLED_SCL 22
Adafruit_SH1106 display(21, 22);

Adafruit_SGP30 sgp;
//#define SDA 26
//#define SCL 27

//Definiujemy pin ESP, doktóregopodłączony jest czujnik DH11
#define DHTPIN 18

//Wybieramy typ wykorzystanego czujnik (DHT11)
#define DHTTYPE    DHT11
DHT dht(DHTPIN, DHTTYPE);

//Tworzymy objekt AsyncWebServer naporcie 80
AsyncWebServer server(80);
/*
uint32_t getAbsoluteHumidity(float temperature, float humidity) {
    // approximation formula from Sensirion SGP30 Driver Integration chapter 3.15
    const float absoluteHumidity = 216.7f * ((humidity / 100.0f) * 6.112f * exp((17.62f * temperature) / (243.12f + temperature)) / (273.15f + temperature)); // [g/m^3]
    const uint32_t absoluteHumidityScaled = static_cast<uint32_t>(1000.0f * absoluteHumidity); // [mg/m^3]
    return absoluteHumidityScaled;
}
*/
//Odczytujemy temperature zczujnika DHT11
String readDHTTemperature() {
  float t = dht.readTemperature();
  if (isnan(t)) {
    Serial.println("Błąd pomiaru temperatury");
    return "--";
  }
  else {
    Serial.println(t);
    return String(t);
  }
}

//Odczytujemy wilgotność zczujnika DHT11
String readDHTHumidity() {
  float h= dht.readHumidity();
  if (isnan(h)) {
    Serial.println("Błąd pomiaru wilgotności");
    return "--";
  }
  else {
    Serial.println(h);
    return String(h);
  }
}
/*
String readSGPeCO2() {
  float eco2 = sgp.eCO2;
    if (isnan(eco2)) {
    Serial.println("Mistake in measuring eCO2");
    return "--";
  }
  else {
    Serial.println(eco2);
    return String(eco2);
  }
}

String readSGPTVOC() {
  float tvoc = sgp.TVOC;
    if (isnan(tvoc)) {
    Serial.println("Mistake in measuring TVOC");
    return "--";
  }
  else {
    Serial.println(tvoc);
    return String(tvoc);
  }
}*/
//ADD TVOC + CO2
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
  <p>
  <iconify-icon icon="iwwa:co2"></iconify-icon>
  <span class="sgp-labels">eCO2</span>
  <span id="eco2">%ECO2%</span>
  <sup class="units">ppm</sup>
  </p>
  <p>
  <iconify-icon icon="material-symbols:water-voc-outline"></iconify-icon>
  <span class="sgp-labels">TVOC</span>
  <span id="tvoc">%TVOC%</span>
  <sup class="units">ppb</sup>
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

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("eco2").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/eco2", true);
  xhttp.send();
}, 10000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("tvoc").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/tvoc", true);
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
 /* Serial.print("Found SGP30 serial #");
  Serial.print(sgp.serialnumber[0], HEX);
  Serial.print(sgp.serialnumber[1], HEX);
  Serial.println(sgp.serialnumber[2], HEX);
*/

    err = sgp_measure_signals_blocking_read(&scaled_ethanol_signal,

                                            &scaled_h2_signal);

    if (err == STATUS_OK) {

        Serial.println("get ram signal!");

    } else {

        Serial.println("error reading signals");

    }

 

    // Set absolute humidity to 13.000 g/m^3

    //It's just a test value

    sgp_set_absolute_humidity(13000);

    err = sgp_iaq_init();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting WiFi..." );
  }
//ADD TVOC + cO2
  //Wyświetlenie lokalnego adresu IP modułu ESP32
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
  /*  server.on("/eco2", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", readSGPeCO2().c_str());
  });
    server.on("/tvoc", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", readSGPTVOC().c_str());
  });*/

  server.begin();
}

//int counter = 0;

void loop() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  //float co2 = sgp.eCO2;
  //float tvoc = sgp.TVOC;
  /*sgp.setHumidity(getAbsoluteHumidity(temperature, humidity));
  if (! sgp.IAQmeasure()) {
    Serial.println("Measurement failed");
    return;
  }
  Serial.print("TVOC "); Serial.print(sgp.TVOC); Serial.print(" ppb\t");
  Serial.print("eCO2 "); Serial.print(sgp.eCO2); Serial.println(" ppm");

  if (! sgp.IAQmeasureRaw()) {
    Serial.println("Raw Measurement failed");
    return;
  }
  Serial.print("Raw H2 "); Serial.print(sgp.rawH2); Serial.print(" \t");
  Serial.print("Raw Ethanol "); Serial.print(sgp.rawEthanol); Serial.println("");
 
  delay(1000);

  counter++;
  if (counter == 30) {
    counter = 0;

    uint16_t TVOC_base, eCO2_base;
    if (! sgp.getIAQBaseline(&eCO2_base, &TVOC_base)) {
      Serial.println("Failed to get baseline readings");
      return;
    }
    Serial.print("****Baseline values: eCO2: 0x"); Serial.print(eCO2_base, HEX);
    Serial.print(" & TVOC: 0x"); Serial.println(TVOC_base, HEX);*/
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

    //delay(1000);
  //Wypisanie danych naekranie OLED
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.println("Temperature:");
  display.print(t);
  display.cp437(true);
  display.write(167);
  display.println("C");
  display.display();
  display.setCursor(0, 30);
  display.setTextSize(1);
  display.println("Humidity:");
  display.setTextSize(1);
  display.print(h);
  display.println("%");
  display.display();
 /* display.setCursor(0, 1);
  display.setTextSize(1);
  display.println("eCO2:");
  display.setTextSize(1);
  display.print(sgp.eCO2);
  display.println(" ppm");
  display.display();
  display.setCursor(0, 30);
  display.setTextSize(1);
  display.println("TVOC");
  display.setTextSize(1);
  display.print(sgp.TVOC);
  display.println(" ppb\t");
  display.display();*/
  display.clearDisplay();
  delay(1000);
}