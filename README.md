# arduinotracker
Arduino GPS tracker

## Getting Started

1. Clone this repo into your Arduino sketches directory.
  ```
  cd <arduino sketches directory>
  git clone https://github.com/davidkellis/arduinotracker.git
  ```

2. Install the Adafruit FONA Arduino library - https://github.com/adafruit/Adafruit_FONA

3. Change the line (around line 10) that reads
  ```
  #define FONA_TX 9   // digital pin 9 on Arduino Micro
  ```
  to the appropriate pin for your Arduino. See https://learn.adafruit.com/adafruit-fona-808-cellular-plus-gps-breakout?view=all#wiring-to-arduino for more information about the proper pin numberings.
  
4. Open the arduinotracker sketch in the Arduino IDE.

5. Connect your Arduino to your computer.

6. Click the Upload button in the Arduino IDE to upload the arduinotracker sketch to your Arduino.

7. Wait forever while the GPS unit gets a fix.

8. After an eternity, the application running on the Arduino will send updated GPS coordinates to the URL that is built in the buildUrl function (defined around line 217).
  
  Currently, the URL that gets built is of the form: http://www.sideprojects.space/gpstracker/checkin/:deviceId/:lat/:long
  where :deviceId is the FONA's IMEI number, :lat is the FONA's current latitude, and :long is the FONA's current longitude.
