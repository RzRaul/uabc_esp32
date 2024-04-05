## Código para configurar el SPIFFS en un ESP32
```c
esp_err_t spi_master_init(void){
  spi_bus_config_t buscfg = {
    .mosi = MOSI_PIN,
    .miso = MISO_PIN,
    .sclk_io_num =CLK_PIN,
    .quadwp_io_num = -1,
    .quadhd_io_num = -1,
  };
  esp_err_t ret = spi_bus_initialize(SPI_MASTER_NUM, &config,SPI_DMA_CH_AUTO);
  if(ret != ESP_OK){
    return ret;
  }
  spi_device_interface_config_t devcfg = {
    .mode = 1, //modo definido por el device
    .spics_io_num = CS_PIN,
    .queue_size = 1,
    .clock_speed_hz = 1.6,
    .pre_cb = NULL,
    .address_bits = 8,
  };

  spi_bus_add_device(SPI_MASTER_NUM, &devcfg,&spi);
  
}
```


### Escritura 
```c
esp_err_t device_register_write(uint8_t reg_address, uint8_t data){
  spi_transaction_t t;
  memset(&t, 0, sizeof(t));
  t.addr = reg_address & 0x7F;
  t.length = 8;
  t.tx_buffer = &data;
  esp_err_t ret = spi_device_transmit(spi, &t);
  return ret;
}
```
### Lectura

```c
esp_err_t device_register_read(uint8_t reg_addr, uint8_t *data, size_t len){
  spi_transaction_t t;
  memset(&t, 0, sizeof(t));
  t.addr = reg_address & 0x80;
  t.length = len;
  t.rx_buffer = &data;
  esp_err_t ret = spi_device_transmit(spi, &t);
  return ret;

}
```

### Transacciones

- Abrir archivo de de la SD card
- Leer archivo
- Escribir archivo
- Cerrar archivos

### Actividad
TAREA ES CREAR VIM!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
