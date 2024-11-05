#include <WebServer.h>
#include <WiFi.h>
#include <esp32cam.h>

const char* WIFI_SSID = "P";
const char* WIFI_PASS = "26112000";

WebServer server(80);
static auto hiRes = esp32cam::Resolution::find(800, 600);

#define LED_PIN 4  // Định nghĩa chân LED

void serveJpg() {
  auto frame = esp32cam::capture();
  if (frame == nullptr) {
    Serial.println("CAPTURE FAIL");
    server.send(503, "", "");
    return;
  }
  server.setContentLength(frame->size());
  server.send(200, "image/jpeg");
  WiFiClient client = server.client();
  frame->writeTo(client);
}

void handleJpgHi() {
  if (!esp32cam::Camera.changeResolution(hiRes)) {
    Serial.println("SET-HI-RES FAIL");
  }
  serveJpg();
}

void handleLedOn() {
  digitalWrite(LED_PIN, HIGH);  // Bật LED
  server.send(200, "text/plain", "LED ON");
}

void handleLedOff() {
  digitalWrite(LED_PIN, LOW);  // Tắt LED
  server.send(200, "text/plain", "LED OFF");
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);  // Thiết lập LED là output
  digitalWrite(LED_PIN, LOW);  // Tắt LED ban đầu
  
  using namespace esp32cam;
  Config cfg;
  cfg.setPins(pins::AiThinker);
  cfg.setResolution(hiRes);
  cfg.setBufferCount(2);
  cfg.setJpeg(80);

  bool ok = Camera.begin(cfg);
  Serial.println(ok ? "CAMERA OK" : "CAMERA FAIL");

  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  
  Serial.print("http://");
  Serial.println(WiFi.localIP());

  server.on("/cam-hi.jpg", handleJpgHi);
  server.on("/led/on", handleLedOn);
  server.on("/led/off", handleLedOff);

  server.begin();
}

void loop() {
  server.handleClient();
}
