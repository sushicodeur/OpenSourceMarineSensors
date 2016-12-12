#include <ESP8266WiFi.h>
#include <WiFiUDP.h>
#include <DHT.h>

#define DHTPIN  D2
#define DHTTYPE DHT11

const char* ssid = "YOUR-WIFI";
const char* password = "123password";

const IPAddress remoteIp(192, 168, 1, 255);
const int remotePort = 10112;

WiFiUDP UDP;

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);
  connectWifi();
  dht.begin();
  Serial.print("00 (DHTPIN) : ");
  Serial.println(DHTPIN);
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {

    char temperature[5];
    getTemperature(temperature);

    Serial.print("temperature : ");
    Serial.println(temperature);

    sendIIMTA(temperature);

    delay(5000);

  } else {
    connectWifi();
  }
}

void sendIIMTA(char *temperature) {
  char nmeaTemplate[17] = "$IIMTA,%s,C*";

  Serial.print("nmeaTemplate : ");
  Serial.println(nmeaTemplate);

  char nmeaSentence[17];
  sprintf(nmeaSentence, nmeaTemplate, temperature);

  Serial.print("nmeaSentence : ");
  Serial.println(nmeaSentence);

  byte crc = calcNmeaChecksum(nmeaSentence);

  Serial.print("crc : ");
  Serial.println(crc);

  char nmeaWithCrc[17];
  sprintf(nmeaWithCrc, "%s%02d", nmeaSentence, crc);

  Serial.print("nmeaWithCrc : ");
  Serial.println(nmeaWithCrc);

  sendUDPPacket(nmeaWithCrc, remoteIp, remotePort);
}

void getTemperature(char *temperature) {

  float t = dht.readTemperature();

  if (isnan(t)) {
    Serial.println("###### Error reading temperature! ######");
    dtostrf(999, 2, 1, temperature);
  }
  else {
    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.println(" °C");

    dtostrf(t, 2, 1, temperature);
  }
}

byte calcNmeaChecksum(char *sentence) {
  byte CRC = 0;
  byte x = 1;
  while (sentence[x] != '*') { // XOR every character in between '$' and '*'
    CRC = CRC ^ sentence[x] ;
    x++;
  }
  return CRC;
}

boolean connectWifi() {
  boolean state = true;
  int i = 0;
  WiFi.begin(ssid, password);
  Serial.println("");
  Serial.print("#WIFI Connecting to ");
  Serial.println(ssid);

  Serial.print("#WIFI Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (i > 10) {
      state = false;
      break;
    }
    i++;
  }

  if (state) {
    Serial.println("");
    Serial.print("#WIFI Connected to ");
    Serial.println(ssid);
    Serial.print("#WIFI IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("");
    Serial.println("#WIFI Connection failed.");
  }

  return state;
}

void sendUDPPacket(const char* packet, IPAddress remoteIp, int remotePort) {

  Serial.println("");
  Serial.println("#UDPSend beginPacket...");

  if (UDP.beginPacket(remoteIp, remotePort)) {

    Serial.print("#UDPSend writePacket: ");
    Serial.println(packet);

    UDP.write(packet);

    Serial.println("#UDPSend endPacket...");

    if (UDP.endPacket()) {
      Serial.println("#UDPSend endPacket success");

    } else {
      Serial.println("#UDPSend endPacket failed");
    }
  } else {
    Serial.println("#UDPSend beginPacket failed");
  }

}
