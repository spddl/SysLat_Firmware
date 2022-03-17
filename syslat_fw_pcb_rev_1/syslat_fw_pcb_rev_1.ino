#include <Time.h>
#include <LiquidCrystal.h>

LiquidCrystal lcd(8, 6, 4, 0, 1, 9, 5); // syslat custom pcb version

const byte IOCpin = 7;
bool button_flag = false;

void setup() {
  lcd.begin(8, 1);
  lcd.setCursor(0, 0);
  Serial.begin(9600); // start serial port at 9600 bps and wait for port to open:

  while (!Serial) {
    lcd.print("Waiting for GoSysLat");
    delay(350);
    for (int s = 0; s < 20; s++) { // 'Waiting for GoSysLat' is 20 characters
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
  int timeoutCounter = 0;
  while (timeoutCounter < 14) {
    timeoutCounter++;
    unsigned long timerTotal = timeTheFlash();

    if (timerTotal > 1) {
      Serial.print(timerTotal); // send µs
      lcd.setCursor(0, 0);
      lcd.clear();
      lcd.print(timerTotal / 1000.0, 3); // show ms

      int delayTime = 40 + timerTotal / 1000;
      for (int i = 0; digitalRead(IOCpin) != LOW && i < delayTime; i++) {
        _delay_ms(1);
      }

      timeoutCounter = 0;
    } else {
      if (timeoutCounter == 1) {
        lcd.clear();
        lcd.print(".");
      } else if (timeoutCounter < 9) {
        lcd.scrollDisplayRight();
      } else {
        lcd.scrollDisplayLeft();
      }
      delay(500);
    }

    Serial.write("C"); // Finish
    Serial.flush();
  }
}

unsigned long timeTheFlash() {
  int i = 0;
  button_flag = false;
  unsigned long timerTotal;
  unsigned long timerBegin = micros(); // Microseconds
  Serial.write("A"); // White
  while (!button_flag && i < 5000) { // 0,5 Sec Timeout
    _delay_us(100);
    i++;
  }

  if (!button_flag) {
    timerTotal = 0;
  } else {
    timerTotal = micros() - timerBegin;
  }
  Serial.write("B"); // Black
  return timerTotal;
}