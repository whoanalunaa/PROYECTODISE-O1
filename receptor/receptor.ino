#include <BLEDevice.h>  
#include <BLEServer.h>  
#include <BLEUtils.h>  
#include <BLE2902.h>  

// Pines de conexión del motor  
const int IN1 = 25;  // Motor A - adelante  
const int IN2 = 26;  // Motor A - atrás  
const int IN3 = 27;  // Motor B - derecho  
const int IN4 = 14;  // Motor B - izquierdo  
const int ENA = 32;  // Pin de control de velocidad del Motor A  
const int ENB = 33;  // Pin de control de velocidad del Motor B  

BLEServer* pServer;  
BLECharacteristic* pCharacteristic;  
bool deviceConnected = false;  

// ID de servicio y característica BLE  
#define SERVICE_UUID "abcd1234-5678-90ab-cdef-abcd12345678"  
#define CHARACTERISTIC_UUID "abcd1234-5678-90ab-cdef-abcd12345679"  

// Clase de callback para conexión y desconexión del servidor  
class MyServerCallbacks : public BLEServerCallbacks {  
    void onConnect(BLEServer* pServer) override {  
        deviceConnected = true;  
        Serial.println("Cliente conectado.");  
    }  

    void onDisconnect(BLEServer* pServer) override {  
        deviceConnected = false;  
        Serial.println("Cliente desconectado.");  
    }  
};  

void handleData(String data) {  
    int commaIndex = data.indexOf(',');  
    if (commaIndex > 0) {  
        int movement = data.substring(0, commaIndex).toInt();  // Obtener movimiento  
        int direction = data.substring(commaIndex + 1).toInt(); // Obtener dirección  

        // Controlar velocidad (usar 128 para la mitad de la potencia)  
        int speed = 100;  

        // Movimiento: 0 = detener, 1 = adelante, -1 = atrás  
        if (movement == 1) {  
            // Mover hacia adelante: ambos motores hacia adelante  
            digitalWrite(IN1, HIGH); // Motor A adelante  
            digitalWrite(IN2, LOW);  
            digitalWrite(IN3, HIGH); // Motor B adelante  
            digitalWrite(IN4, LOW);  
            analogWrite(ENA, speed); // Ajustar velocidad del Motor A  
            analogWrite(ENB, speed); // Ajustar velocidad del Motor B  
        } else if (movement == -1) {  
            // Retroceder: ambos motores hacia atrás  
            digitalWrite(IN1, LOW);  
            digitalWrite(IN2, HIGH); // Motor A atrás  
            digitalWrite(IN3, LOW);  
            digitalWrite(IN4, HIGH); // Motor B atrás  
            analogWrite(ENA, speed); // Ajustar velocidad del Motor A  
            analogWrite(ENB, speed); // Ajustar velocidad del Motor B  
        } else {  
            // Detener ambos motores  
            digitalWrite(IN1, LOW);  
            digitalWrite(IN2, LOW);  
            digitalWrite(IN3, LOW);  
            digitalWrite(IN4, LOW);  
            analogWrite(ENA, 0);  // Detener Motor A  
            analogWrite(ENB, 0);  // Detener Motor B  
        }  

        // Giro: 0 = detener, 1 = girar a la derecha, -1 = girar a la izquierda  
        if (direction == 1) {  
            // Girar a la derecha  
            digitalWrite(IN1, HIGH); // Motor A adelante  
            digitalWrite(IN2, LOW);  
            digitalWrite(IN3, LOW);  // Motor B detenido  
            digitalWrite(IN4, LOW);  
            analogWrite(ENA, speed); // Ajustar velocidad del Motor A  
            analogWrite(ENB, 0);     // Detener Motor B  
        } else if (direction == -1) {  
            // Girar a la izquierda  
            digitalWrite(IN1, LOW);  // Motor A detenido  
            digitalWrite(IN2, LOW);  
            digitalWrite(IN3, HIGH); // Motor B adelante  
            digitalWrite(IN4, LOW);  
            analogWrite(ENA, 0);     // Detener Motor A  
            analogWrite(ENB, speed); // Ajustar velocidad del Motor B  
        }  

        // Mostrar datos procesados en el monitor serie  
        Serial.print("Movimiento: ");  
        Serial.print(movement);  
        Serial.print(", Dirección: ");  
        Serial.println(direction);  
    }  
}  

void setup() {  
    Serial.begin(115200);  

    // Configurar pines de motor  
    pinMode(IN1, OUTPUT);  
    pinMode(IN2, OUTPUT);  
    pinMode(IN3, OUTPUT);  
    pinMode(IN4, OUTPUT);  
    
    // Configurar pines de velocidad como salida  
    pinMode(ENA, OUTPUT);  
    pinMode(ENB, OUTPUT);  

    // Inicializar BLE  
    BLEDevice::init("MotorController"); // Nombre del dispositivo BLE  
    pServer = BLEDevice::createServer();  
    pServer->setCallbacks(new MyServerCallbacks()); // Configurar callbacks  

    BLEService* pService = pServer->createService(SERVICE_UUID);  
    pCharacteristic = pService->createCharacteristic(  
        CHARACTERISTIC_UUID,  
        BLECharacteristic::PROPERTY_READ |  
        BLECharacteristic::PROPERTY_WRITE  
    );  
    pCharacteristic->addDescriptor(new BLE2902());  
    pService->start();  

    BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();  
    pAdvertising->addServiceUUID(SERVICE_UUID);  
    pAdvertising->start();  

    Serial.println("Esperando conexión...");  
}  

void loop() {  
    if (deviceConnected) {  
        String receivedData = pCharacteristic->getValue();  // Nota: getValue devuelve String directamente  

        if (receivedData.length() > 0) {  
            Serial.println("Datos recibidos: " + receivedData);  
            handleData(receivedData);  

            // Limpiar el valor después de manejar los datos  
            pCharacteristic->setValue("");  
        }  
    }  

    delay(100); // Esperar un tiempo breve antes de volver a comprobar  
}