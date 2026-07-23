#include "esp_camera.h"
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>

// Hotspot
const char* ap_ssid = "Free_WiFi";      // name that shows up in WiFi list
const char* ap_password = "";            // "" = open network (no password)

const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 4, 1);          // the ESP32's own address
DNSServer dnsServer;
WebServer server(80);
bool cameraReady = false;
esp_err_t cameraError = ESP_OK;

// Camera pin configuration for AI-Thinker ESP32-CAM
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

bool startCamera() {
  camera_config_t config = {};
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_QVGA;
  config.jpeg_quality = 12;
  config.fb_count = 1;
  config.fb_location = psramFound() ? CAMERA_FB_IN_PSRAM : CAMERA_FB_IN_DRAM;
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;

  cameraError = esp_camera_init(&config);
  if (cameraError != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x\n", cameraError);
    return false;
  }
  Serial.println("Camera initialized");
  return true;
}

// The MJPEG video stream
void handleStream() {
  Serial.println("Stream request");
  if (!cameraReady) {
    server.send(503, "text/plain", "Camera not ready. init error: 0x" + String(cameraError, HEX));
    return;
  }

  WiFiClient client = server.client();
  String response = "HTTP/1.1 200 OK\r\n";
  response += "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n";
  server.sendContent(response);

  while (client.connected()) {
    camera_fb_t * fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      break;
    }
    String part = "--frame\r\nContent-Type: image/jpeg\r\nContent-Length: ";
    part += fb->len;
    part += "\r\n\r\n";
    server.sendContent(part);
    server.sendContent((const char *)fb->buf, fb->len);
    server.sendContent("\r\n");
    esp_camera_fb_return(fb);
    if (!client.connected()) break;
    delay(40);
  }
  Serial.println("Stream ended");
}

// One still frame. This is useful for testing before opening the live stream.
void handleJpg() {
  Serial.println("JPG request");
  if (!cameraReady) {
    server.send(503, "text/plain", "Camera not ready. init error: 0x" + String(cameraError, HEX));
    return;
  }

  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    server.send(500, "text/plain", "Camera capture failed");
    return;
  }

  WiFiClient client = server.client();
  server.sendHeader("Content-Type", "image/jpeg");
  server.sendHeader("Content-Length", String(fb->len));
  server.sendHeader("Connection", "close");
  server.send(200);
  client.write(fb->buf, fb->len);
  esp_camera_fb_return(fb);
}

// Clean full-screen viewer with client-side rotation controls.
void handleRoot() {
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>";
  html += "html,body{width:100%;height:100%;margin:0;overflow:hidden;background:#000;}";
  html += "body{font-family:Arial,sans-serif;}";
  html += "#viewer{position:fixed;inset:0;display:grid;place-items:center;background:#000;overflow:hidden;}";
  html += "#stream{display:block;max-width:none;max-height:none;transform-origin:center center;}";
  html += "#controls{position:fixed;left:0;right:0;bottom:16px;display:flex;gap:12px;justify-content:center;z-index:2;}";
  html += "button{font-size:18px;font-weight:bold;color:#fff;background:rgba(0,0,0,.55);border:1px solid rgba(255,255,255,.45);border-radius:10px;padding:10px 18px;}";
  html += "#status{position:fixed;inset:0;display:grid;place-items:center;color:#fff;background:#000;font:18px Arial,sans-serif;}";
  html += "</style>";
  html += "</head><body>";
  if (!cameraReady) {
    html += "<div id='status'>Camera not ready: 0x";
    html += String(cameraError, HEX);
    html += "</div>";
  } else {
    html += "<div id='viewer'><img id='stream' src='/stream' alt='camera stream'></div>";
    html += "<div id='controls'><button onclick='rotate(-90)'>CCW</button><button onclick='rotate(90)'>CW</button></div>";
    html += "<script>";
    html += "let deg=Number(localStorage.rot||0);";
    html += "const img=document.getElementById('stream');";
    html += "function fit(){const vw=innerWidth,vh=innerHeight,iw=img.naturalWidth||320,ih=img.naturalHeight||240;";
    html += "const side=Math.abs(deg)%180,rw=side?ih:iw,rh=side?iw:ih,scale=Math.min(vw/rw,vh/rh);";
    html += "img.style.width=iw+'px';img.style.height=ih+'px';img.style.transform='rotate('+deg+'deg) scale('+scale+')';}";
    html += "function rotate(delta){deg=(deg+delta+360)%360;localStorage.rot=deg;fit();}";
    html += "img.onload=fit;addEventListener('resize',fit);setInterval(fit,1000);fit();";
    html += "</script>";
  }
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  cameraReady = startCamera();

  // Start the hotspot
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(ap_ssid, ap_password);
  Serial.print("Hotspot started. Connect to: ");
  Serial.println(ap_ssid);
  Serial.print("Then open: http://");
  Serial.println(apIP);

  // DNS server: answer EVERY domain with our own IP (captive portal trick)
  dnsServer.start(DNS_PORT, "*", apIP);

  // Routes
  server.on("/", handleRoot);
  server.on("/jpg", handleJpg);
  server.on("/stream", handleStream);

  // These are the URLs phones/laptops secretly check to test internet.
  // Answering them makes the camera page auto-pop-up.
  server.onNotFound(handleRoot);

  server.begin();
  Serial.println("Server started");
}

void loop() {
  dnsServer.processNextRequest();   // handle DNS lookups
  server.handleClient();            // handle web requests
}
