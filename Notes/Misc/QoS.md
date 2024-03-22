# Quality of service del protocolo MQTT
El QoS se maneja con niveles de calidad de servicio

### QoS 0:
El mensaje se manda como máximo 1 vez
Para lograr esto, se manda el mensaje y no se espera confirmación de que el mensaje fue recibido.

### QoS 1:
El mensaje se manda como mínimo 1 vez
Para lograr esto, se usa un ACK (acknowledgement) que es un mensaje de confirmación de que el mensaje fue recibido. Si no se recibe el ACK, se manda el mensaje de nuevo.

### QoS 2:
El mensaje se manda exactamente 1 vez
Para lograr esto se usa un handshake de 4 pasos. El mensaje se manda, se recibe un mensaje de confirmación, se manda un mensaje de confirmación de la confirmación y se recibe un mensaje de confirmación de la confirmación.

Una de las ventajas del protocolo MQTT es que desacopla la calidad de servicio del protocolo de transporte. Es decir, el protocolo MQTT puede usar TCP/IP, pero también puede usar otros protocolos de transporte como WebSockets, LoRa, etc.

## Comodines del protocolo MQTT
- `+`: Se sustituye por un solo nivel de directorio
- `#`: Se sustituye por cualquier número de niveles de directorio
- `/`: Se usa para separar los niveles de directorio
- `*`: Se usa para sustituir cualquier número de caracteres en un solo nivel de directorio
- `?`: Se usa para sustituir un solo caracter en un solo nivel de directorio
- `^`: Se usa para negar una suscripción

Por ejemplo:
Una suscripción de sensores/+/temperatura se suscribiría a sensores/sala/temperatura, sensores/cocina/temperatura, etc.
Otra sería de sensores/# que se suscribiría a sensores/sala/temperatura, sensores/sala/humedad, etc.

Para poner la calidad de servicio se utiliza el comando `-q` seguido del número de calidad de servicio. Por ejemplo:
mosquitto_sub -t "sensores/+/temperatura" -q 1

Taller:
La bandera -r Retain flag: Si se pone, se manda el último mensaje que se mandó a ese tópico. Si no se pone, no se manda el último mensaje.

-d Debug flag: Muestra mensajes de debug.

-t topic: El tópico al que se quiere suscribir.
-q qos: La calidad de servicio. Puede ser 0, 1 o 2.
-i id: El id del cliente.
-d Debug flag: Muestra mensajes de debug.

