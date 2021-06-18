/**
 * Скетч переключателя по двойному хлопку
 * 
 * При каждом хлопке срабатывает аппаратное прерывание INT0, 
 * управление передается в обработчик clap().
 * 
 * Если промежуток между хлопками
 * не превышает DOUBLE_CLAP_TIMEOUT, то произойдет переключение состояние
 * вывода RELAY_PIN.
 * 
 * Если ничего не происходит IDLE_BEFORE_SLEEP, то МК переходит в режим сна.
 * 
 * Copyright (C) 2021 Alexey Silichenko (a.silichenko@gmail.com)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 * USA
 */
#include <avr/sleep.h>

#define RELAY_PIN PB3
#define BTN_PIN   PB0

// сколько хлопков переключает реле
#define CLAP_NUMBER_SWITCH    2

// отсечение ложных срабатываний от одного хлопка
#define CLAP_DEBOUCE_TIMEOUT  500UL
// в течение этого промежутка времени следующий хлопок засчитывается в серию
#define DOUBLE_CLAP_TIMEOUT   1000UL

// время бездействия до перехода в сон
#define IDLE_BEFORE_SLEEP     10*1000

void setup() {
  pinMode(RELAY_PIN, OUTPUT);
  
  attachInterrupt(0, clap, RISING);

  sei(); // разрешаем прерывания
}

volatile bool relayState = LOW;
volatile long clapTime = 0;

void clap() {
  static long debounceTime = 0;
  static long lastClapTime = 0;
  static byte clapCnt = 0;
  
  clapTime = millis();

  const long debounceInterval = clapTime - debounceTime;
  debounceTime = clapTime;
  if(debounceInterval < CLAP_DEBOUCE_TIMEOUT) return;
      
  const long clapInterval = clapTime - lastClapTime;
  lastClapTime = clapTime;

  // продолжить серию или начать новую
  clapCnt = (clapInterval < DOUBLE_CLAP_TIMEOUT) ? clapCnt + 1 : clapCnt = 1;
  if(CLAP_NUMBER_SWITCH == clapCnt) switchRelay();
}

void switchRelay() {
  relayState = !relayState;
}

void loop() {
  if(millis() - clapTime > IDLE_BEFORE_SLEEP) goSleep();
  digitalWrite(RELAY_PIN, relayState);
}

void goSleep() {
  sleep_enable();   // установили флаг сна
  sleep_cpu();      // уснули
  // <-- вышли из сна по прерыванию
  sleep_disable();  // снимаем флаг сна
}
