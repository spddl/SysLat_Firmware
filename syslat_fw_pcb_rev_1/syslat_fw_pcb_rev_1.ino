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
  while (timeoutCounter < 15) {
    timeoutCounter++;
    unsigned long timerTotal = timeTheFlash();

    if (timerTotal > 1) {
      Serial.print(timerTotal); // send µs
      lcd.setCursor(0, 0);
      lcd.clear();
      lcd.print(static_cast<float>(timerTotal) / 1000); // show ms
      timeoutCounter = 0;
      delay(40 + timerTotal / 1000); // here we need a value that is above the maximum latency, 40 seems to be ok
    } else {
      if (timeoutCounter == 1) {
        lcd.clear();
        lcd.print(".");
      } else if (timeoutCounter < 9) {
        lcd.scrollDisplayRight();
      } else {
        lcd.scrollDisplayLeft();
      }

      delay(200);
    }

    Serial.write("C"); // Finish
    Serial.flush();
  }

  //Reset
  lcd.clear();
  timeoutCounter = 0;
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