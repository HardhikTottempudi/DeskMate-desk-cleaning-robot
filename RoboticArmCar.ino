#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Servo.h>  // Built-in library for ESP8266

// Wi-Fi AP credentials
#define APSSID "YOUR_AP_NAME"
#define APPSK "YOUR_AP_PASSWORD"

ESP8266WebServer server(80);

// Servo pins (GPIOs)
const int servoPins[4] = {5, 4, 14, 12}; // D1, D2, D5, D6
Servo servos[4];
int servoAngles[4] = {180, 90, 90, 90};

// DC Motor pins (IN1/IN2 for Motor A, IN3/IN4 for Motor B)
#define IN1 0   // D3
#define IN2 2   // D4
#define IN3 13  // D7
#define IN4 15  // D8

void handleRoot() {
  String html = R"rawliteral(
  <!DOCTYPE html>
  <html>
  <head>
    <title>ESP8266 Robot Arm</title>
    <style>
      body { font-family: Arial; text-align: center; background: #f2f2f2; margin: 0; padding: 20px; }
      h1 { color: #333; }
      .slider-container { margin: 15px auto; padding: 10px; background: white; border-radius: 8px; box-shadow: 0 0 10px rgba(0,0,0,0.1); width: 300px; }
      input[type=range] { width: 100%; }
      #joystick-container {
        width: 200px;
        height: 200px;
        background: white;
        border-radius: 50%;
        position: relative;
        margin: 20px auto;
        border: 2px solid #ccc;
        touch-action: none;
      }
      #joystick {
        width: 60px;
        height: 60px;
        background: #4CAF50;
        border-radius: 50%;
        position: absolute;
        top: 70px;
        left: 70px;
        cursor: pointer;
        touch-action: none;
      }
    </style>
    <script>
      function updateServo(id, angle) {
        fetch('/setServo' + id + '?angle=' + angle);
        document.getElementById('val' + id).innerText = angle;
      }

      let isDragging = false;
      let currentX;
      let currentY;
      let initialX;
      let initialY;
      let xOffset = 0;
      let yOffset = 0;
      const joystickBoundary = 60;

      function dragStart(e) {
        if (e.type === "touchstart") {
          initialX = e.touches[0].clientX - xOffset;
          initialY = e.touches[0].clientY - yOffset;
        } else {
          initialX = e.clientX - xOffset;
          initialY = e.clientY - yOffset;
        }
        if (e.target === document.getElementById("joystick")) {
          isDragging = true;
        }
      }

      function dragEnd(e) {
        initialX = currentX;
        initialY = currentY;
        isDragging = false;
        // Return to center
        setTranslate(0, 0, document.getElementById("joystick"));
        xOffset = 0;
        yOffset = 0;
        fetch('/stop');
      }

      function drag(e) {
        if (isDragging) {
          e.preventDefault();
          if (e.type === "touchmove") {
            currentX = e.touches[0].clientX - initialX;
            currentY = e.touches[0].clientY - initialY;
          } else {
            currentX = e.clientX - initialX;
            currentY = e.clientY - initialY;
          }

          xOffset = currentX;
          yOffset = currentY;

          // Limit movement to circle
          let distance = Math.sqrt(currentX * currentX + currentY * currentY);
          if (distance > joystickBoundary) {
            currentX *= joystickBoundary / distance;
            currentY *= joystickBoundary / distance;
          }

          setTranslate(currentX, currentY, document.getElementById("joystick"));
          handleJoystickMovement(currentX, currentY);
        }
      }

      function setTranslate(xPos, yPos, el) {
        el.style.transform = "translate(" + xPos + "px, " + yPos + "px)";
      }

      function handleJoystickMovement(x, y) {
        const threshold = 20;
        if (Math.abs(x) < threshold && Math.abs(y) < threshold) {
          fetch('/stop');
          return;
        }

        if (Math.abs(x) > Math.abs(y)) {
          // Horizontal movement
          if (x > 0) {
            fetch('/right');
          } else {
            fetch('/left');
          }
        } else {
          // Vertical movement
          if (y < 0) {
            fetch('/forward');
          } else {
            fetch('/backward');
          }
        }
      }

      window.onload = function() {
        const joystick = document.getElementById("joystick");
        const container = document.getElementById("joystick-container");

        container.addEventListener("touchstart", dragStart, false);
        container.addEventListener("touchend", dragEnd, false);
        container.addEventListener("touchmove", drag, false);

        container.addEventListener("mousedown", dragStart, false);
        container.addEventListener("mouseup", dragEnd, false);
        container.addEventListener("mousemove", drag, false);
        container.addEventListener("mouseleave", dragEnd, false);
      }
    </script>
  </head>
  <body>
    <h1>ESP8266 Robotic Arm</h1>
  )rawliteral";

  for (int i = 0; i < 4; i++) {
    html += "<div class='slider-container'>";
    html += "<label>Servo " + String(i + 1) + " (0–180°)</label><br>";
    html += "<input type='range' min='0' max='180' value='" + String(servoAngles[i]) + "' oninput='updateServo(" + String(i) + ", this.value)'>";
    html += " <span id='val" + String(i) + "'>" + String(servoAngles[i]) + "</span>°";
    html += "</div>";
  }

  html += R"rawliteral(
    <h2>Base Motor Control</h2>
    <div id="joystick-container">
      <div id="joystick"></div>
    </div>
  </body></html>
  )rawliteral";

  server.send(200, "text/html", html);
}

void handleSetServo(int index) {
  if (server.hasArg("angle")) {
    int angle = constrain(server.arg("angle").toInt(), 0, 180);
    servos[index].write(angle);
    servoAngles[index] = angle;
    Serial.printf("Servo %d set to %d°\n", index + 1, angle);
  }
  server.send(200, "text/plain", "OK");
}

void handleBackward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  Serial.println("Base moving forward");
  server.send(200, "text/plain", "OK");
}

void handleForward() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  Serial.println("Base moving backward");
  server.send(200, "text/plain", "OK");
}

void handleRight() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  Serial.println("Base moving Rigth");
  server.send(200, "text/plain", "OK");
}
void handleLeft() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  Serial.println("Base moving Left");
  server.send(200, "text/plain", "OK");
}

void handleStop() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  Serial.println("Base stopped");
  server.send(200, "text/plain", "OK");
}

void setup() {
  Serial.begin(115200);
  delay(500);

  for (int i = 0; i < 4; i++) {
    servos[i].attach(servoPins[i]);
    servos[i].write(servoAngles[i]);
  }

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  handleStop();

  WiFi.softAP(APSSID, APPSK);
  Serial.print("Access Point started: ");
  Serial.println(APSSID);
  Serial.print("Visit http://");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/setServo0", []() { handleSetServo(0); });
  server.on("/setServo1", []() { handleSetServo(1); });
  server.on("/setServo2", []() { handleSetServo(2); });
  server.on("/setServo3", []() { handleSetServo(3); });
  server.on("/forward", handleForward);
  server.on("/backward", handleBackward);
  server.on("/right", handleRight);
  server.on("/left", handleLeft);
  server.on("/stop", handleStop);

  server.begin();
  Serial.println("Web server running.");
}

void loop() {
  server.handleClient();
}

