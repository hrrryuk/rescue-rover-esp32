#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>

const char* ssid = "*";
const char* password = "*";

WebServer server(80);
WebSocketsServer webSocket(81);

#define PWMA 13
#define AIN1 27
#define AIN2 14

#define PWMB 32
#define BIN1 25
#define BIN2 33

#define STBY 26

#define CH_A 0
#define CH_B 1

void startMotor() {
  digitalWrite(STBY, HIGH);
}

void stopMotor() {
  digitalWrite(STBY, LOW);
}

void setMotor(int IN1, int IN2, int CH, int speed) {
  speed = constrain(speed, -255, 255);
  if (speed > 0) {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
  } 
  else if (speed < 0) {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
  } 
  else {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
  }
  ledcWrite(CH, abs(speed));
}

void motorLeft(int speed) {
  setMotor(AIN1, AIN2, CH_A, speed);
}

void motorRight(int speed) {
  setMotor(BIN1, BIN2, CH_B, speed);
}

void brake() {
  motorLeft(0);
  motorRight(0);
}

int currentLeft = 0;
int currentRight = 0;
unsigned long lastSignal = 0;

int ramp(int target, int current, int step = 15) {
  if (target > current) return min(current + step, target);
  if (target < current) return max(current - step, target);
  return current;
}

