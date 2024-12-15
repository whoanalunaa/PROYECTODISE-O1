#include <BLEDevice.h>  
#include <BLEClient.h>  
#include <BLEUtils.h>  
#include <BLEScan.h>  

#define SERVICE_UUID "abcd1234-5678-90ab-cdef-abcd12345678" // UUID del servicio  
#define CHARACTERISTIC_UUID "abcd1234-5678-90ab-cdef-abcd12345679" // UUID de la característica  
#define POT_FORWARD_BACK_PIN 32 // Pin para el potenciómetro de adelante/atrás  
#define POT_LEFT_RIGHT_PIN 35 // Pin para el potenciómetro de derecha/izquierda  

BLEClient* pClient = nullptr;  
BLERemoteCharacteristic* pRemoteCharacteristic = nullptr;  
bool connected = false;  
bool doConnect = false;  
BLEAdvertisedDevice* myDevice;  

// Función para conectar al servidor BLE  
void connectToServer() {  
    Serial.println("Intentando conectar al servidor BLE...");  
    pClient = BLEDevice::createClient();  
    Serial.println("Cliente BLE creado.");  

    if (pClient->connect(myDevice)) {  
        Serial.println("Conectado al servidor BLE.");  
        connected = true;  

        // Obtener la característica remota  
        BLERemoteService* pRemoteService = pClient->getService(SERVICE_UUID);  
        if (pRemoteService == nullptr) {  
            Serial.println("Error: No se encontró el servicio remoto.");  
            pClient->disconnect();  
            return;  
        }  
        pRemoteCharacteristic = pRemoteService->getCharacteristic(CHARACTERISTIC_UUID);  
        if (pRemoteCharacteristic == nullptr) {  
            Serial.println("Error: No se encontró la característica remota.");  
            pClient->disconnect();  
            return;  
        }  

        Serial.println("Característica remota encontrada.");  
    } else {  
        Serial.println("Error: No se pudo conectar al servidor BLE.");  
    }  
}  

// Callbacks para dispositivos BLE encontrados  
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {  
    void onResult(BLEAdvertisedDevice advertisedDevice) override {  
        Serial.print("Dispositivo encontrado: ");  
        Serial.println(advertisedDevice.toString().c_str());  

        if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(BLEUUID(SERVICE_UUID))) {  
            Serial.println("Dispositivo con servicio UUID encontrado.");  
            myDevice = new BLEAdvertisedDevice(advertisedDevice);  
            doConnect = true; // Indicar que debemos conectarnos  
        }  
    }  
};  

void setup() {  
    Serial.begin(115200);  
    Serial.println("Iniciando escaneo BLE...");  

    // Inicializar BLE  
    BLEDevice::init("MotorController-Emisor");  
    BLEScan* pBLEScan = BLEDevice::getScan();  
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());  
    pBLEScan->setActiveScan(true);  
    pBLEScan->start(30, false);  

    // Configurar los pines de los potenciómetros  
    pinMode(POT_FORWARD_BACK_PIN, INPUT);  
    pinMode(POT_LEFT_RIGHT_PIN, INPUT);  
}  

void loop() {  
    // Conectar al servidor si se encontró un dispositivo válido  
    if (doConnect && !connected) {  
        connectToServer();  
        doConnect = false;  
    }  

    // Enviar datos si estamos conectados  
    if (connected) {  
        if (pRemoteCharacteristic != nullptr) {  
            // Leer los valores de los potenciómetros  
            int potForwardBackValue = analogRead(POT_FORWARD_BACK_PIN); // Leer el potenciómetro adelante/atrás  
            int potLeftRightValue = analogRead(POT_LEFT_RIGHT_PIN); // Leer el potenciómetro derecha/izquierda  

            // Mostrar valores leídos para depuración  
            Serial.print("Pot Forward/Back Raw: ");  
            Serial.println(potForwardBackValue);  
            Serial.print("Pot Left/Right Raw: ");  
            Serial.println(potLeftRightValue);  

            // Verificación de valores leídos  
            if (potForwardBackValue <= 0 || potForwardBackValue >= 3300) {  
                Serial.println("Advertencia: Lectura inesperada en Pot Forward/Back.");  
            }  
            if (potLeftRightValue <= 0 || potLeftRightValue >= 3300) {  
                Serial.println("Advertencia: Lectura inesperada en Pot Left/Right.");  
            }  

            // Mapear los valores de los potenciómetros  
            int movement = map(potForwardBackValue, 0, 3300, -1, 1); // Mueve de -1 (atrás) a 1 (adelante)  
            int direction = map(potLeftRightValue, 0, 3300, -1, 1); // Mueve de -1 (izquierda) a 1 (derecha)  

            // Construir el comando a enviar  
            String command = String(movement) + "," + String(direction);  

            // Enviar el valor al esclavo  
            pRemoteCharacteristic->writeValue(command.c_str());  
            Serial.println("Comando enviado: " + command);  
        } else {  
            Serial.println("Error: Característica remota no disponible.");  
        }  
    }  

    delay(100); // Esperar un tiempo antes de la siguiente iteración  
}