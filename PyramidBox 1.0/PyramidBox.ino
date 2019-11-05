/*
  _____  __   __  ______ _______ _______ _____ ______  ______   _____  _     _
 |_____]   \_/   |_____/ |_____| |  |  |   |   |     \ |_____] |     |  \___/ 
 |          |    |    \_ |     | |  |  | __|__ |_____/ |_____] |_____| _/   \_   1.0

   ===========================================
      Copyright (c) 2019 Soumak Dalal
          github.com/soumakdalal
   ===========================================
*/

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "./DNSServer.h"
#include "FS.h"

/* Set These To Your Desired Credentials. */
#ifndef APSSID
/* Wireless Network Name (SSID) */
#define APSSID "Pyramid Box"
/* Pre-Shared Key (PSK) */
#define APPSK  "openthebox"
#endif

const char        *ssid = APSSID;
const char        *password = APPSK;
const byte        DNS_PORT = 53;
const String      messagesFile = "/messages.txt";
const String      chatFile = "/pyramidbox.html";
String            chatHtml;

IPAddress         apIP(10, 10, 10, 1);
DNSServer         dnsServer;
ESP8266WebServer  webServer(80);

void setup() {
  Serial.begin(115200);
  SPIFFS.begin();
  WiFi.mode(WIFI_AP);              
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  /* You Can Remove The Password Parameter If You Want The AP To Be Open. */
  WiFi.softAP(ssid, password);
  dnsServer.start(DNS_PORT, "*", apIP);
  chatHtml = fileRead(chatFile);
  webServer.begin();

  setupAppHandlers();
  handleSendMessage();
  showChatPage();

}

void handleSendMessage() {
  if (webServer.hasArg("message")) {
    String message = webServer.arg("message");
    fileWrite(messagesFile, message + "\n" , "a+");
    webServer.sendHeader("Access-Control-Allow-Methods", "POST,GET,OPTIONS");
    webServer.sendHeader("Access-Control-Allow-Origin", "*");
    webServer.send(200, "text/plain", "Message Sent");
  }
}

void handleClearMessages() {
  SPIFFS.remove(messagesFile);
  webServer.sendHeader("Access-Control-Allow-Methods", "POST,GET,OPTIONS");
  webServer.sendHeader("Access-Control-Allow-Origin", "*");
  webServer.send(200, "text/plain", "File Deleted");
}

void showChatPage() {
  webServer.send(200, "text/html", chatHtml);
}

void showMessages() {
  String messages = fileRead(messagesFile);
  webServer.sendHeader("Access-Control-Allow-Methods", "POST,GET,OPTIONS");
  webServer.sendHeader("Access-Control-Allow-Origin", "*");
  webServer.send(200, "text/plain", messages);
}

void setupAppHandlers() {
  webServer.onNotFound([]() {
    showChatPage();
  });
  webServer.on("10.10.10.1", showChatPage);
  webServer.on("/sendMessage", handleSendMessage);
  webServer.on("/readMessages", showMessages);
  webServer.on("/clearMessages", handleClearMessages);
}

String fileRead(String name) {
  String contents;
  int i;
  File file = SPIFFS.open(name, "a+");
  for (i = 0; i < file.size(); i++)
  {
    contents += (char)file.read();
  }
  file.close();
  return contents;
}

void fileWrite(String name, String content, String mode) {
  File file = SPIFFS.open(name.c_str(), mode.c_str());
  file.write((uint8_t *)content.c_str(), content.length());
  file.close();
}

void loop() {
  dnsServer.processNextRequest();
  webServer.handleClient();
}