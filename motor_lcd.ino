#include <AccelStepper.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// --- НАСТРОЙКИ ДВИГАТЕЛЯ ---
// Пины для TMC2208 (режим Step/Dir)
#define STEP_PIN 2
#define DIR_PIN 3
#define EN_PIN 4  // Если ENABLE подключен, иначе можно закомментировать использование

// Параметры двигателя
#define STEPS_PER_REV 200 // Для Nema 17 обычно 200 шагов на оборот (1.8 градуса)
// microsteps зависят от драйвера. Если MS1-MS3 замкнуты на GND/VCC иначе.
// Для простоты считаем 1 шаг = 1 импульс. Если включен микрошаг 1/16, то 3200.
// Здесь предполагаем полный шаг или настройку драйвера под 1 импульс = 1 шаг.
#define SPEED 400.0       // Скорость вращения
#define ACCEL 800.0       // Ускорение

AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);

// --- НАСТРОЙКИ LCD ---
// Адрес 0x27 или 0x3F (зависит от платы), размер 16x2
LiquidCrystal_I2C lcd(0x27, 16, 2); 

// --- СОСТОЯНИЕ ---
unsigned long motorStartTime = 0;
bool isMotorRunning = false;
const unsigned long MOTOR_DURATION = 2000; // 2 секунды в миллисекундах

void setup() {
  // Инициализация Serial
  Serial.begin(9600);
  
  // Инициализация двигателя
  pinMode(EN_PIN, OUTPUT);
  digitalWrite(EN_PIN, LOW); // Активный низкий уровень (включаем драйвер)
  
  stepper.setMaxSpeed(SPEED);
  stepper.setAcceleration(ACCEL);
  
  // Инициализация LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("System Ready");
  lcd.setCursor(0, 1);
  lcd.print("Waiting...");
  
  delay(1000);
  lcd.clear();
}

void loop() {
  // Проверка данных от Raspberry Pi
  if (Serial.available() > 0) {
    char command = Serial.read();
    
    if (command == 's' && !isMotorRunning) {
      startMotorSequence();
    }
  }
  
  // Логика выполнения движения
  if (isMotorRunning) {
    stepper.run(); // Необходимо вызывать постоянно для работы AccelStepper
    
    // Проверяем, прошло ли 2 секунды
    if (millis() - motorStartTime >= MOTOR_DURATION) {
      stopMotorSequence();
    }
  }
}

void startMotorSequence() {
  Serial.println("Command received: Start Motor");
  
  isMotorRunning = true;
  motorStartTime = millis();
  
  // Настройка LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Загрузка");
  // Можно добавить анимацию или прогресс бар, если нужно
  
  // Запуск движения (бесконечное вращение, пока не остановим)
  // Используем большое число шагов или runSpeed, но с AccelStepper лучше задать скорость
  stepper.setSpeed(SPEED);
  // Мы будем использовать runSpeed в цикле, так как нам нужно время, а не расстояние
}

void stopMotorSequence() {
  Serial.println("Sequence finished: Stop Motor");
  
  isMotorRunning = false;
  stepper.stop(); // Плавная остановка (если бы мы использовали moveTo)
  // Так как мы использовали setSpeed/runSpeed, просто обнуляем скорость
  stepper.setSpeed(0);
  
  // Очистка LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Done");
  lcd.setCursor(0, 1);
  lcd.print("Ready");
  
  delay(2000); // Пауза перед следующим ожиданием
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Waiting...");
}
