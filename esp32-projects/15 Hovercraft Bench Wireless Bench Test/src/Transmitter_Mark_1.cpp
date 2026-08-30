#include <Arduino.h>

/*
 --------------------------------------------------------------
       Import Libraries
 --------------------------------------------------------------
*/
// Oled Library
#include "DisplayManager.h"   // Hardware Library
#include "Transmitter_Info.h" // Data Library

// Radio Library
#include "RadioPayload.h" // Data Library
#include "NRF_Radio.h"          // Hardware Library
 

// Input Devices Libraries
#include "Joystick.h"
#include "POT.h"
#include "Toggle.h"


/*
 --------------------------------------------------------------
       Generate Objects
 --------------------------------------------------------------
*/

// ------- Display Object -----------
// OLED Pin Defintitions
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_I2C_ADDRESS 0x3C

#define SDA_PIN 21
#define SCL_PIN 22
TwoWire I2C_0 = TwoWire(0);

DisplayManager oled_display(SCREEN_WIDTH, SCREEN_HEIGHT, OLED_RESET, OLED_I2C_ADDRESS, I2C_0, SDA_PIN, SCL_PIN);
Transmitter_Info transmitter_info(oled_display);

// ------- Joy Stick Objects -----------
// Analog Joystick Pin Defintitions
Joystick joy1(32, 33, 25, "1", Joystick::Toggle);
Joystick joy2(26, 13, 4, "2", Joystick::Momentary);

// ------- Potentiometer Objects -----------
// Potentiometer Pin Definitions
#define POT1_PIN 36
#define POT2_PIN 39
#define POT3_PIN 34

Potentiometer pot1(POT1_PIN, "POT 1");
Potentiometer pot2(POT2_PIN, "POT 2");
Potentiometer pot3(POT3_PIN, "POT 3");

// ------- Toggle Switch Objects -----------
ToggleSwitch Switch1(27, "Light Switch");
ToggleSwitch Switch2(14, "Light Switch");
ToggleSwitch Switch3(35, "Light Switch");


// --------- NRF Radio Object --------------
// --------- Pin Definitions for NRF2401L ----------
#define PIN_SCK   18
#define PIN_MISO  19
#define PIN_MOSI  23
#define PIN_CSN   17
#define PIN_CE    5

static const uint8_t RADIO_RX_ADDR[6] = "00001";
static const uint8_t RADIO_TX_ADDR[6] = "00002";

// Create radio object (CE, CSN)
NrfRadio<RadioPayload> radio(PIN_CE, PIN_CSN, RADIO_RX_ADDR, RADIO_TX_ADDR, 1);

// ------- Persistent Received Data -----------
// Stores the LAST valid Telemetry packet
RadioPayload telemetryRx{};
/*
 --------------------------------------------------------------
       Setup
 --------------------------------------------------------------
*/
void setup() {
  Serial.begin(115200);
  delay(200);

  Serial.print("Payload size: ");
  Serial.println(sizeof(RadioPayload));

  // ================ Initialize the Elements ============
  oled_display.begin("Hovercraft Transmitter Mark 1");

  // Initialize Both Joystick modes
  joy1.begin(120, Joystick::PullUp);
  joy2.begin(120, Joystick::PullUp);

  // Potentiometer setup
  pot1.begin();
  pot2.begin();
  pot3.begin();

  // Intialize the Toggle Switches
  Switch1.begin();
  Switch2.begin();
  Switch3.begin();

  // Intialize the Radio
  radio.begin();



  // ================== Calibration Message ================
  oled_display.displayMessage("Calibrating the Analog Sticks");

  // Calibrate the Joysticks
  Serial.println("Starting Joy 1 calibration");
  joy1.calibrate();
  oled_display.displayMessage("Joystick 1 Calibrated");
  Serial.println("Joy 1 done");
  delay(500);
 

  Serial.println("Starting Joy 2 calibration");
  joy2.calibrate();
  oled_display.displayMessage("Joystick 2 Calibrated");
  Serial.println("Joy 2 done");
  delay(500);


  oled_display.displayMessage("Calibration Complete");
  Serial.println("Entering loop");
  delay(500);



}

/*
 --------------------------------------------------------------
       Loop
 --------------------------------------------------------------
*/
void loop() {
  static unsigned long lastDisplayMs = 0;
  unsigned long nowMs = millis();

  // 1) Maintain Heartbeat + Timeout
  radio.serviceConnection();

  // 2) Check for recieved packets

  RadioPayload in{};
  bool gotPacket = radio.receivePackage(in);

  if (gotPacket)
  {
      if (in.type == PacketType::TELEMETRY)
      {
          telemetryRx = in;
          Serial.println("------ Telemtery Packet --------");

          Serial.print(">Servo 1 Angle: ");
          Serial.println(telemetryRx.Servo_1_Angle);

          Serial.print(">Servo 2 Angle: ");
          Serial.println(telemetryRx.Servo_2_Angle);

          Serial.print(">Servo 3 Angle: ");
          Serial.println(telemetryRx.Servo_3_Angle);

          Serial.print(">Servo 4 Angle: ");
          Serial.println(telemetryRx.Servo_4_Angle);

          Serial.println();
      }
  }

  // 3) Read Input Devices
  // -------------- Read Analog Joysticks ------------
  joy1.update();
  joy2.update();
  
  // ------------- Read Potentiometer Value ----------
  pot1.update();
  pot2.update();
  pot3.update();

  // ------------ Read ToggleSwitch Values ----------
  Switch1.update();
  Switch2.update();
  Switch3.update();


  // 4) Send Control Packets every 20ms
  static uint32_t lastControlMs = 0;

    if (nowMs - lastControlMs >= 20)
    {
        lastControlMs = nowMs;

        RadioPayload out{};

        out.type = PacketType::CONTROL;

        // Joystick 1
        out.joy1X = joy1.getX();
        out.joy1Y = joy1.getY();
        out.joy1Button = joy1.getButtonState();

        // Joystick 2
        out.joy2X = joy2.getX();
        out.joy2Y = joy2.getY();
        out.joy2Button = joy2.getButtonState();

        // Potentiometers
        out.pot1 = pot1.getValue();
        out.pot2 = pot2.getValue();
        out.pot3 = pot3.getValue();

        // Toggle switches
        out.switch1 = Switch1.isOn();
        out.switch2 = Switch2.isOn();
        out.switch3 = Switch3.isOn();

        radio.sendPackage(out);
    }



  // -------------- Display OLED -----------
  if (nowMs - lastDisplayMs >= 200) {
    lastDisplayMs = nowMs;

    transmitter_info.update(
      (const char*) RADIO_TX_ADDR,
      (const char*) RADIO_RX_ADDR,
      joy1.getX(),
      joy1.getY(),
      joy1.getButtonState(),

      joy2.getX(),
      joy2.getY(),
      joy2.getButtonState(),

      pot1.getValue(),
      pot2.getValue(),
      pot3.getValue(),

      Switch1.isOn(),
      Switch2.isOn(),
      Switch3.isOn(),

      radio.isConnected(),

      telemetryRx.motor_1_state,
      telemetryRx.motor_2_state,
      telemetryRx.motor_3_state,
      telemetryRx.motor_4_state,
      telemetryRx.motor_5_state,
      telemetryRx.motor_6_state
    );
  }

  delay(5);

}