void joystickDrive(int V, int H) {
  lastSignal = millis();

  if (abs(V) < 10) V = 0;
  if (abs(H) < 10) H = 0;

  int throttle = map(V, -100, 100, -255, 255);
  int steering = map(H, -100, 100, -255, 255);

  int left  = throttle - steering;
  int right = throttle + steering;

  left  = constrain(left, -255, 255);
  right = constrain(right, -255, 255);

  currentLeft  = ramp(left, currentLeft);
  currentRight = ramp(right, currentRight);

  motorLeft(currentLeft);
  motorRight(currentRight);
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  if (type == WStype_TEXT) {
    StaticJsonDocument<200> doc;
    DeserializationError err = deserializeJson(doc, payload, length);
    if (err) return;
    const char* t = doc["t"];
    if (strcmp(t, "cmd") == 0) {
      const char* c = doc["c"];
      if (strcmp(c, "START") == 0) startMotor();
      if (strcmp(c, "STOP") == 0) stopMotor();
      if (strcmp(c, "BRAKE") == 0) brake();
      return;
    }
    if (strcmp(t, "joy") == 0) {
      int V = doc["v"];
      int H = doc["h"];
      joystickDrive(V, H);
    }
  }
  if (type == WStype_DISCONNECTED) {
    brake();
    currentLeft = 0;
    currentRight = 0;
  }
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>ResQ Rover</title>

<style>
* {
  box-sizing: border-box;
}
body {
  margin: 0;
  font-family: Consolas;
  background: #121212;
  color: white;
  text-align: center;
}
.cam {
  width: 100%;
  height: 30vh;
  background: #000;
  overflow: hidden;
  margin: 20px 0;
}
.cam img {
  width: 100%;
  height: 100%;
  object-fit: cover;
  display: block;
}
.mode {
  display: flex;
  justify-content: space-between;
  margin: 20px;
  gap: 20px;
}
.controls {
  display: flex;
  justify-content: space-between;
  margin: 20px;
}
.pad {
  width: 90px;
  height: 180px;
  background: #161616;
  border: 2px solid #222;
  border-radius: 10px;
  position: relative;
  touch-action: none;
}
.center-line {
  position: absolute;
  width: 100%;
  height: 2px;
  background: #222;
  top: 50%;
  left: 0;
  transform: translateY(-50%);
}
.knob {
  width: 100%;
  height: 25px;
  background: #00ff88;
  position: absolute;
  left: 0;
  top: 50%;
  transform: translateY(-50%);
  border-radius: 10px;
}
.buttons {
  display: flex;
  flex-direction: column;
  justify-content: space-between;
}
button {
  width: 100%;
  height: 50px;
  border: none;
  border-radius: 5px;
  font-weight: bold;
  padding: 0 15px;
}
.start { background: #00ff88; }
.stop { background: #ff4444; }
.brake { background: #ffaa00; }
.auto { background: #00ff88; }
.manual { background: #ff4444; }
</style>
</head>
<body>

<div class="cam">
  <img id="camera" src="http://192.168.0.105:81/stream">
</div>
<div class="mode">
  <button class="auto" onclick="sendCmd('AUTO')">AUTOMATIC</button>
  <button class="manual" onclick="sendCmd('MANUAL')">MANUAL</button>
</div>
<div class="controls">
  <div class="pad" id="vPad">
    <div class="center-line"></div>
    <div class="knob" id="vKnob"></div>
  </div>
  <div class="buttons">
    <button class="start" onclick="sendCmd('START')">START</button>
    <button class="stop" onclick="sendCmd('STOP')">STOP</button>
    <button class="brake" onclick="sendCmd('BRAKE')">BRAKE</button>
  </div>
  <div class="pad" id="hPad">
    <div class="center-line"></div>
    <div class="knob" id="hKnob"></div>
  </div>
</div>

<script>
const ws = new WebSocket(`ws://${location.hostname}:81`);

let lastV = 0;
let lastH = 0;

ws.onopen = () => {
  console.log("WebSocket connected");
};

ws.onclose = () => {
  console.log("WebSocket disconnected");
};

function sendCmd(cmd) {
  if (ws.readyState === 1) {
    ws.send(JSON.stringify({
      t: "cmd",
      c: cmd
    }));
  }
}

function sendJoy(v, h) {
  if (ws.readyState === 1) {
    ws.send(JSON.stringify({
      t: "joy",
      v: v,
      h: h
    }));
  }
}

const vPad = document.getElementById("vPad");
const vKnob = document.getElementById("vKnob");

let vActive = false;

vPad.addEventListener("touchstart", () => vActive = true);

vPad.addEventListener("touchend", () => {
  vActive = false;
  vKnob.style.top = "50%";
  lastV = 0;
  sendJoy(0, lastH);
});

vPad.addEventListener("touchmove", (e) => {
  if (!vActive) return;

  const rect = vPad.getBoundingClientRect();
  const y = e.touches[0].clientY - rect.top;

  let percent = (rect.height / 2 - y) / (rect.height / 2);
  percent = Math.max(-1, Math.min(1, percent));

  const value = Math.round(-percent * 100);

  vKnob.style.top = `${50 - percent * 50}%`;

  lastV = value;
  sendJoy(value, lastH);
});

const hPad = document.getElementById("hPad");
const hKnob = document.getElementById("hKnob");

let hActive = false;

hPad.addEventListener("touchstart", () => hActive = true);

hPad.addEventListener("touchend", () => {
  hActive = false;
  hKnob.style.top = "50%";
  lastH = 0;
  sendJoy(lastV, 0);
});

hPad.addEventListener("touchmove", (e) => {
  if (!hActive) return;

  const rect = hPad.getBoundingClientRect();
  const y = e.touches[0].clientY - rect.top;

  let percent = (rect.height / 2 - y) / (rect.height / 2);
  percent = Math.max(-1, Math.min(1, percent));

  const value = Math.round(percent * 100);

  hKnob.style.top = `${50 - percent * 50}%`;

  lastH = value;
  sendJoy(lastV, value);
});
</script>

</body>
</html>
)rawliteral";

void handleRoot() {
  server.send_P(200, "text/html", index_html);
}

void setup() {
  Serial.begin(115200);

  pinMode(STBY, OUTPUT);
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);

  digitalWrite(STBY, LOW);

  ledcSetup(CH_A, 1000, 8);
  ledcSetup(CH_B, 1000, 8);

  ledcAttachPin(PWMA, CH_A);
  ledcAttachPin(PWMB, CH_B);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.begin();

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  startMotor();
}

void loop() {
  server.handleClient();
  webSocket.loop();

  if (millis() - lastSignal > 1200) {
    brake();
    currentLeft = 0;
    currentRight = 0;
  }
}