#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_I2C_ADDRESS 0x3C

#define SDA_PIN 21
#define SCL_PIN 22

// Your joystick wiring
#define JOY_X 25
#define JOY_Y 33
#define JOY_SW 32

const int DEADZONE = 120;   // tweak 80-200

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

int xCenter = 2048;
int yCenter = 2048;


class Joystick {
public:
  
  enum ButtonMode {Momentary, Toggle};
  enum PullMode   { PullUp, PullDown, NoPull };

  // Constructor: Sets up the x pin, y pin, switch pin, and name of the joystick
  Joystick(int x, int y, int sw, const char* name, ButtonMode mode) {
    xPin = x;
    yPin = y;
    swPin = sw;
    _name = name;
    
    _xCenter = 2048;
    _yCenter = 2048;

    // Button Attributes
    _mode = mode;
    _toggleState = false;
    _lastRawPressed = false;
    _btnOutput = false;
  }

  // Method 1: sets up the pinmode of the switch and establishes deadzone
  void begin(int deadzone = 120, PullMode mode = PullUp) {
    _deadzone = deadzone;

    if (mode == PullUp) {
        pinMode(swPin, INPUT_PULLUP);
        _pressedLevel = LOW;      // pressed reads LOW
    }
    else if (mode == PullDown) {
        pinMode(swPin, INPUT_PULLDOWN);
        _pressedLevel = HIGH;     // pressed reads HIGH
    }
    else { // NoPull
        pinMode(swPin, INPUT);
        // In this case you MUST have an external resistor
    }
  }

  // Method 2: Calibrates the Joystick to find the xCenter and yCenter
  void calibrate(Adafruit_SSD1306 &display, int samples = 200) {
    // Sets up 2 variables to sum up the reading per sample of each axis 
    long xSum = 0;
    long ySum = 0;

    // OLED message
    display.clearDisplay();
    display.setCursor(0, 16);
    display.print(_name);
    display.println(": Calibrating");
    display.setCursor(0, 26);
    display.println("Don't touch stick");
    display.display();

    delay(1000);

    // Loop Function to find the x and y sum for the specified sample rate
    for (int i = 0; i < samples; i++) {
        xSum += analogRead(xPin);
        ySum += analogRead(yPin);
        delay(3);
    }

    // Finds the center by dividing the sum by the number of samples for each axis
    _xCenter = xSum / samples;
    _yCenter = ySum / samples;

    // Show results on the OLED Screen
    display.clearDisplay();
    display.setCursor(0, 16);
    display.print(_name);
    display.println(": Cal done");
    display.setCursor(0, 26);
    display.print("Xc: "); display.println(_xCenter);
    display.setCursor(0, 36);
    display.print("Yc: "); display.println(_yCenter);
    display.display();

    delay(2000);
  }

  // Method 3: Reads Button State and defines Button Type (momentary vs Toggle)
  void readButton() {
    // Digital Read of Button
    bool rawPressed = (digitalRead(swPin) == _pressedLevel);

    // Button Behavior Logic
    if (_mode == Momentary) { // Momentary Mode
      _btnOutput = rawPressed;
    }

    else { // Toggle Mode
      if (!_lastRawPressed && rawPressed) {
        _toggleState = !_toggleState;
      }
      _btnOutput = _toggleState;
    }

    _lastRawPressed = rawPressed;
  }

  // Method 4: Reads the Analog Input of each Joystick Axis
  void joystickRead() {
    _x = analogRead(xPin);
    _y = analogRead(yPin);
  }

  // Method 5: Displays text on Oled Screen
  void drawBlock(Adafruit_SSD1306 &display, int x0, int y0) {
    display.setTextSize(1);

    // 1) Print X Value
    display.setCursor(x0, y0);
    display.print("x");
    display.print(_name);
    display.print(":");
    display.print(_x);

    // 2) Print Y value
    display.setCursor(x0, y0 + 10);
    display.print("y");
    display.print(_name);
    display.print(":");
    display.print(_y);

    //3) Print Direction Values
    // --- Vertical (U/D) at fixed position ---
    display.setCursor(x0, y0 + 20);

    if (_y > _yCenter + _deadzone)
        display.print("D");     // Down
    else if (_y < _yCenter - _deadzone)
        display.print("U");     // Up
    else
        display.print("-");     // Centered


    // --- Horizontal (R/L) at fixed position ---
    display.setCursor(x0 + 10, y0 + 20);

    if (_x > _xCenter + _deadzone)
        display.print("R");     // Right
    else if (_x < _xCenter - _deadzone)
        display.print("L");     // Left
    else
        display.print("-");     // Centered

    // --- Button state (O/F) ---
    display.setCursor(x0 + 20, y0 + 20);
    display.print("| ");
    display.print(_btnOutput ? "O" : "F");
  }
  

private:
  // Variables lock in the xpin, ypin, and switch pin, and name
  int xPin, yPin, swPin;
  const char* _name;

