Los dispositivos que trabajan con 7 bits trabajan siempre con paquetes de 8 bits. son 7 de dirección y 1 de operación (lectura o escritura). El bit de operación se coloca en el bit 0 del byte de dirección. El bit 0 se coloca a 1 para lectura y a 0 para escritura. El bit 1 del byte de dirección se coloca a 1 si el dispositivo tiene la dirección 0x40-0x4F y a 0 si tiene la dirección 0x20-0x27.

Las direcciones de 10 bits se mandan en dos paquetes de 8 bits. El primer byte tiene la dirección 11110XX, donde XX son los bits más significativos de la dirección. El segundo byte tiene la dirección 1111110X, donde X es el bit menos significativo de la dirección. El bit 0 del primer byte se coloca a 1 para lectura y a 0 para escritura.

Con 7 bits la cantidad máxima de dispositivos es 112

### Protocolo
#### Nota importante, por cada byte de datos enviado el dispositivo debe responder con un ACK. Si el dispositivo no responde con un ACK el controlador debe detener la transmisión y reiniciar el bus.

##### El proceso para escribir en un dispositivo I2C es el siguiente:
1. El controlador envía la dirección del dispositivo
2. Envía un 8vo bit para indicar si es una operación de lectura o escritura
3. El dispositivo responde con un ACK
4. El controlador envía qué registro del dispositivo quiere escribir
5. El dispositivo responde con un ACK
6. El controlador envía el byte de datos (o los bytes de datos)
7. El dispositivo responde con un ACK (por cada byte de datos)

##### El proceso para leer de un dispositivo I2C es el siguiente:
1. El controlador envía la dirección del dispositivo
2. El controlador pone el bit de operación a 0
3. El dispositivo responde con un ACK
4. El controlador envía qué registro del dispositivo quiere leer
5. El dispositivo responde con un ACK
6. El controlador envía una condición de reinicio repetida
7. El controlador envía la dirección del dispositivo (7 bits)
8. El controlador pone el bit de operación a 1 (Read)
9. El dispositivo responde con un ACK
10. El dispositivo envía el byte de datos
11. El controlador responde con un ACK (por cada byte de datos)
12. El dispositivo envía un NACK

##### Las condiciones del protocolo son:
Las condiciones de inicio y fin son las únicas que se hacen con el bus en alto. El bus en alto es el estado de reposo del bus.



