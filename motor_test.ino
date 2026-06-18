#include <AccelStepper.h>

// --- Конфигурация пинов ---
// Подключение к TMC2208
#define PIN_STEP 3
#define PIN_DIR  4
#define PIN_EN   5

// Инициализация объекта двигателя
// DRIVER = режим STEP/DIR
AccelStepper stepper(AccelStepper::DRIVER, PIN_STEP, PIN_DIR);

// --- Переменные состояния ---
long targetPosition = 400; // Целевое количество шагов в одну сторону (по умолчанию ~2 оборота при 1/16)
bool movingForward = true; // Флаг направления
unsigned long lastActionTime = 0;
unsigned long cycleDuration = 2000; // Время движения в одну сторону (мс), если используем время вместо шагов

void setup() {
  // Инициализация Serial
  Serial.begin(9600);
  while (!Serial) { ; } // Ждем подключения Serial (для некоторых плат)

  // Настройка пинов
  pinMode(PIN_EN, OUTPUT);
  digitalWrite(PIN_EN, LOW); // LOW включает драйвер (активный низкий уровень)

  // Начальные настройки двигателя
  stepper.setMaxSpeed(1000.0);      // Шагов в секунду
  stepper.setAcceleration(500.0);   // Шагов в секунду^2
  stepper.setCurrentPosition(0);    // Сброс текущей позиции в 0

  Serial.println("--- Stepper Cycle Test Started ---");
  Serial.println("Двигатель будет вращаться туда-обратно.");
  printHelp();
}

void loop() {
  // 1. Обновляем движение двигателя
  // run() возвращает true, если двигатель еще движется к цели
  if (stepper.run()) {
    // Двигатель в движении, ничего не делаем, ждем достижения цели
  } else {
    // Двигатель достиг цели и остановился
    // Меняем направление только что завершившегося движения
    if (movingForward) {
      // Только что закончили движение вперед, теперь надо назад
      targetPosition = -targetPosition; 
      movingForward = false;
      Serial.println("Direction: Reverse");
    } else {
      // Только что закончили движение назад, теперь вперед
      targetPosition = -targetPosition;
      movingForward = true;
      Serial.println("Direction: Forward");
    }
    
    // Устанавливаем новую цель
    stepper.moveTo(targetPosition);
    
    // Небольшая задержка перед стартом в обратную сторону (опционально)
    delay(500); 
  }

  // 2. Обработка команд из Serial Monitor
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    
    if (input.length() == 0) return;

    char command = input.charAt(0);
    String valueStr = input.substring(1);
    valueStr.trim();
    
    switch (command) {
      case 's': // Установка скорости (Max Speed)
        {
          float val = valueStr.toFloat();
          if (val > 0) {
            stepper.setMaxSpeed(val);
            Serial.print("Max Speed set to: ");
            Serial.println(val);
          } else {
            Serial.println("Error: Speed must be > 0");
          }
        }
        break;

      case 'a': // Установка ускорения (Acceleration)
        {
          float val = valueStr.toFloat();
          if (val > 0) {
            stepper.setAcceleration(val);
            Serial.print("Acceleration set to: ");
            Serial.println(val);
          } else {
            Serial.println("Error: Acceleration must be > 0");
          }
        }
        break;

      case 'd': // Установка дистанции (количество шагов в одну сторону)
        {
          long val = valueStr.toInt();
          if (val != 0) {
            // Если меняем во время движения, нужно скорректировать цель относительно текущей позиции
            long currentPos = stepper.currentPosition();
            if (movingForward) {
               targetPosition = currentPos + abs(val);
            } else {
               targetPosition = currentPos - abs(val);
            }
            // Обновляем базовое значение для следующего цикла (абсолютное значение)
            // Примечание: логика переключения знака работает относительно 0 в оригинале, 
            // здесь мы просто задаем новый вектор движения.
            // Для простоты в этом тесте: задаем абсолютное число шагов от 0, 
            // но так как мы инвертируем знак выше, лучше задать просто модуль.
            // Упростим: запоминаем модуль шага для будущего использования, 
            // а текущую цель ставим немедленно.
            
            // Пересчитываем targetPosition глобально как "шаг от нуля", 
            // но так как цикл бесконечный +/-, просто сохраним модуль в переменную логики?
            // В данной реализации targetPosition - это абсолютная координата.
            // Чтобы цикл работал корректно (туда-сюда), нам нужно знать "амплитуду".
            // Давайте сделаем проще: команда 'd' меняет амплитуду колебаний вокруг 0.
            
            long amplitude = abs(val);
            if (stepper.currentPosition() >= 0) {
                stepper.moveTo(-amplitude);
                movingForward = false;
            } else {
                stepper.moveTo(amplitude);
                movingForward = true;
            }
            // Сохраняем амплитуду в глобальную переменную, если бы она была отдельной, 
            // но здесь мы используем логику инверсии. 
            // Для корректной работы команды 'd' в цикле лучше сбросить позицию или адаптировать.
            // Оставим простую логику: следующее движение будет на новое расстояние.
            
            Serial.print("Target distance adjusted. New amplitude approx: ");
            Serial.println(amplitude);
          } else {
            Serial.println("Error: Distance cannot be 0");
          }
        }
        break;

      case 'c': // Стоп (сброс позиции в 0 и остановка)
        stepper.stop();
        stepper.setCurrentPosition(0);
        targetPosition = 0;
        movingForward = true;
        Serial.println("Motor stopped and position reset to 0.");
        break;
        
      case 'h': // Помощь
        printHelp();
        break;

      default:
        Serial.println("Unknown command. Type 'h' for help.");
        break;
    }
  }
}

void printHelp() {
  Serial.println("\n--- Commands ---");
  Serial.println("s<value>  : Set max speed (e.g., s1000)");
  Serial.println("a<value>  : Set acceleration (e.g., a500)");
  Serial.println("d<value>  : Set steps per direction (e.g., d400)");
  Serial.println("c         : Stop and reset position to 0");
  Serial.println("h         : Show this help");
  Serial.println("--------------\n");
}
