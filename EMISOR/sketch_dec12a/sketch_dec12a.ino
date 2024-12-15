#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

void setup() {
  Serial.begin(115200);

  // Inicializa Bluetooth en el receptor
  SerialBT.begin("ESP32-Receptor"); 
  Serial.println("Bluetooth receptor iniciado. Esperando comandos...");
}

void loop() {
  // Verificar si el Bluetooth está conectado
  if (SerialBT.hasClient()) {
    Serial.println("Dispositivo conectado. Esperando comandos...");

    // Verificar si hay datos disponibles
    if (SerialBT.available()) {
      // Leer y mostrar el comando recibido
      String comando = SerialBT.readStringUntil('\n');
      Serial.println("Comando recibido: " + comando);

      // Condicionar acciones con base en el comando
      if (comando == "adelante") {
        Serial.println("Moviendo adelante...");
      } else if (comando == "parar") {
        Serial.println("Deteniendo...");
      } else {
        Serial.println("Comando no reconocido.");
      }
    }
  } else {
    Serial.println("Esperando conexión...");
    delay(1000); // Reduce el uso de recursos
  }
}

