#include <WiFi.h>
#include "esp_wpa2.h"
#include <HardwareSerial.h>
#include "credentials.h"

#define UART_RX_PIN 16
#define UART_TX_PIN 17
#define UART_BAUDRATE 115200

#define TCP_PORT 8888
#define BUFFER_SIZE 512

HardwareSerial SerialBridge(2);
WiFiServer server(TCP_PORT);
WiFiClient client;

void startAccessPoint() {
    Serial.println("[WIFI] Starting fallback Access Point...");
    WiFi.disconnect(true);
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASS);
    IPAddress ip = WiFi.softAPIP();
    Serial.print("[WIFI] Access Point active. SSID: ");
    Serial.println(AP_SSID);
    Serial.print("[WIFI] Listening IP address: ");
    Serial.println(ip);
}

void connectToWiFi() {
    if (!USE_EDUROAM) {
        startAccessPoint();
        return;
    }

    Serial.println("[WIFI] Connecting to eduroam (WPA2-Enterprise)...");
    WiFi.disconnect(true);
    WiFi.mode(WIFI_STA);

    esp_wifi_sta_wpa2_ent_enable();
    esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)EAP_IDENTITY, strlen(EAP_IDENTITY));
    esp_wifi_sta_wpa2_ent_set_username((uint8_t *)EAP_USERNAME, strlen(EAP_USERNAME));
    esp_wifi_sta_wpa2_ent_set_password((uint8_t *)EAP_PASSWORD, strlen(EAP_PASSWORD));

    WiFi.begin("eduroam");

    uint32_t start_ms = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start_ms < 15000) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("[WIFI] Connected to eduroam successfully!");
        Serial.print("[WIFI] Assigned IP address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("[WIFI] Connection timed out.");
        startAccessPoint();
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n==================================================");
    Serial.println("   ESP32 Wireless UART Bridge");
    Serial.println("==================================================");

    pinMode(UART_RX_PIN, INPUT_PULLUP);

    SerialBridge.begin(UART_BAUDRATE, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);
    Serial.println("[UART] Hardware UART2 initialized @ 115200 8N1");

    connectToWiFi();

    server.begin();
    server.setNoDelay(true);
    Serial.printf("[TCP] Bridge server listening on port %d\n", TCP_PORT);
}

void loop() {
    if (!client || !client.connected()) {
        WiFiClient newClient = server.available();
        if (newClient) {
            client = newClient;
            client.setNoDelay(true);
            Serial.print("[TCP] Client connected from: ");
            Serial.println(client.remoteIP());
        }
    }

    if (client && client.connected()) {
        uint8_t buf[BUFFER_SIZE];

        while (client.available() > 0) {
            int bytesToRead = client.available();
            if (bytesToRead > BUFFER_SIZE) bytesToRead = BUFFER_SIZE;
            int bytesRead = client.read(buf, bytesToRead);
            if (bytesRead > 0) {
                SerialBridge.write(buf, bytesRead);
                Serial.printf("[BRIDGE] Forwarded %d bytes from Wi-Fi -> UART2\n", bytesRead);
            }
        }

        while (SerialBridge.available() > 0) {
            int bytesToRead = SerialBridge.available();
            if (bytesToRead > BUFFER_SIZE) bytesToRead = BUFFER_SIZE;
            int bytesRead = SerialBridge.readBytes(buf, bytesToRead);
            if (bytesRead > 0) {
                client.write(buf, bytesRead);
                Serial.printf("[BRIDGE] Forwarded %d bytes from UART2 -> Wi-Fi\n", bytesRead);
            }
        }
    }
}
