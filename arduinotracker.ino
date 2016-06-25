/*
Open up the serial console on the Arduino at 115200 baud to interact with FONA

Note that if you need to set a GPRS APN, username, and password scroll down to
the commented section below at the end of the setup() function.
*/
#include "Adafruit_FONA.h"

#define FONA_RX 2
#define FONA_TX 9   // digital pin 9 on Arduino Micro
#define FONA_RST 4

// this is a large buffer for replies
char replybuffer[255];

// We default to using software serial. If you want to use hardware serial
// (because softserial isnt supported) comment out the following three lines 
// and uncomment the HardwareSerial line
#include <SoftwareSerial.h>
SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
SoftwareSerial *fonaSerial = &fonaSS;

// Hardware serial is also possible!
//  HardwareSerial *fonaSerial = &Serial1;

// Use this for FONA 800 and 808s
Adafruit_FONA fona = Adafruit_FONA(FONA_RST);
// Use this one for FONA 3G
//Adafruit_FONA_3G fona = Adafruit_FONA_3G(FONA_RST);

//uint8_t readline(char *buff, uint8_t maxbuff, uint16_t timeout = 0);

uint8_t type;

char imei[16] = {0};

void setup() {
  while (!Serial);

  Serial.begin(115200);
  Serial.println(F("arduinotracker"));
  Serial.println(F("Initializing....(May take 3 seconds)"));

  fonaSerial->begin(4800);
  if (!fona.begin(*fonaSerial)) {
    Serial.println(F("Couldn't find FONA"));
    while (1);
  }
  type = fona.type();
  Serial.println(F("FONA is OK"));
  Serial.print(F("Found "));
  switch (type) {
    case FONA800L:
      Serial.println(F("FONA 800L")); break;
    case FONA800H:
      Serial.println(F("FONA 800H")); break;
    case FONA808_V1:
      Serial.println(F("FONA 808 (v1)")); break;
    case FONA808_V2:
      Serial.println(F("FONA 808 (v2)")); break;
    case FONA3G_A:
      Serial.println(F("FONA 3G (American)")); break;
    case FONA3G_E:
      Serial.println(F("FONA 3G (European)")); break;
    default: 
      Serial.println(F("???")); break;
  }
  
  // Print module IMEI number.
  if (getIMEI(imei)) {
    Serial.print("Module IMEI: ");
    Serial.println(imei);
  }

  // Optionally configure a GPRS APN, username, and password.
  // You might need to do this to access your network's GPRS/data
  // network.  Contact your provider for the exact APN, username,
  // and password values.  Username and password are optional and
  // can be removed, but APN is required.
  fona.setGPRSNetworkSettings(F("pwg"), F(""), F(""));    // For US Mobile - see https://www.usmobile.com/help/knowledge-base/configuring-your-device/

  // Optionally configure HTTP gets to follow redirects over SSL.
  // Default is not to follow SSL redirects, however if you uncomment
  // the following line then redirects over SSL will be followed.
  //fona.setHTTPSRedirect(true);
}

// turn GPRS on
bool isGprsEnabled = false;
bool enableGprs() {
  if (!isGprsEnabled) {
    isGprsEnabled = fona.enableGPRS(true);
  }
  return isGprsEnabled;
}

// turn GPS on
bool isGpsEnabled = false;
bool enableGps() {
  if (!isGpsEnabled) {
    isGpsEnabled = fona.enableGPS(true);
  }
  return isGpsEnabled;
}

// check GPS fix
// returns:
//  -1 on error
//   0 when GPS is off
//   1 when no fix
//   2 when 2d fix
//   3 when 3d fix
bool getGpsFix() {
  int8_t stat = fona.GPSstatus();
  return stat >= 2;
  if (stat < 0 || stat > 3) return -1;
  if (stat == 0) return 0;
  if (stat == 1) return 1;
  if (stat == 2) return 2;
  if (stat == 3) return 3;
}

// get unique device identifier - IMEI number
// imei must be a 16 character char array
// returns true if we successfully retrieve IMEI
bool getIMEI(char *imei) {
  uint8_t imeiLen = fona.getIMEI(imei);
  return imeiLen > 0;
}

// lat - latitude in degrees
// lon - longitude in degrees
// returns true if successfully retrieves lat/long; false otherwise
bool getGpsLocation(float *lat, float *lon) {
  float *speed_kph = NULL;
  float *heading = NULL;
  float *altitude = NULL;
  Serial.println("getGpsLocation: ");
  bool retval = fona.getGPS(lat, lon, speed_kph, heading, altitude);
  Serial.println(retval);
  return retval;
}

bool httpGet(char *url, uint16_t *statusCode, String response) {
  Serial.println("httpGET:");
  Serial.println(url);
  int16_t length;
  response = String("");

  if (!fona.HTTP_GET_start(url, statusCode, (uint16_t *)&length)) {
    Serial.println("Failed!");
    return false;
  }
  
  while (length > 0) {
    while (fona.available()) {
      char c = fona.read();
      response += c;
//      Serial.write(c);
      length--;
      if (!length) break;
    }
  }
  
  fona.HTTP_GET_end();

  Serial.println("****");
  Serial.println(response);

  return true;
}

void loop() {
  Serial.print(F("loop"));
//  while (! Serial.available() ) {
//    if (fona.available()) {
//      Serial.write(fona.read());
//    }
//  }

  float lat;
  float lon;
  char latString[16] = {0};
  char lonString[16] = {0};
  char url[200] = {0};

  // turn on GPRS and GPS
  if (enableGprs() && enableGps()) {
    Serial.println(F("GPRS and GPS turned on"));
    
    if (getGpsLocation(&lat, &lon)) {
      latString[0] = '\0';
      lonString[0] = '\0';
      url[0] = '\0';
      toString(latString, lat, 9, 6);
      toString(lonString, lon, 10, 6);
//      Serial.println(lat);
//      Serial.println(lon);
//      Serial.println(latString);
//      Serial.println(lonString);
      buildUrl(url, imei, latString, lonString);
      Serial.println(url);
  
      // make a GET request against http://www.sideprojects.space/gpstracker/checkin/:deviceId/:lat/:long
      String response = "";
      uint16_t statusCode;
      httpGet(url, &statusCode, response);
    }
  } else {
    Serial.println(F("Failed to turn on GPRS and GPS"));
  }
  
  delay(10000);
}

// build a URL string of the following form: http://www.sideprojects.space/gpstracker/checkin/:deviceId/:lat/:long
void buildUrl(char *url, char *imei, char *lat, char *lon) {
  strcat(url, "http://www.sideprojects.space/gpstracker/checkin/");
  strcat(url, imei);
  strcat(url, "/");
  strcat(url, lat);
  strcat(url, "/");
  strcat(url, lon);
}

void toString(char *dest, float f, int width, int precision) {
  char tmp[50];
  dtostrf(f, width, precision, tmp);
  sprintf(dest, "%s", tmp);
}

