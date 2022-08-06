/*
  LED

  This example creates a Bluetooth® Low Energy peripheral with service that contains a
  characteristic to control an LED.

  The circuit:
  - Arduino MKR WiFi 1010, Arduino Uno WiFi Rev2 board, Arduino Nano 33 IoT,
    Arduino Nano 33 BLE, or Arduino Nano 33 BLE Sense board.

  You can use a generic Bluetooth® Low Energy central app, like LightBlue (iOS and Android) or
  nRF Connect (Android), to interact with the services and characteristics
  created in this sketch.

  This example code is in the public domain.
*/

#include <ArduinoBLE.h>
#include <utility/HCI.h>

BLEService ledService("19B10000-E8F2-537E-4F6C-D104768A1214"); // Bluetooth® Low Energy LED Service

// Bluetooth® Low Energy LED Switch Characteristic - custom 128-bit UUID, read and writable by central
BLEByteCharacteristic switchCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite);

const int ledPin = LED_BUILTIN; // pin to use for the LED

#define OCF_RESET              0x0003


/* Analysis while debugging the HCI communication.
 *
 * Command tx ->  0x1 0x3 0xC 0x0
 *   Reset
 * Command tx ->  0x1 0x1 0x10 0x0
 *   Read Local Version Information
 * Command tx ->  0x1 0x1 0xC 0x8 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0x3F
 *   Set Event Mask
 * Command tx ->  0x1 0x1 0x20 0x8 0xFF 0x3 0x0 0x0 0x0 0x0 0x0 0x0
 *   LE Set Event Mask
 * Command tx ->  0x1 0x2 0x20 0x0
 *   LE Read Buffer Size
 * Command tx ->  0x1 0x31 0x20 0x3 0x0 0x1 0x1
 *   LE Set Default PHY
 *      All PHYs preference: 0x00
 *      TX PHYs preference: 0x01
 *        LE 1M
 *      RX PHYs preference: 0x01
 *        LE 1M
 * Command tx ->  0x1 0x5 0x20 0x6 0xC9 0xC8 0x75 0x0 0xA 0xEF
 *   LE Set Random Address
 *      Address: C9:C8:75:00:0A:EF (Static)
 * Command tx ->  0x1 0xA 0x20 0x1 0x0
 *   LE Set Advertise Enable (disable) ..... E status: 0x42
 *     If the periodic advertising train corresponding to the Sync_Handle parameter
 *     does not exist, the Controller shall return the error code Unknown Advertising
 *     Identifier (0x42).
 * Command tx ->  0x1 0x6 0x20 0xF 0xA0 0x0 0xA0 0x0 0x0 0x1 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x7 0x0
 *   LE Set Advertising Parameters
 *   p. 2353
 *        Min advertising interval: 100.000 msec (0x00a0)
 *        Max advertising interval: 100.000 msec (0x00a0)
 *        Type: Connectable and Scannable undirected - ADV_IND (0x00)
 *        Own address type: Random Address (0x01)
 *        Direct address type: Public (0x00)
 *        Direct address: 00:00:00:00:00:00 (OUI 00-00-00)
 *        Channel map: 37, 38, 39 (0x07)
 *        Filter policy: Allow Scan Request from Any, Allow Connect Request from Any (0x00)
 * Command tx ->  0x1 0x8 0x20 0x20 0x15 0x2 0x1 0x6 0x11 0x6 0x14 0x12 0x8A 0x76 0x4 0xD1 0x6C 0x4F 0x7E 0x53 0xF2 0xE8 0x0 0x0 0xB1 0x19 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0
 *   LE Set Advertising Data
 *   Bluetooth HCI Command - LE Set Advertising Data
 *       Command Opcode: LE Set Advertising Data (0x2008)
 *           0010 00.. .... .... = Opcode Group Field: LE Controller Commands (0x08)
 *           .... ..00 0000 1000 = Opcode Command Field: LE Set Advertising Data (0x008)
 *       Parameter Total Length: 32
 *       Data Length: 21
 *       Advertising Data
 *           Flags
 *               Length: 2
 *               Type: Flags (0x01)
 *               000. .... = Reserved: 0x0
 *               ...0 .... = Simultaneous LE and BR/EDR to Same Device Capable (Host): false (0x0)
 *               .... 0... = Simultaneous LE and BR/EDR to Same Device Capable (Controller): false (0x0)
 *               .... .1.. = BR/EDR Not Supported: true (0x1)
 *               .... ..1. = LE General Discoverable Mode: true (0x1)
 *               .... ...0 = LE Limited Discoverable Mode: false (0x0)
 *           128-bit Service Class UUIDs (incomplete)
 *               Length: 17
 *               Type: 128-bit Service Class UUIDs (incomplete) (0x06)
 *               Custom UUID: 19b10000-e8f2-537e-4f6c-d104768a1214 (Unknown)
 *           Unused
 * Command tx ->  0x1 0x9 0x20 0x20 0x5 0x4 0x9 0x4C 0x45 0x44 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0
 *   LE Set Scan Response Data
 *       LED
 * Command tx ->  0x1 0xA 0x20 0x1 0x1
 *   LE Set Advertise Enable
 */

/* Regex to convert the debug print commands to Wireshark HEX format
 *
 * /0x([A-F0-9][A-F0-9])/$1/g
 * /0x([A-F0-9])/0$1/g
 * /Command tx -> /0000 /g
 *
 */


// moved to library
//static uint8_t randAddr[]= {0xC9, 0xC8, 0x75, 0x00, 0x0A, 0xEF};
//uint8_t phySetting[] = {0x00, 0x01, 0x01};

void setup() {
  Serial.begin(115200);
  while (!Serial);

// moved to library
//  delay(2000);

  HCI.debug(Serial2);

  // set LED pin to output mode
  pinMode(ledPin, OUTPUT);

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");

    while (1);
  }

  // moved to library
//  HCI.sendCommand(OGF_LE_CTL << 10 | 0x31, 3, phySetting);
//  HCI.leSetRandomAddress(randAddr);

  // set advertised local name and service UUID:
  BLE.setLocalName("LED");
  BLE.setAdvertisedService(ledService);

  // add the characteristic to the service
  ledService.addCharacteristic(switchCharacteristic);

  // add service
  BLE.addService(ledService);

  // set the initial value for the characeristic:
  switchCharacteristic.writeValue(0);

  // start advertising
  BLE.advertise();

  Serial.println("BLE LED Peripheral");
}

void loop() {
  // listen for Bluetooth® Low Energy peripherals to connect:
  BLEDevice central = BLE.central();

  // if a central is connected to peripheral:
  if (central) {
    Serial.print("Connected to central: ");
    // print the central's MAC address:
    Serial.println(central.address());

    // while the central is still connected to peripheral:
    while (central.connected()) {
      // if the remote device wrote to the characteristic,
      // use the value to control the LED:
      if (switchCharacteristic.written()) {
        if (switchCharacteristic.value()) {   // any value other than 0
          Serial.println("LED on");
          digitalWrite(ledPin, HIGH);         // will turn the LED on
        } else {                              // a 0 value
          Serial.println(F("LED off"));
          digitalWrite(ledPin, LOW);          // will turn the LED off
        }
      }
    }

    // when the central disconnects, print it out:
    Serial.print(F("Disconnected from central: "));
    Serial.println(central.address());
  }
}