  // Variables lock in the x and y analogread and the pressed state
  int _x, _y;

  // Variables lock in xCenter and yCenter and deadzone
  int _xCenter;
  int _yCenter;
  int _deadzone;

  //Button Attributes
  ButtonMode _mode;
  bool _toggleState;
  bool _lastRawPressed;
  bool _btnOutput;
  int _pressedLevel; 
};

class PushButton {
public:
  enum ButtonMode {Momentary, Toggle};
  enum PullMode   { PullUp, PullDown, NoPull };

  PushButton(int sw, const char* name, ButtonMode mode){
    swPin = sw;
    _name = name;

    // Button Attributes
    _mode = mode;
    _toggleState = false;
    _lastRawPressed = false;
    _btnOutput = false;

    // default (will be set properly in begin())
    _pressedLevel = LOW;
  }

  // Method 1: Sets up the pinmode fof the switch
  void begin(PullMode mode = NoPull) {
    if (mode == PullUp) {
        pinMode(swPin, INPUT_PULLUP);
        _pressedLevel = LOW;      // pressed reads LOW
    }
    else if (mode == PullDown) {
        pinMode(swPin, INPUT_PULLDOWN);
        _pressedLevel = HIGH;     // pressed reads HIGH
    }
    else { // NoPull
        pinMode(swPin, INPUT);
        // In this case you MUST have an external resistor
    }
  }

  // Method 2: Reads Button State and Defines Button Type (Momentary vs Toggle)
  bool readButton() {
    // Digital Read of Button
    bool rawPressed = (digitalRead(swPin) == _pressedLevel);

    // Button Behavior Logic
    if (_mode == Momentary) { // Momentary Mode
      _btnOutput = rawPressed;
    }

    else { // Toggle Mode
      if (!_lastRawPressed && rawPressed) {
        _toggleState = !_toggleState;
      }
      _btnOutput = _toggleState;
    }

    _lastRawPressed = rawPressed;

    return _btnOutput;
  }

  // Method 3: Display Button Status on Oled Display
  void oledisplay(Adafruit_SSD1306 &display, int x0, int y0) {

    // --- Button state (O/F) ---
    display.setCursor(x0, y0);
    display.print("But ");
    display.print(_name);
    display.print(_btnOutput ? "O" : "F");
  }

private:
  int swPin;
  const char* _name;

  //Button Attributes
  ButtonMode _mode;
  bool _toggleState;
  bool _lastRawPressed;
  bool _btnOutput;
  int _pressedLevel; 
};

Joystick joy1(25, 33, 32, "1", Joystick::Toggle);
Joystick joy2(14, 27, 26, "2", Joystick::Momentary);

PushButton calBtn1(34, "1", PushButton::Momentary);

void setup() {
  Serial.begin(115200);


  Wire.begin(SDA_PIN, SCL_PIN);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS)) {
    while (true) delay(1000);
  }

  delay(2000);
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print("Joystick Tester");
  display.display();
  delay(2000);

  // Initialize Both Joystick modes
  joy1.begin(120, Joystick::PullUp);
  joy2.begin(120, Joystick::PullUp);

  calBtn1.begin(PushButton::PullDown); 

  // Calibrate the Joysticks
  joy1.calibrate(display);
  joy2.calibrate(display);

}

void loop() {
  display.clearDisplay();

  display.setTextSize(1);
  display.setCursor(10,0);
  display.print("!Joystick Tester!");

  joy1.readButton();
  joy1.joystickRead();
  joy1.drawBlock(display, 0, 18);

  joy2.readButton();
  joy2.joystickRead();
  joy2.drawBlock(display, 70, 18);


  display.display();
}