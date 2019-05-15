#include<U8g2lib.h>
#include<LiquidCrystal_I2C.h>
#include<Timer.h>
#include<Terminal.h>
#include<Buzzer.h>
#include<BillCoinAcceptor.h>
#include<Storage.h>
#include<Protocol.h>
#include<Button.h>
#include<Device.h>
#include<Helper.h>
#include<WatchDog.h>

U8G2_ST7920_128X64_1_SW_SPI u8g2(U8G2_R0, /* clock=*/ 12, /* data=*/ 11, /* CS=*/ 10, /* reset=*/ 100);
Timer tLcd1602(Timer::MILLIS), tLcd12864(Timer::MILLIS), tInterrupt(Timer::MILLIS), tMinute(Timer::MILLIS);
Terminal terminals[4] = {Terminal(A0), Terminal(A1), Terminal(A2), Terminal(A3)};
Buzzer buzzer = Buzzer();
BillCoinAcceptor coinAcceptor = BillCoinAcceptor(2);
Storage storage = Storage();
Protocol protocol = Protocol(9, 8);
Button buttons[4] = {Button(4), Button(5), Button(6), Button(7)};
Helper helper = Helper();
LiquidCrystal_I2C lcd(0x27, 25, 4);

void cbMinute() {
  //standby power consumption of device is 3watts therefore
  float result = 5000.0 / 60.0;
  storage.incrementPower((uint32_t)result);
}

void cbLcd1602() {
  if (Timer::getSeconds() < 3) {
    lcd.setCursor(0, 0);
    lcd.print(helper.padding(Device::getCompany(), " ", 16));
    lcd.setCursor(0, 1);
    lcd.print(helper.padding(Device::getCountry(), " ", 16));
  } else if (Timer::getSeconds() >= 3 && Timer::getSeconds() < 6) {
    lcd.setCursor(0, 0);
    lcd.print(helper.padding(Device::getTransaction(), " ", 16));
    lcd.setCursor(8, 0);
    lcd.print(storage.getCurrentTransaction());
    lcd.setCursor(0, 1);
    lcd.print(helper.padding(Device::getGross(), " ", 16));
    lcd.setCursor(8, 1);
    lcd.print(helper.toUtf8Currency(storage.getCurrentGross()));
  }  else if (Timer::getSeconds() >= 6 && Timer::getSeconds() < 9) {
    lcd.setCursor(0, 0);
    lcd.print(helper.padding(Device::getServingTime(), " ", 16));
    lcd.setCursor(8, 0);
    lcd.print(helper.padding(helper.toUtf8Time(storage.getCurrentServe()), " ", 16));
    lcd.setCursor(0, 1);
    lcd.print(helper.padding(Device::getCredit(), " ", 16));
    lcd.setCursor(8, 1);
    lcd.print(helper.toUtf8Currency(storage.getCurrentCredit()));
  } else if (Timer::getSeconds() >= 9 && Timer::getSeconds() < 12) {
    lcd.setCursor(0, 0);
    lcd.print(helper.padding(Device::getPower(), " ", 16));
    lcd.setCursor(8, 0);
    lcd.print(storage.getCurrentPower());
    lcd.setCursor(0, 1);
    lcd.print(helper.padding("", " ", 16));

  } else {
    lcd.setCursor(0, 0);
    lcd.print(helper.padding(Device::getCoin(), " ", 16));
    lcd.setCursor(8, 0);
    lcd.print(helper.padding(helper.toUtf8Currency(coinAcceptor.coinPulse), " ", 16));


    if (terminals[0].timeLapse > 0) {
      lcd.setCursor(0, 1);
      lcd.print(helper.padding(Device::getTime(), " ", 16));
      lcd.setCursor(8, 1);
      lcd.print(helper.padding(helper.toUtf8Time(terminals[0].timeLapse), " ", 16));

    } else {
      if (storage.getMode() == 0) {
        if (coinAcceptor.coinPulse > 0) {
          lcd.setCursor(0, 1);
          lcd.print(helper.padding(Device::getTime(), " ", 16));
          lcd.setCursor(8, 1);
          lcd.print(helper.padding(helper.toUtf8Time(coinAcceptor.coinPulse * storage.getRate()), " ", 16));
        } else {
          lcd.setCursor(0, 1);
          lcd.print(helper.padding(Device::getTime(), " ", 16));
          lcd.setCursor(8, 1);
          lcd.print(helper.padding(Device::getVacant(), " ", 16));
        }
      } else {
        lcd.setCursor(0, 1);
        lcd.print(helper.padding(Device::getTime(), " ", 16));
        lcd.setCursor(8, 1);
        lcd.print(helper.padding(Device::getFree(), " ", 16));
      }
    }
  }
}

