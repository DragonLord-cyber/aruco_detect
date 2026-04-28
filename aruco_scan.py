#!/usr/bin/env python3
"""
Скрипт для сканирования ArUco маркеров в реальном времени
используя OpenCV и picamera2
"""

import cv2
import cv2.aruco as aruco
from picamera2 import Picamera2
import numpy as np


def main():
    # Инициализация камеры
    picam2 = Picamera2()
    
    # Конфигурация камеры
    config = picam2.create_preview_configuration(
        main={"size": (640, 480), "format": "RGB888"},
        controls={"FrameRate": 30}
    )
    picam2.configure(config)
    picam2.start()
    
    print("Камера запущена. Нажмите 'q' для выхода.")
    
    # Создание детектора ArUco
    # Используем словарь DICT_6X6_250 (можно изменить на другой)
    aruco_dict = aruco.getPredefinedDictionary(aruco.DICT_6X6_250)
    
    # Параметры детектора
    detector_params = aruco.DetectorParameters()
    detector_params.cornerRefinementMethod = aruco.CORNER_REFINE_SUBPIX
    
    # Создание детектора
    detector = aruco.ArucoDetector(aruco_dict, detector_params)
    
    while True:
        # Захват кадра
        frame = picam2.capture_array()
        
        # Конвертация в оттенки серого для детекции
        gray = cv2.cvtColor(frame, cv2.COLOR_RGB2GRAY)
        
        # Детекция маркеров
        corners, ids, rejected = detector.detectMarkers(gray)
        
        # Если маркеры найдены
        if ids is not None:
            # Отрисовка обнаруженных маркеров
            aruco.drawDetectedMarkers(frame, corners, ids)
            
            # Вывод информации о найденных маркерах
            for i, marker_id in enumerate(ids.flatten()):
                print(f"Найден маркер ID: {marker_id}")
                
                # Опционально: можно добавить оценку позы
                # Для этого нужны калибровочные параметры камеры
        else:
            print("Маркеры не обнаружены")
        
        # Отображение результата
        cv2.imshow('ArUco Marker Detection', frame)
        
        # Проверка нажатия клавиши 'q' для выхода
        key = cv2.waitKey(1) & 0xFF
        if key == ord('q'):
            break
    
    # Освобождение ресурсов
    cv2.destroyAllWindows()
    picam2.stop()
    print("Программа завершена.")


if __name__ == "__main__":
    main()
