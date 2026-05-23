--- arduino_motor_lcd.ino (原始)


+++ arduino_motor_lcd.ino (修改后)
/*
 * Код для Arduino Uno
 * Управление шаговым двигателем Nema 17 через TMC2208 V2.0
 * Вывод сообщения на LCD дисплей через I2C
 *
 * Необходимые библиотеки:
 * 1. AccelStepper (для управления шаговым двигателем)
 * 2. LiquidCrystal_I2C (для LCD дисплея)
 *
 * Установка библиотек в Arduino IDE:
 * - Sketch -> Include Library -> Manage Libraries
 * - Найти и установить "AccelStepper"
 * - Найти и установить "LiquidCrystal I2C"
 */

#include <AccelStepper.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Настройки LCD дисплея (адрес 0x27 или 0x3F, размер 16x2)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Настройки шагового двигателя
// Определение пинов для TMC2208
#define STEP_PIN 2
#define DIR_PIN 3
#define ENABLE_PIN 4

// Создание объекта шагового двигателя
AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);

// Переменные для управления временем
unsigned long motorStartTime = 0;
bool motorRunning = false;

// Флаг условия запуска (замените на ваше условие)
bool triggerCondition = false;

void setup() {
  // Инициализация последовательного порта для отладки
  Serial.begin(9600);

  // Инициализация LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("System Ready");
  lcd.setCursor(0, 1);
  lcd.print("Waiting...");

  // Настройка пинов
  pinMode(ENABLE_PIN, OUTPUT);
  digitalWrite(ENABLE_PIN, LOW); // Активный низкий уровень для включения драйвера

  // Настройка шагового двигателя
  stepper.setMaxSpeed(1000.0);      // Максимальная скорость (шагов/сек)
  stepper.setAcceleration(500.0);   // Ускорение (шагов/сек^2)
  stepper.setSpeed(200.0);          // Начальная скорость

  Serial.println("System initialized");
}

void loop() {
  // ============================================
  // ЗАМЕНИТЕ ЭТО УСЛОВИЕ НА ВАШЕ СОБСТВЕННОЕ
  // ============================================
  // Пример условия: если на пине 5 высокий уровень
  // if (digitalRead(5) == HIGH) {
  //   triggerCondition = true;
  // }

  // Временное условие для тестирования (нажмите 's' в серийном мониторе)
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    if (cmd == 's' || cmd == 'S') {
      triggerCondition = true;
      Serial.println("Condition triggered!");
    }
  }

  // Проверка условия запуска
  if (triggerCondition && !motorRunning) {
    startMotorSequence();
  }

  // Управление двигателем во время вращения
  if (motorRunning) {
    stepper.runSpeed();

    // Проверка истечения времени (2 секунды)
    if (millis() - motorStartTime >= 2000) {
      stopMotorSequence();
    }
  }

  // Обновление положения двигателя (необходимо вызывать постоянно)
  // stepper.run() используется только если задана целевая позиция
}

void startMotorSequence() {
  Serial.println("Starting motor sequence...");

  motorRunning = true;
  motorStartTime = millis();

  // Включение двигателя (активный низкий)
  digitalWrite(ENABLE_PIN, LOW);

  // Установка скорости вращения
  stepper.setSpeed(200.0); // 200 шагов в секунду

  // Вывод сообщения на LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Загрузка");
  lcd.setCursor(0, 1);
  lcd.print("Loading...");

  Serial.println("Motor running for 2 seconds");
}

void stopMotorSequence() {
  Serial.println("Stopping motor sequence...");

  motorRunning = false;
  triggerCondition = false; // Сброс условия

  // Остановка двигателя
  stepper.stop();

  // Выключение двигателя (опционально, можно оставить включенным)
  // digitalWrite(ENABLE_PIN, HIGH); // Отключить драйвер

  // Возврат LCD в исходное состояние
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Complete!");
  lcd.setCursor(0, 1);
  lcd.print("System Ready");

  Serial.println("Sequence complete");

  delay(1000); // Небольшая задержка перед следующим циклом

  // Очистка экрана и возврат в режим ожидания
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("System Ready");
  lcd.setCursor(0, 1);
  lcd.print("Waiting...");
}
