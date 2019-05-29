/*
    Project     :   Sulit Piso Charge
    Version     :   2.0

    Created by  :   Rhalf Wendel Caacbay
    Email       :   rhalfcaacbay@gmail.com

*/
#include<U8g2lib.h>
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
Timer tDisplay(Timer::MILLIS), tInterrupt(Timer::MILLIS), tLimit(Timer::MILLIS), tPower(Timer::MILLIS);
Terminal terminals[4] = {Terminal(A0), Terminal(A1), Terminal(A2), Terminal(A3)};
Buzzer buzzer = Buzzer(13, 1875, 50);
BillCoinAcceptor coinAcceptor = BillCoinAcceptor(2);
Storage storage = Storage();
Protocol protocol = Protocol(9, 8);
Button buttons[4] = {Button(4), Button(5), Button(6), Button(7)};
Helper helper = Helper();

uint8_t index = 0;
String space = " ";
bool isLimit = false;

void cbLimit() {
  uint32_t amount = storage.getCurrentAmount();
  uint32_t limit = storage.getLimit();
  if (amount >= limit) isLimit = true;
  else isLimit = false;
}

void cbPower() {
  //standby power consumption of device is 3watts therefore
  //float power = 3000.0 / 60.0;
  //power = 50
  storage.incrementPower(50);
}

void cbDisplay() {
  cbLcd12864();
}

void cbLcd12864() {
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_profont11_tr);
    uint16_t x = 8, y = 0;
    if (Timer::getSeconds() < 3) {
      u8g2.drawXBMP( 0, 0, 128, 64, Device::LOGO);
    } else if (Timer::getSeconds() >= 3 && Timer::getSeconds() < 12) {

      u8g2.setCursor(x, 10);
      u8g2.print(Device::getTrans());
      u8g2.print("A");
      u8g2.setCursor( x + 48, 10);
      u8g2.print(storage.getCurrentTransA());

      u8g2.drawUTF8( x, 20, Device::getAmount());
      u8g2.setCursor( x + 48, 20);
      u8g2.print(helper.toUtf8Currency(storage.getCurrentAmount()));

      u8g2.setCursor( x, 30);
      u8g2.print(Device::getTrans());
      u8g2.print("C");
      u8g2.setCursor( x + 48, 30);
      u8g2.print(storage.getCurrentTransC());

      u8g2.drawUTF8( x, 40, Device::getCredit());
      u8g2.setCursor( x + 48, 40);
      u8g2.print(helper.toUtf8Currency(storage.getCurrentCredit()));

      u8g2.drawUTF8( x, 50, Device::getServe());
      u8g2.setCursor( x + 48, 50);
      u8g2.print(helper.toUtf8Time(storage.getCurrentServe()));

      u8g2.drawUTF8( x, 60, Device::getPower());
      u8g2.setCursor( x + 48, 60);
      u8g2.print(storage.getCurrentPower() / 1000.0);

    } else {

      u8g2.setCursor(x, 10);
      u8g2.print(Device::getCoin());

      if (isLimit) {
        if (Timer::getSecondsToggle(1)) u8g2.drawUTF8(x + 30, 10, Device::getLimit());
      }

      u8g2.setCursor(x + 66, 10);
      u8g2.print(helper.toUtf8Currency(coinAcceptor.coinPulse));

      u8g2.setCursor(x, 20);
      u8g2.print(Device::getTime());
      u8g2.setCursor(x + 66, 20);
      u8g2.print(helper.toUtf8Time(coinAcceptor.coinPulse * storage.getRate()));

      u8g2.drawHLine(0, 23, 128);

      //================================================================t0
      u8g2.setCursor(x, 34);
      u8g2.print(Device::getTerminal());
      u8g2.print("1");
      u8g2.setCursor(x + 66, 34);
      if (terminals[0].timeLapse > 0) u8g2.print(helper.toUtf8Time(terminals[0].timeLapse));
      else {
        if (storage.getMode() == 0) u8g2.print(Device::getVacant());
        else u8g2.print(Device::getFree());
      }
      //================================================================t1
      u8g2.setCursor(x, 44);
      u8g2.print(Device::getTerminal());
      u8g2.print("2");
      u8g2.setCursor(x + 66, 44);
      if (terminals[1].timeLapse > 0) u8g2.print(helper.toUtf8Time(terminals[1].timeLapse));
      else {
        if (storage.getMode() == 0) u8g2.print(Device::getVacant());
        else u8g2.print(Device::getFree());
      }
      //================================================================t2
      u8g2.setCursor(x, 54);
      u8g2.print(Device::getTerminal());
      u8g2.print("3");
      u8g2.setCursor(x + 66, 54);
      if (terminals[2].timeLapse > 0) u8g2.print(helper.toUtf8Time(terminals[2].timeLapse));
      else {
        if (storage.getMode() == 0) u8g2.print(Device::getVacant());
        else u8g2.print(Device::getFree());
      }
      //================================================================t3
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
  for (index = 0; index < 4; index++) {
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
      Serial.print(">>" + protocol.buffer);
      protocol.interpret();
      Serial.print("<<" + protocol.buffer);
      protocol.buffer = "";
    }
  }
}

void onReceived(void) {
  Serial.print(">>" + protocol.buffer);
  protocol.interpret();
  Serial.print("<<" + protocol.buffer);
  protocol.print(protocol.buffer);
  protocol.buffer = "";
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
  for (index = 0; index < 4; index++) {
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
      if (storage.getMode() == 0) storage.incrementAmount(coinValue);
      if (storage.getMode() >= 1) storage.incrementCredit(coinValue);

      //charger consumes 20watts per hour
      //float power = (20000.0 / 3600.0) * timeValue;
      //power = 5.56
      storage.incrementPower(6 * timeValue);

      //trigger
      terminals[index].set(timeValue);
      coinAcceptor.coinPulse = 0;
    }
  }
}

void onLongPressed(uint8_t pin) {
  for (index = 0; index < 4; index++) {
    if (buttons[index].getPin() == pin) {
      buzzer.play();
      terminals[index].reset();
    }
  }
}

void setup() {
  // put your setup code here, to run once:

  if (storage.getFirst() != 1) {
    storage.format(190528);
    storage.setFirmware(20);
    storage.setFirst(1);
  }

  buzzer.play();
  Serial.begin(9600);

  protocol.onReceived(onReceived);
  protocol.begin(9600);

  u8g2.begin();
  u8g2.enableUTF8Print();

  coinAcceptor.attach(onCoin);
  protocol.terminals = terminals;

  tDisplay.begin(Timer::FOREVER, 1000, cbDisplay);
  tInterrupt.begin(Timer::FOREVER, 25, cbInterrupt);

  tLimit.begin(Timer::FOREVER, 10000, cbLimit);
  tPower.begin(Timer::FOREVER, 60000, cbPower);

  tDisplay.start();
  tInterrupt.start();

  tLimit.start();
  tPower.start();

  for (index = 0; index < 4; index++) {
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

  tLimit.run();
  tPower.run();

  protocol.run();

  WatchDog::reset();
}
