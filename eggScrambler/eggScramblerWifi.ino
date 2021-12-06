/*
 * Much of this code was repurposed from one of the Arduino Wifi examples. 
 * Importantly, the variable runEggScrambler can have a value of 1 or -1 (mutually exclusive).
 * If runEggScrambler = 1, it means that we have received a start request from the web server.
 * As a result, is_button_pressed := 1. If runEggScramber = -1, it means that we have received 
 * a stop request from the server. As aa result, stop_button_pressed := 1. Mapping the 
 * runEggScrambler value to the is_button_pressed and stop_button_pressed variables is completed
 * in the eggScrambler_utils file. 
 */

#include <SPI.h>
#include <WiFi101.h>

#include "arduino_secrets.h"
char ssid[] = SECRET_SSID;        

int status = WL_IDLE_STATUS;
WiFiServer server(80);


void setupWifi() {
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    while (true);       // don't continue
  }

  // attempt to connect to WiFi network:
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    // print the network name (SSID);
    Serial.println(ssid);

    status = WiFi.begin(ssid);
    // wait 10 seconds for connection:
    delay(10000);
  }
  server.begin();                           // start the web server on port 80
  printWiFiStatus();                        // you're connected now, so print out the status
}


int checkForClients() {
  int runEggScrambler = 0;
  WiFiClient client = server.available();   // listen for incoming clients
  
  if (client) {                             // if you get a client,
    Serial.println("new client");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    boolean requestValid = true;
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character
          // Check to see if the client request was "GET /start" or "GET /stop":
          if (currentLine.indexOf("GET /start?cooking-time=") != -1) {
            // start the scrambler, check cooking time
            int startInd = currentLine.indexOf("GET /start?cooking-time=") + 24;
            int endInd = currentLine.length() - 9; // cut off " HTTP/1.1 " at the end
            String cookingTimeString = currentLine.substring(startInd, endInd);
            int cookingTime = cookingTimeString.toInt();
            Serial.print("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\nThe cooking time is ");
            Serial.println(cookingTime);
            if (cookingTime == 0) {
              requestValid = false;
            } else {
              cooking_time = cookingTime * 1000;
              runEggScrambler = 1;
            }
          } else if (currentLine.indexOf("GET /stop") != -1) {
            // stop the scrambler
            runEggScrambler = -1;
          }

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            if (requestValid) {
              client.println("HTTP/1.1 200 OK");
            } else {
              client.println("HTTP/1.1 400");
            }
            client.println("Content-type:text/html");
            client.println("Access-Control-Allow-Origin: null");
            client.println();

            // the content of the HTTP response follows the header:
            client.print("");

            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          }
          else {      // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        }
        else if (c != '\r') {    // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
//    return runEggScrambler;
  }
  return runEggScrambler;
}

void printWiFiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
  // print where to go in a browser:
  Serial.print("This is the Arduino's IP: ");
  Serial.println(ip);
}
