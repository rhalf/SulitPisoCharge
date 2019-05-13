#include<U8g2lib.h>
#include<LiquidCrystal_I2C.h>
#include<Timer.h>
#include<Terminal.h>
#include<Buzzer.h>
#include<CoinAcceptor.h>
#include<Storage.h>
#include<Protocol.h>
#include<Button.h>
#include<Device.h>
#include<Helper.h>
#include<WatchDog.h>

U8G2_ST7920_128X64_1_SW_SPI u8g2(U8G2_R0, /* clock=*/ 12, /* data=*/ 11, /* CS=*/ 10, /* reset=*/ 100);
Timer tDisplay(Timer::MILLIS), tInterrupt(Timer::MILLIS);
Terminal terminals[4] = {Terminal(A0), Terminal(A1), Terminal(A2), Terminal(A3)};
Buzzer buzzer = Buzzer();
CoinAcceptor coinAcceptor = CoinAcceptor(2);
Storage storage = Storage();
Protocol protocol = Protocol(9, 8);
Button buttons[4] = {Button(4), Button(5), Button(6), Button(7)};
Helper helper = Helper();
LiquidCrystal_I2C lcd(0x27, 25, 4);

void cbDisplay() {
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
    lcd.print(helper.padding(Device::getTransaction(), " ", 16));
    lcd.setCursor(8, 0);
    lcd.print(storage.getLifetimeTransaction());
    lcd.setCursor(0, 1);
    lcd.print(helper.padding(Device::getGross(), " ", 16));
    lcd.setCursor(8, 1);
    lcd.print(helper.toUtf8Currency(storage.getLifetimeGross()));
  } else {
    lcd.setCursor(0, 0);
    lcd.print(helper.padding("Coin    " + helper.toUtf8Currency(coinAcceptor.coinPulse), " ", 16));
    lcd.setCursor(0, 1);

    if (terminals[0].timeLapse > 0) lcd.print(helper.padding("Term1   " + helper.toUtf8Time(terminals[0].timeLapse), " ", 16));
    else {
      if (storage.getMode() == 0) lcd.print(helper.padding("Term1   Vacant", " ", 16));
      else lcd.print(helper.padding("Term1   Free", " ", 16));
    }
  }

  u8g2.firstPage();
  do {
    uint16_t x = 8, y = 0;
    if (Timer::getSeconds() < 3) {
      u8g2.drawXBMP( 0, 0, 128, 64, Device::LOGO);
    } else if (Timer::getSeconds() >= 3 && Timer::getSeconds() < 9) {
      u8g2.setFont(u8g2_font_baby_tr);
      u8g2.drawUTF8( x, 8, Device::getTransaction());
      u8g2.setCursor( x + 64, 8);
      u8g2.print(storage.getCurrentTransaction());

      u8g2.drawUTF8( x, 16, Device::getGross());
      u8g2.setCursor( x + 64, 16);
      u8g2.print(helper.toUtf8Currency(storage.getCurrentGross()));

      u8g2.drawUTF8( x, 24, Device::getServingTime());
      u8g2.setCursor( x + 64, 24);
      u8g2.print(helper.toUtf8Time(storage.getCurrentServe()));

      u8g2.drawUTF8( x, 32, Device::getCredit());
      u8g2.setCursor( x + 64, 32);
      u8g2.print(helper.toUtf8Currency(storage.getCurrentCredit()));

      u8g2.drawHLine(0, 33, 128);

      u8g2.drawUTF8( x, 40, Device::getTransaction());
      u8g2.setCursor( x + 64, 40);
      u8g2.print(storage.getLifetimeTransaction());

      u8g2.drawUTF8( x, 48, Device::getGross());
      u8g2.setCursor( x + 64, 48);
      u8g2.print(helper.toUtf8Currency(storage.getLifetimeGross()));

      u8g2.drawUTF8( x, 56, Device::getServingTime());
      u8g2.setCursor( x + 64, 56);
      u8g2.print(helper.toUtf8Time(storage.getLifetimeServe()));

      u8g2.drawUTF8( x, 64, Device::getCredit());
      u8g2.setCursor( x + 64, 64);
      u8g2.print(helper.toUtf8Currency(storage.getLifetimeCredit()));

    } else {
      u8g2.setFont(u8g2_font_profont11_tr );
      
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

      uint16_t coinValue = coinAcceptor.coinPulse;
      uint16_t timeValue = coinAcceptor.coinPulse * storage.getRate();

      //add to record
      if (storage.getMode() == 0) storage.incrementGross(coinValue);
      if (storage.getMode() >= 1) storage.incrementCredit(coinValue);
      
      storage.incrementTransaction(1);
      storage.incrementServe(timeValue);
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
  Serial.println(F("SPM001"));

  protocol.setOnReceived(onReceived);
  protocol.begin(9600);

  lcd.init();
  lcd.backlight();

  u8g2.begin();
  u8g2.enableUTF8Print();

  coinAcceptor.attach(onCoin);
  protocol.terminals = terminals;

  tDisplay.begin(Timer::FOREVER, 500, cbDisplay);
  tInterrupt.begin(Timer::FOREVER, 25, cbInterrupt);

  tDisplay.start();
  tInterrupt.start();

  for (uint8_t index = 0; index < 4; index++) {
    //terminals[index].setActiveState(false);
    //buttons[index].setActiveState(false);
    buttons[index].setOnShortPressed(onShortPressed);
    buttons[index].setOnLongPressed(onLongPressed);
  }

  WatchDog::enable(WatchDog::S002);
}

void loop() {
  // put your main code here, to run repeatedly:
  tDisplay.run();
  tInterrupt.run();
  protocol.run();

  WatchDog::reset();
}
