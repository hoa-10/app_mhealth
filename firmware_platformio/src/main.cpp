#include <Arduino.h>
#include <ArduinoBLE.h>
#include <Arduino_LSM9DS1.h>

#define SERVICE_UUID    "19b10000-e8f2-537e-4f6c-d104768a1214"
#define IMU_CHAR_UUID   "19b10001-e8f2-537e-4f6c-d104768a1214"

BLEService imuService(SERVICE_UUID);
// 12 bytes = 6 x int16 (ax, ay, az * 1000; gx, gy, gz * 10) - Vừa khít 1 gói BLE duy nhất
BLECharacteristic imuChar(IMU_CHAR_UUID, BLERead | BLENotify, 12);

int16_t packet[6] = {0, 0, 0, 0, 0, 0};
float ax, ay, az, gx, gy, gz;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_BUILTIN, HIGH); delay(80);
    digitalWrite(LED_BUILTIN, LOW); delay(80);
  }

  IMU.begin();

  if (!BLE.begin()) {
    while (1) {
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
      delay(150);
    }
  }

  BLE.setDeviceName("Nano33");
  BLE.setLocalName("Nano33");
  BLE.setAdvertisedService(imuService);

  imuService.addCharacteristic(imuChar);
  BLE.addService(imuService);

  imuChar.writeValue((byte*)packet, 12);
  BLE.advertise();
}

void loop() {
  BLEDevice central = BLE.central();

  if (central) {
    digitalWrite(LED_BUILTIN, HIGH);

    while (central.connected()) {
      if (IMU.accelerationAvailable()) {
        IMU.readAcceleration(ax, ay, az);
        packet[0] = (int16_t)(ax * 1000.0f);
        packet[1] = (int16_t)(ay * 1000.0f);
        packet[2] = (int16_t)(az * 1000.0f);
      }

      if (IMU.gyroscopeAvailable()) {
        IMU.readGyroscope(gx, gy, gz);
        packet[3] = (int16_t)(gx * 10.0f);
        packet[4] = (int16_t)(gy * 10.0f);
        packet[5] = (int16_t)(gz * 10.0f);
      }

      // Gửi 1 gói 12 byte duy nhất chứa trọn vẹn cả 6 trục
      imuChar.writeValue((byte*)packet, 12);

      delay(20); // 50 Hz mượt mà, không nghẽn sóng
    }

    digitalWrite(LED_BUILTIN, LOW);
    BLE.advertise();
  }
}




