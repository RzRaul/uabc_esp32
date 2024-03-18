El protocolo MQTT es un protocolo de tipo Publish-Subscribe, es M2M (Machine to Machine) que se ejecuta sobre TCP/IP.

Una de las problemáticas que se presentan es que cada dispositivo tiene su propio protocolo de comunicación, lo que dificulta la comunicación entre dispositivos de diferentes fabricantes. MQTT es un protocolo de comunicación que se ha convertido en un estándar para la comunicación entre dispositivos IoT.

MQTT tiene un modelo donde los sensores y actuadores publican mensajes en un tema (topic) y los dispositivos que están interesados en esos mensajes se suscriben a ese tema. El servidor MQTT es el encargado de enrutar los mensajes a los dispositivos suscritos a ese tema en particular. Este modelo permite que los dispositivos no tengan que conocerse entre sí, lo que facilita la comunicación entre dispositivos de diferentes fabricantes.

### Broker
El broker se encarga de enrutar los mensajes a los dispositivos suscritos a un tema en particular. El broker es el intermediario entre los dispositivos que publican mensajes y los dispositivos que están suscritos a esos mensajes.

### Topics
Los mensajes se publican en un tema (topic) y los dispositivos interesados en esos mensajes se suscriben a ese tema. Los temas son cadenas de texto que permiten filtrar los mensajes que se desean recibir.

### Calidad de Servicio
La calidad de servicio (QoS) es una característica del protocolo MQTT que permite garantizar la entrega de los mensajes. El protocolo MQTT soporta tres niveles de calidad de servicio:

### Retained Messages
Los mensajes retenidos son mensajes que se almacenan en el broker y se envían a los dispositivos que se suscriban a ese tema en particular. Los mensajes retenidos son útiles para enviar información que no cambia con frecuencia, como el estado de un dispositivo.

Es un protocolo basado en Eventos, por lo que el consumo de energía es menor
