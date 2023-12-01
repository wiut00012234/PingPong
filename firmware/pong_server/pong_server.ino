#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// Replace with your network credentials
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

ESP8266WebServer server(80);

void setup() {
  Serial.begin(9600); // Start the serial communication
  WiFi.begin(ssid, password); // Connect to WiFi

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  // Define server routes
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", getHTML());
  });

  server.on("/player1", HTTP_GET, []() {
    Serial.write('1'); // Send signal for player 1 to Arduino
    server.send(200, "text/plain", "Player 1 action");
  });

  server.on("/player2", HTTP_GET, []() {
    Serial.write('2'); // Send signal for player 2 to Arduino
    server.send(200, "text/plain", "Player 2 action");
  });

  server.begin(); // Start the server
}

void loop() {
  server.handleClient(); // Handle client requests
}

String getHTML() {
  return R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>LED Game Controller</title>
    <style>
        body { text-align: center; font-family: Arial, sans-serif; }
        .button {
            display: inline-block;
            margin: 20px;
            padding: 20px;
            font-size: 20px;
            cursor: pointer;
            background-color: #4CAF50;
            color: white;
            border: none;
            border-radius: 5px;
        }
        .button:hover {
            background-color: #45a049;
        }
    </style>
</head>
<body>
    <h1>LED Strip Game Controller</h1>
    <div id="playerSelection">
        <button class="button" onclick="selectPlayer(1)">Player 1</button>
        <button class="button" onclick="selectPlayer(2)">Player 2</button>
    </div>
    <div id="gameControl" style="display:none;">
        <button class="button" id="gameButton">Press to Play!</button>
    </div>

    <script>
        function selectPlayer(player) {
            document.getElementById("playerSelection").style.display = 'none';
            document.getElementById("gameControl").style.display = 'block';
            document.getElementById("gameButton").onclick = function() { sendCommand(player); };
        }

        function sendCommand(player) {
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function() {
                if (this.readyState == 4 && this.status == 200) {
                    console.log('Command sent to player ' + player);
                }
            };
            xhttp.open("GET", "/player" + player, true);
            xhttp.send();
        }
    </script>
</body>
</html>
  )rawliteral";
}
