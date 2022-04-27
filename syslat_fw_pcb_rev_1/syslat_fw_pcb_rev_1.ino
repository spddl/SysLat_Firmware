#include <Time.h>
#include <LiquidCrystal.h>

LiquidCrystal lcd(8, 6, 4, 0, 1, 9, 5); // syslat custom pcb version

const byte IOCpin = 7;
bool button_flag = false;

// default values
bool Conf_LCD = true;
bool Conf_Detect_Light = true;

void setup() {
  lcd.begin(8, 1);
  lcd.clear();
  Serial.begin(9600);

  while (!Serial) {
    lcd.print("Waiting for GoSysLat");
    delay(350);
    for (int s = 0; s < 20; s++) { // 'Waiting for GoSysLat' is 20 characters
      lcd.scrollDisplayLeft();
      delay(500);
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
    unsigned long timerTotal = timeTheFlash(); // 0,5 Sec Timeout

    if (timerTotal > 1) {
      SendLatencyToClient(timerTotal);

      if (Conf_LCD) {
        ShowLatencyOnLCD(timerTotal);
      }

      Serial.write(3); // Finish (C)

      int delayTime = 50 + timerTotal / 1000; // here we need a value that is above the maximum latency, 50 seems to be ok
      for (int i = 0; digitalRead(IOCpin) != LOW && i < delayTime; i++) {
        _delay_ms(1);
      }
      timeoutCounter = 0;
    } else {
      Serial.write(3); // Finish (C)

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

    if (Serial.available() > 0) {
      SetSetting(Serial.read());
    }

    Serial.flush();
  }
}

unsigned long timeTheFlash() {
  int i = 0;
  button_flag = false;
  unsigned long timerTotal;
  unsigned long timerBegin = micros(); // Microseconds
  Serial.write(1); // White (A)

  for (int i = 0; !button_flag && i < 5000; i++) { // 0,5 Sec Timeout
    _delay_us(100);
  }

  if (!button_flag) {
    timerTotal = 0;
  } else {
    timerTotal = micros() - timerBegin;
  }
  Serial.write(2); // Black (B)
  return timerTotal;
}

unsigned long timeTheTrigger() {
  lcd.clear();
  lcd.print("Ready!");
  while (true) {
    if (Serial.available() > 0) {
      int data = Serial.read();
      if (data == 5) {
        lcd.clear();
        break;
      } else if (data != 0) {
        continue;
      }
      int i = 0;
      button_flag = false;
      unsigned long timerTotal;
      unsigned long timerBegin = micros(); // Microseconds
      for (int i = 0; !button_flag && i < 5000; i++) { // 0,5 Sec Timeout
        _delay_us(100);
      }

      if (!button_flag) {
        timerTotal = 0;
      } else {
        timerTotal = micros() - timerBegin;
      }

      ShowLatencyOnLCD(timerTotal);

      SendLatencyToClient(timerTotal);

      Serial.write(3); // Finish (C)
      Serial.flush();

      delay(500);
      lcd.clear();
      lcd.print("Ready!");
    }
  }
}

void ShowLatencyOnLCD(unsigned long timerTotal) {
  lcd.clear();
  float timerTotalFloat = timerTotal / 1000.0;
  if (100 < timerTotalFloat) {
    lcd.print(" ");
  } else if (10 < timerTotalFloat) {
    lcd.print("  ");
  } else {
    lcd.print("   ");
  }
  lcd.print(timerTotalFloat, 3); // show ms
}

void SendLatencyToClient(unsigned long timerTotal) {
  if (65535 > timerTotal) {
    byte buf[2];
    buf[0] = timerTotal & 255;
    buf[1] = (timerTotal >> 8) & 255;
    Serial.write(buf, 2); // send the latency time as bytes in µs
  } else {
    byte buf[3];
    buf[0] = timerTotal & 255;
    buf[1] = (timerTotal >> 8) & 255;
    buf[2] = (timerTotal >> 16) & 255;
    Serial.write(buf, 3); // send the latency time as bytes in µs
  }
}

void SetSetting(int id) {
  switch (id) {
    case 1: // LCD ON
      if (!Conf_LCD) {
        Conf_LCD = true;
      }
      break;
    case 2: // LCD OFF
      if (Conf_LCD) {
        Conf_LCD = false;
        lcd.clear();
      }
      break;

    case 3: // Detect white light (Black > White)
      if (!Conf_Detect_Light) {
        Conf_Detect_Light = true;
        cli();
        detachInterrupt(digitalPinToInterrupt(IOCpin));
        attachInterrupt(digitalPinToInterrupt(IOCpin), IOC, RISING);
        sei();
      }
      break;
    case 4: // Detect no light (White > Black)
      if (Conf_Detect_Light) {
        Conf_Detect_Light = false;
        cli();
        detachInterrupt(digitalPinToInterrupt(IOCpin));
        attachInterrupt(digitalPinToInterrupt(IOCpin), IOC, FALLING);
        sei();
      }
      break;

    case 5: // Trigger
      timeTheTrigger();
      break;
  }
}
