import cv2
import cv2.aruco as aruco
import numpy as np
from picamera2 import Picamera2
import time
import serial
import sys

# --- НАСТРОЙКИ ---
ARDUINO_PORT = '/dev/ttyUSB0'  # ЗАМЕНИТЕ на ваш порт (например, '/dev/ttyACM0')
BAUD_RATE = 9600
TARGET_MARKER_ID = 0
COOLDOWN_SECONDS = 2.5  # Чуть больше 2 секунд, чтобы двигатель успел отработать цикл

# Инициализация камеры
picam2 = Picamera2()
config = picam2.create_preview_configuration(main={"size": (640, 480)})
picam2.configure(config)
picam2.start()

# Инициализация словаря ArUco и детектора
aruco_dict = aruco.getPredefinedDictionary(aruco.DICT_6X6_250)
parameters = aruco.DetectorParameters()
detector = aruco.ArucoDetector(aruco_dict, parameters)

# Инициализация последовательного порта
try:
    ser = serial.Serial(ARDUINO_PORT, BAUD_RATE, timeout=1)
    time.sleep(2)  # Ждем перезагрузки Arduino
    print(f"Подключено к {ARDUINO_PORT}")
except serial.SerialException as e:
    print(f"Ошибка подключения к порту {ARDUINO_PORT}: {e}")
    print("Убедитесь, что Arduino подключен и порт указан верно.")
    ser = None

last_trigger_time = 0

print("Нажмите 'q' для выхода. Ожидание маркера ID 0...")

try:
    while True:
        # Получение кадра
        frame = picam2.capture_array()
        
        # Конвертация в оттенки серого для детекции
        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        
        # Детекция маркеров
        corners, ids, rejected = detector.detectMarkers(gray)
        
        found_target = False
        
        if ids is not None:
            for i in range(len(ids)):
                marker_id = ids[i][0]
                
                # Отрисовка рамки и ID
                aruco.drawDetectedMarkers(frame, corners, ids)
                
                if marker_id == TARGET_MARKER_ID:
                    found_target = True
                    current_time = time.time()
                    
                    # Проверка кулдауна, чтобы не спамить сигналами постоянно
                    if current_time - last_trigger_time > COOLDOWN_SECONDS:
                        if ser and ser.is_open:
                            print(f"Маркер ID {TARGET_MARKER_ID} обнаружен! Отправка сигнала...")
                            ser.write(b's')  # Отправляем символ 's'
                            last_trigger_time = current_time
                        else:
                            print("Порт Arduino не открыт!")
                    
                    # Визуальная индикация на экране (зеленая рамка)
                    cv2.putText(frame, "ID 0 FOUND -> SIGNAL SENT", (10, 30), 
                                cv2.FONT_HERSHEY_SIMPLEX, 0.8, (0, 255, 0), 2)
                else:
                    # Обычные маркеры (красная рамка текста)
                    pass 

        # Вывод изображения
        cv2.imshow("Aruco Scanner", frame)
        
        # Выход по клавише 'q'
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

except KeyboardInterrupt:
    print("Остановка программы...")
finally:
    if ser:
        ser.close()
    cv2.destroyAllWindows()
    picam2.stop()
