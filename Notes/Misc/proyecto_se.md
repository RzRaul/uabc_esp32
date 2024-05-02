# Proyecto para el último día de clase
Una aplicación de internet de las cosas

Fecha: 30 de mayo de 2024
Examen: 3 y 23 de  mayo de 2024 6 a 8 pm



- Debe tener por lo menos 3 nodos
    - ej: 3 ESP32
      - 1 Esp32 como access point
      - 2 Esp32 como estaciones
        - 1 protocolo para cada estación
          - La estructura de los mensajes de la comunicación será:
            - Header (por ejemplo 0x18 para la temperatura)
            - Length (tamaño del mensaje en bytes)
            - Datos (ejemplo 25 grados Celsius)
            - Fin de mensaje (por ejemplo 0x0D)
            - CRC (Cyclic Redundancy Check) (4 bytes para verificar la integridad del mensaje)
              - Es un polinomio que se aplica a los datos y verifica la integridad
- Debe tener una aplicación real
- Debe tener una interfaz amigable para el usuario
- Debe considerar las limitaciones de los sistemas embebidos
- Se debe hacer una demo:
  - Explicar el costo del proyecto
  - Explicar la dificultad del proyecto




