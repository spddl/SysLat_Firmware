#include <Time.h>
#include <LiquidCrystal.h>

LiquidCrystal lcd(8, 6, 4, 0, 1, 9, 5); // syslat custom pcb version

int timeoutCounter = 0;
const byte IOCpin = 7;
bool button_flag = false;
float f_timerTotal = 0;

unsigned long timerBegin, timerTotal;

void setup() {
  lcd.begin(8, 1);
  lcd.setCursor(0, 0);
  Serial.begin(9600); // start serial port at 9600 bps and wait for port to open:

  while (!Serial) {
    lcd.print("Waiting for SysLat");
    delay(350);
    for (int s = 0; s < 27; s++) { // 'waiting for syslat' is 18 characters
      lcd.scrollDisplayLeft();
      delay(400);
      if (Serial) {
        break;
      }
    }
    lcd.clear();
    lcd.setCursor(0, 0);
  }

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(IOCpin, INPUT);

  cli(); // disallow interupts while we set one up...
  attachInterrupt(digitalPinToInterrupt(IOCpin), IOC, RISING);
  sei(); // allow interrupts
}

void IOC() {
  button_flag = true; // Interrupt On Change ISR - as soon as pin goes high, this flag is set
}

//-----------------------------------------------------------BEGIN MAIN-----------------------------------------------------------//
void loop() {
  while (timeoutCounter < 15) {
    timeoutCounter++;
    timeTheFlash();
    
    if (timerTotal > 1) {
      f_timerTotal = static_cast<float>(timerTotal);
      Serial.print(timerTotal);
      lcd.setCursor(0, 0);
      lcd.clear();
      lcd.print(f_timerTotal / 1000);
      timeoutCounter = 0;
      delay(40 + timerTotal / 1000);
    } else {
      if (timeoutCounter == 1) {
        // lcd.clear();
        lcd.print(".");
      } else if ( timeoutCounter < 9) {
        lcd.scrollDisplayRight();
      } else {
        lcd.scrollDisplayLeft();
      }

      delay(100);
    }

    Serial.write("C"); // Finish
    Serial.flush();
  }

  //Reset
  lcd.clear();
  timeoutCounter = 0;
}

void timeTheFlash() {
  int i = 0;
  button_flag = false;
  timerBegin = micros(); // Microseconds
  Serial.write("A"); // White
  while ((!button_flag) && (i < 5000)) {
    _delay_us(100);
    // delayMicroseconds(100);
    i++;
  }
  if (i == 5000) {
    timerTotal = 0;
  } else {
    timerTotal = micros() - timerBegin;
  }
  Serial.write("B"); // Black
}