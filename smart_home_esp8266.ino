/*Shobharaj H L
  1SV21EC030 */
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiClient.h>
#include <DHT.h>

// ---------------- Device Pins ----------------
#define FAN_PIN      D1
#define LIGHT_PIN    D2
#define AC_PIN       D3
#define DEVICE4_PIN  D5
#define DEVICE5_PIN  D6

// ---------------- Sensor Pins ----------------
#define DHTPIN          D4
#define RAIN_SENSOR_PIN A0
#define GAS_SENSOR_PIN  D7
#define DHTTYPE         DHT11

// ---------------- Global Objects ----------------
DHT dht(DHTPIN, DHTTYPE);
ESP8266WebServer server(80);

// ---------------- WiFi Credentials ----------------
const char* ssid = "saad 4G";
const char* password = "11112222";

// ---------------- System Variables ----------------
float temperature = 0;
float humidity = 0;
int rainValue = 0;
int gasValue = 0;

bool deviceStates[5] = {false, false, false, false, false};
const int devicePins[5] = {FAN_PIN, LIGHT_PIN, AC_PIN, DEVICE4_PIN, DEVICE5_PIN};

// ---------------- Authentication ----------------
const String loginUsername = "saad";
const String loginPassword = "Saad#sidd";
bool isAuthenticated = false;

// ---------------- Thresholds ----------------
const float TEMP_HIGH = 30.0;
const float TEMP_LOW = 15.0;
const float HUMIDITY_HIGH = 70.0;
const float HUMIDITY_LOW = 30.0;
const int RAIN_THRESHOLD = 500;
const int GAS_THRESHOLD = 1;

// ---------------- Function Prototypes ----------------
void handleLoginPage();
void handleLogin();
void handleDashboard();
void handleToggle();
void handleSensor();
void handleLogout();
String getLoginHTML(bool showError = false);
String getDashboardHTML();

// ====================================================
//                      SETUP
// ====================================================
void setup() {
  Serial.begin(115200);

  // Initialize relays
  for (int i = 0; i < 5; i++) {
    pinMode(devicePins[i], OUTPUT);
    digitalWrite(devicePins[i], LOW);
  }

  // Initialize sensors
  pinMode(RAIN_SENSOR_PIN, INPUT);
  pinMode(GAS_SENSOR_PIN, INPUT);
  dht.begin();

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected! IP: " + WiFi.localIP().toString());

  // Server routes
  server.on("/", handleLoginPage);
  server.on("/login", handleLogin);
  server.on("/dashboard", handleDashboard);
  server.on("/toggle", handleToggle);
  server.on("/sensor", handleSensor);
  server.on("/logout", handleLogout);

  server.begin();
}

// ====================================================
//                       LOOP
// ====================================================
void loop() {
  server.handleClient();

  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 2000) {
    // Read sensors
    float newTemp = dht.readTemperature();
    float newHum = dht.readHumidity();

    if (!isnan(newTemp)) temperature = newTemp;
    if (!isnan(newHum)) humidity = newHum;

    rainValue = analogRead(RAIN_SENSOR_PIN);
    gasValue = digitalRead(GAS_SENSOR_PIN);

    lastUpdate = millis();
  }
}

// ====================================================
//                    LOGIN PAGE
// ====================================================
void handleLoginPage() {
  if (!isAuthenticated) {
    server.send(200, "text/html", getLoginHTML());
  } else {
    server.sendHeader("Location", "/dashboard");
    server.send(302);
  }
}

// ====================================================
//                      LOGIN
// ====================================================
void handleLogin() {
  if (server.hasArg("username") && server.hasArg("password")) {
    String user = server.arg("username");
    String pass = server.arg("password");

    if (user == loginUsername && pass == loginPassword) {
      isAuthenticated = true;
      server.sendHeader("Location", "/dashboard");
      server.send(302);
    } else {
      server.send(200, "text/html", getLoginHTML(true));
    }
  } else {
    server.send(400, "text/plain", "Bad Request");
  }
}

// ====================================================
//                    SENSOR HANDLER
// ====================================================
void handleSensor() {
  if (!isAuthenticated) {
    server.sendHeader("Location", "/");
    server.send(302);
    return;
  }

  String json = String("{") +
                "\"temp\":" + String(temperature, 1) + "," +
                "\"hum\":" + String(humidity, 1) + "," +
                "\"rain\":" + String(rainValue) + "," +
                "\"gas\":" + String(gasValue) + "}";

  server.send(200, "application/json", json);
}

// ====================================================
//                   DASHBOARD PAGE
// ====================================================
void handleDashboard() {
  if (!isAuthenticated) {
    server.sendHeader("Location", "/");
    server.send(302);
    return;
  }

  server.send(200, "text/html", getDashboardHTML());
}

// ====================================================
//                    LOGIN HTML
// ====================================================
String getLoginHTML(bool showError) {
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<title>VS TEC - Login</title>
<meta name="viewport" content="width=device-width, initial-scale=1">
<style>
  body {
    font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
    background: linear-gradient(135deg, #f5f7fa 0%, #c3cfe2 100%);
    margin: 0;
    padding: 0;
    display: flex;
    justify-content: center;
    align-items: center;
    min-height: 100vh;
  }
  .login-container {
    background: white;
    border-radius: 10px;
    box-shadow: 0 10px 25px rgba(0,0,0,0.1);
    width: 350px;
    padding: 40px;
    text-align: center;
  }
  .logo {
    color: #4361ee;
    font-size: 2rem;
    margin-bottom: 20px;
  }
  input {
    width: 100%;
    padding: 12px 15px;
    margin: 8px 0;
    border: 1px solid #ddd;
    border-radius: 5px;
    font-size: 1rem;
    box-sizing: border-box;
  }
  button {
    width: 100%;
    padding: 12px;
    background: #4361ee;
    color: white;
    border: none;
    border-radius: 5px;
    font-size: 1rem;
    cursor: pointer;
    margin-top: 15px;
    transition: background 0.3s;
  }
  button:hover {
    background: #3a56d4;
  }
  .error {
    color: #f72585;
    margin-top: 10px;
    font-size: 0.9rem;
  }
</style>
</head>
<body>
  <div class="login-container">
    <div class="logo">saad</div>
    <form action="/login" method="POST">
      <input type="text" name="username" placeholder="Username" required>
      <input type="password" name="password" placeholder="Password" required>
      <button type="submit">Login</button>
    </form>
)rawliteral";

  if (showError) {
    html += "<div class='error'>Invalid username or password!</div>";
  }

  html += R"rawliteral(
  </div>
</body>
</html>
)rawliteral";

  return html;
}
