#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);


#define ENCODER_CLK    14
#define ENCODER_DT     27
#define ENCODER_SW     26
#define MENU_BTN       25
#define BACK_BTN       33
#define PTT_BTN        32
#define OLED_SDA       21
#define OLED_SCL       22

enum Screen {
  BOOT,
  MAIN_MENU,
  FM_RADIO,
  VHF_RADIO,
  SETTINGS,
  FM_SETTINGS,
  ABOUT
};

Screen currentScreen = BOOT;
Screen previousScreen = MAIN_MENU;
unsigned long bootTime = 0;

int menuSelection = 0;
float fmFreq = 98.3f;
float vhfFreq = 145.5f;
int vhfChannel = 1;
bool vhfTX = false;
int settingsSelection = 0;

volatile int encoderPos = 0;
volatile bool encoderPressed = false;

void IRAM_ATTR encoderInterrupt() {
  if (digitalRead(ENCODER_CLK) == digitalRead(ENCODER_DT)) {
    encoderPos++;
  } else {
    encoderPos--;
  }
}

void IRAM_ATTR encoderClick() {
  encoderPressed = true;
}

void setup() {
  Serial.begin(115200);
  
  pinMode(ENCODER_CLK, INPUT_PULLUP);
  pinMode(ENCODER_DT, INPUT_PULLUP);
  pinMode(ENCODER_SW, INPUT_PULLUP);
  pinMode(MENU_BTN, INPUT_PULLUP);
  pinMode(BACK_BTN, INPUT_PULLUP);
  pinMode(PTT_BTN, INPUT_PULLUP);
  
  attachInterrupt(digitalPinToInterrupt(ENCODER_CLK), encoderInterrupt, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_SW), encoderClick, FALLING);
  
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED init failed");
    while (1);
  }
  
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.clearDisplay();
  
  currentScreen = BOOT;
  bootTime = millis();
}

void loop() {
  handleInputs();
  updateScreen();
  delay(50);
}

void handleInputs() {
  static int lastEncoderPos = 0;
  
  if (encoderPos != lastEncoderPos) {
    int delta = encoderPos - lastEncoderPos;
    lastEncoderPos = encoderPos;
    handleEncoderRotation(delta);
  }
  
  if (encoderPressed) {
    encoderPressed = false;
    handleEncoderPress();
    delay(200);
  }
  
  if (!digitalRead(MENU_BTN)) {
    handleMenuButton();
    delay(200);
  }
  
  if (!digitalRead(BACK_BTN)) {
    handleBackButton();
    delay(200);
  }
  
  if (!digitalRead(PTT_BTN)) {
    vhfTX = true;
  } else {
    vhfTX = false;
  }
}

void handleEncoderRotation(int delta) {
  switch (currentScreen) {
    case MAIN_MENU:
      menuSelection = constrain(menuSelection + delta, 0, 2);
      break;
    case FM_RADIO:
      fmFreq = constrain(fmFreq + (delta * 0.1f), 87.5f, 108.0f);
      break;
    case VHF_RADIO:
      vhfFreq = constrain(vhfFreq + (delta * 0.025f), 136.0f, 174.0f);
      break;
    case SETTINGS:
      settingsSelection = constrain(settingsSelection + delta, 0, 3);
      break;
  }
}

void handleEncoderPress() {
  switch (currentScreen) {
    case MAIN_MENU:
      if (menuSelection == 0) currentScreen = FM_RADIO;
      else if (menuSelection == 1) currentScreen = VHF_RADIO;
      else if (menuSelection == 2) currentScreen = SETTINGS;
      break;
    case SETTINGS:
      if (settingsSelection == 3) currentScreen = ABOUT;
      break;
  }
}

void handleMenuButton() {
  if (currentScreen == FM_RADIO) {
    previousScreen = FM_RADIO;
    currentScreen = FM_SETTINGS;
  }
}

void handleBackButton() {
  if (currentScreen == MAIN_MENU) return;
  
  if (currentScreen == FM_SETTINGS) {
    currentScreen = previousScreen;
  } else if (currentScreen == ABOUT) {
    currentScreen = SETTINGS;
  } else {
    currentScreen = MAIN_MENU;
  }
  menuSelection = 0;
  settingsSelection = 0;
}

void updateScreen() {
  display.clearDisplay();
  
  switch (currentScreen) {
    case BOOT:
      drawBoot();
      break;
    case MAIN_MENU:
      drawMainMenu();
      break;
    case FM_RADIO:
      drawFMRadio();
      break;
    case VHF_RADIO:
      drawVHFRadio();
      break;
    case SETTINGS:
      drawSettings();
      break;
    case FM_SETTINGS:
      drawFMSettings();
      break;
    case ABOUT:
      drawAbout();
      break;
  }
  
  display.display();
}

void drawBoot() {
  if (millis() - bootTime > 2000) {
    currentScreen = MAIN_MENU;
    return;
  }
  
  display.setTextSize(2);
  display.setCursor(25, 16);
  display.println("DMR");
  
  display.setTextSize(1);
  display.setCursor(10, 40);
  display.println("Dual Mode Radio");
}

void drawMainMenu() {
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("DMR");
  
  display.setCursor(0, 16);
  if (menuSelection == 0) display.print("> ");
  else display.print("  ");
  display.println("FM RADIO");
  
  display.setCursor(0, 24);
  if (menuSelection == 1) display.print("> ");
  else display.print("  ");
  display.println("VHF RADIO");
  
  display.setCursor(0, 32);
  if (menuSelection == 2) display.print("> ");
  else display.print("  ");
  display.println("SETTINGS");
}

void drawFMRadio() {
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("FM RADIO");
  
  display.setTextSize(2);
  display.setCursor(20, 24);
  display.print(fmFreq, 2);
  display.println(" MHz");
  
  display.setTextSize(1);
  display.setCursor(40, 50);
  display.println("STEREO");
}

void drawVHFRadio() {
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("VHF RADIO");
  
  display.setTextSize(2);
  display.setCursor(10, 24);
  display.print(vhfFreq, 3);
  display.println(" MHz");
  
  display.setTextSize(1);
  display.setCursor(40, 50);
  if (vhfTX) display.println("TX");
  else display.println("RX");
  
  display.setCursor(90, 50);
  display.print("CH ");
  display.println(vhfChannel);
}

void drawSettings() {
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("SETTINGS");
  
  display.setCursor(0, 16);
  if (settingsSelection == 0) display.print("> ");
  else display.print("  ");
  display.println("FM STEP");
  
  display.setCursor(0, 24);
  if (settingsSelection == 1) display.print("> ");
  else display.print("  ");
  display.println("DISPLAY");
  
  display.setCursor(0, 32);
  if (settingsSelection == 2) display.print("> ");
  else display.print("  ");
  display.println("BATTERY");
  
  display.setCursor(0, 40);
  if (settingsSelection == 3) display.print("> ");
  else display.print("  ");
  display.println("ABOUT");
}

void drawFMSettings() {
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("FM OPTIONS");
  
  display.setCursor(0, 20);
  display.println("RDS: OFF");
  display.setCursor(0, 30);
  display.println("VOL: 8");
  display.setCursor(0, 40);
  display.println("MUTE: OFF");
}

void drawAbout() {
  display.setTextSize(1);
  display.setCursor(20, 10);
  display.println("DMR");
  
  display.setCursor(5, 24);
  display.println("Dual Mode Radio");
  
  display.setCursor(0, 38);
  display.println("ESP32 + SA828 + FM");
  
  display.setCursor(50, 50);
  display.println("v1.0");
}