void cbLcd12864() {
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_profont11_tr);
    uint16_t x = 8, y = 0;
    if (Timer::getSeconds() < 3) {
      u8g2.drawXBMP( 0, 0, 128, 64, Device::LOGO);
    } else if (Timer::getSeconds() >= 3 && Timer::getSeconds() < 12) {

      u8g2.drawUTF8( 40, 10, Device::getCurrent());

      u8g2.drawHLine(0, 12, 128);

      u8g2.drawUTF8( x, 22, Device::getTransaction());
      u8g2.setCursor( x + 48, 22);
      u8g2.print(storage.getCurrentTransaction());

      u8g2.drawUTF8( x, 32, Device::getGross());
      u8g2.setCursor( x + 48, 32);
      u8g2.print(helper.toUtf8Currency(storage.getCurrentGross()));

      u8g2.drawUTF8( x, 42, Device::getServingTime());
      u8g2.setCursor( x + 48, 42);
      u8g2.print(helper.toUtf8Time(storage.getCurrentServe()));

      u8g2.drawUTF8( x, 52, Device::getCredit());
      u8g2.setCursor( x + 48, 52);
      u8g2.print(helper.toUtf8Currency(storage.getCurrentCredit()));

      u8g2.drawUTF8( x, 62, Device::getPower());
      u8g2.setCursor( x + 48, 62);
      u8g2.print(storage.getCurrentPower());

    } else {

      u8g2.setCursor(x, 10);
      u8g2.print(Device::getCoin());
      u8g2.setCursor(x + 66, 10);
      u8g2.print(helper.toUtf8Currency(coinAcceptor.coinPulse));
      u8g2.setCursor(x, 20);
      u8g2.print(Device::getTime());
      u8g2.setCursor(x + 66, 20);
      u8g2.print(helper.toUtf8Time(coinAcceptor.coinPulse * storage.getRate()));

      u8g2.drawHLine(0, 23, 128);

      u8g2.setCursor(x, 34);
      u8g2.print(Device::getTerminal());
      u8g2.print("1");
      u8g2.setCursor(x + 66, 34);

      if (terminals[0].timeLapse > 0) u8g2.print(helper.toUtf8Time(terminals[0].timeLapse));
      else {
        if (storage.getMode() == 0) u8g2.print(Device::getVacant());
        else u8g2.print(Device::getFree());
      }

      u8g2.setCursor(x, 44);
      u8g2.print(Device::getTerminal());
      u8g2.print("2");
      u8g2.setCursor(x + 66, 44);

      if (terminals[1].timeLapse > 0) u8g2.print(helper.toUtf8Time(terminals[1].timeLapse));
      else {
        if (storage.getMode() == 0) u8g2.print(Device::getVacant());
        else u8g2.print(Device::getFree());
      }

      u8g2.setCursor(x, 54);
      u8g2.print(Device::getTerminal());
      u8g2.print("3");
      u8g2.setCursor(x + 66, 54);

      if (terminals[2].timeLapse > 0) u8g2.print(helper.toUtf8Time(terminals[2].timeLapse));
      else {
        if (storage.getMode() == 0) u8g2.print(Device::getVacant());
        else u8g2.print(Device::getFree());
      }

      u8g2.setCursor(x, 64);
      u8g2.print(Device::getTerminal());
      u8g2.print("4");
      u8g2.setCursor(x + 66, 64);

      if (terminals[3].timeLapse > 0) u8g2.print(helper.toUtf8Time(terminals[3].timeLapse));
      else {
        if (storage.getMode() == 0) u8g2.print(Device::getVacant());
        else u8g2.print(Device::getFree());
      }
    }
  } while ( u8g2.nextPage() );
}

void cbInterrupt() {
  for (uint8_t index = 0; index < 4; index++) {
    //Buttons
    buttons[index].run();
    //Terminals
    terminals[index].run();
  }
}

void serialEvent() {
  if (Serial.available()) {
    char chr = (char) Serial.read();
    protocol.buffer += chr;
    if (chr == '\n') {
      Serial.print("<<" + protocol.buffer);
      protocol.interpret();
      Serial.print(">>" + protocol.buffer);
      protocol.buffer = "";
    }
  }
}

void onReceived(void) {
  Serial.print("<<" + protocol.buffer);
  protocol.interpret();
  protocol.send();
  Serial.print(">>" + protocol.buffer);
}

void onCoin() {
  buzzer.play();
  coinAcceptor.readCoinPulse();
}

void onShortPressed(uint8_t pin) {
  coinAcceptor.coinPulse += storage.getMode();

  //check if coin inserted
  if (coinAcceptor.coinPulse == 0 ) return;
  //Process
  for (uint8_t index = 0; index < 4; index++) {
    if (buttons[index].getPin() == pin) {
      // check if money is minimum
      if (!terminals[index].getState())
        if (coinAcceptor.coinPulse < storage.getMinimum())
          continue;
      //process
      buzzer.play();

      uint32_t coinValue = coinAcceptor.coinPulse;
      uint32_t timeValue = coinAcceptor.coinPulse * storage.getRate();

      //add to record
      if (storage.getMode() == 0) storage.incrementGross(coinValue);
      if (storage.getMode() >= 1) storage.incrementCredit(coinValue);

      storage.incrementTransaction(1);
      storage.incrementServe(timeValue);


      //charger consumes 20watts per hour
      float power = (20000.0 / 3600.0) * timeValue;
      storage.incrementPower((uint32_t)power);
      
      //trigger
      terminals[index].set(timeValue);
      coinAcceptor.coinPulse = 0;
    }
  }
}

void onLongPressed(uint8_t pin) {
  for (uint8_t index = 0; index < 4; index++) {
    if (buttons[index].getPin() == pin) {
      buzzer.play();
      terminals[index].reset();
    }
  }
}

void setup() {
  // put your setup code here, to run once:
  buzzer.play();
  Serial.begin(9600);

  protocol.setOnReceived(onReceived);
  protocol.begin(9600);

  lcd.init();
  lcd.backlight();

  u8g2.begin();
  u8g2.enableUTF8Print();

  coinAcceptor.attach(onCoin);
  protocol.terminals = terminals;

  tLcd1602.begin(Timer::FOREVER, 1000, cbLcd1602);
  tLcd12864.begin(Timer::FOREVER, 1000, cbLcd12864);
  tInterrupt.begin(Timer::FOREVER, 25, cbInterrupt);
  tMinute.begin(Timer::FOREVER, 60000, cbMinute);

  tLcd1602.start();
  tLcd12864.start();
  tInterrupt.start();
  tMinute.start();

  for (uint8_t index = 0; index < 4; index++) {
    //terminals[index].setActiveState(false);
    //buttons[index].setActiveState(false);
    buttons[index].setOnShortPressed(onShortPressed);
    buttons[index].setOnLongPressed(onLongPressed);
  }

  WatchDog::enable(WatchDog::S004);
}

void loop() {
  // put your main code here, to run repeatedly:
  tLcd1602.run();
  tLcd12864.run();
  tInterrupt.run();
  tMinute.run();

  protocol.run();

  WatchDog::reset();
}
