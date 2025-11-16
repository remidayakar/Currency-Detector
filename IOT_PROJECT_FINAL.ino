// TCS3200 Color Sensor + Indian Currency Detection + Speaker Feedback + ThingSpeak Logging
// For Arduino R4 WiFi
// Developed for Remi 💸

#include <WiFi.h>
#include "ThingSpeak.h"

// ---------- Pin Definitions ----------
int s0 = 4;
int s1 = 5;
int s2 = 6;
int s3 = 7;
int outPin = 8;
int speakerPin = 9;

// ---------- Wi-Fi & ThingSpeak ----------
const char* ssid = "Remi";
const char* password = "qwerty123";

unsigned long myChannelNumber = 3126112;  
const char* myWriteAPIKey = "977JEZP3WBMGP4ZX";

WiFiClient client;

// ---------- RGB Storage ----------
int blankR, blankG, blankB;
int r10R, r10G, r10B;
int r50R, r50G, r50B;
int r100R, r100G, r100B;
int r200R, r200G, r200B;
int r500R, r500G, r500B;

bool calibrated = false;

// ---------- Setup ----------
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("TCS3200 Currency Detector with ThingSpeak Ready on Arduino R4 WiFi!");

  pinMode(s0, OUTPUT);
  pinMode(s1, OUTPUT);
  pinMode(s2, OUTPUT);
  pinMode(s3, OUTPUT);
  pinMode(outPin, INPUT);
  pinMode(speakerPin, OUTPUT);

  digitalWrite(s0, HIGH);
  digitalWrite(s1, LOW);

  connectWiFi();
  ThingSpeak.begin(client);
  calibrateNotes();
}

// ---------- Loop ----------
void loop() {
  int r, g, b;
  readColor(r, g, b);

  Serial.print("R: "); Serial.print(r);
  Serial.print(" G: "); Serial.print(g);
  Serial.print(" B: "); Serial.println(b);

  String note = detectCurrency(r, g, b);
  Serial.print("  --> Detected: ");
  Serial.println(note);

  playToneForCurrency(note);
  sendToThingSpeak(note);

    delay(3000);
}

// ---------- Wi-Fi ----------
void connectWiFi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);

  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry < 20) {
    delay(500);
    Serial.print(".");
    retry++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ Wi-Fi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n❌ Failed to connect to Wi-Fi.");
  }
}

// ---------- Color Reading ----------
void readColor(int &red, int &green, int &blue) {
  digitalWrite(s2, LOW);
  digitalWrite(s3, HIGH);
  red = pulseIn(outPin, LOW);

  digitalWrite(s2, HIGH);
  digitalWrite(s3, HIGH);
  green = pulseIn(outPin, LOW);

  digitalWrite(s2, LOW);
  digitalWrite(s3, LOW);
  blue = pulseIn(outPin, LOW);
}

// ---------- Calibration ----------
void calibrateNotes() {
  Serial.println("🪙 Currency Calibration Mode");

  Serial.println("Show blank space...");
  delay(5000);
  readColor(blankR, blankG, blankB);
  Serial.println("Blank space calibrated!");

  Serial.println("Show ₹10 note...");
  delay(5000);
  readColor(r10R, r10G, r10B);
  Serial.println("₹10 calibrated!");

  Serial.println("Show ₹50 note...");
  delay(5000);
  readColor(r50R, r50G, r50B);
  Serial.println("₹50 calibrated!");

  Serial.println("Show ₹100 note...");
  delay(5000);
  readColor(r100R, r100G, r100B);
  Serial.println("₹100 calibrated!");

  Serial.println("Show ₹200 note...");
  delay(5000);
  readColor(r200R, r200G, r200B);
  Serial.println("₹200 calibrated!");

  Serial.println("Show ₹500 note...");
  delay(5000);
  readColor(r500R, r500G, r500B);
  Serial.println("₹500 calibrated!");

  calibrated = true;
  Serial.println("✅ Calibration Complete! Now detecting...");
}

// ---------- Detection ----------
String detectCurrency(int r, int g, int b) {
  if (!calibrated) return "Not Calibrated";

  long dBlank = sq(r - blankR) + sq(g - blankG) + sq(b - blankB);
  long d10 = sq(r - r10R) + sq(g - r10G) + sq(b - r10B);
  long d50 = sq(r - r50R) + sq(g - r50G) + sq(b - r50B);
  long d100 = sq(r - r100R) + sq(g - r100G) + sq(b - r100B);
  long d200 = sq(r - r200R) + sq(g - r200G) + sq(b - r200B);
  long d500 = sq(r - r500R) + sq(g - r500G) + sq(b - r500B);

  long minDist = dBlank;
  String note = "Blank Space";

  if (d10 < minDist) { minDist = d10; note = "₹10"; }
  if (d50 < minDist) { minDist = d50; note = "₹50"; }
  if (d100 < minDist) { minDist = d100; note = "₹100"; }
  if (d200 < minDist) { minDist = d200; note = "₹200"; }
  if (d500 < minDist) { minDist = d500; note = "₹500"; }

  return note;
}

// ---------- Sound Feedback ----------
void playToneForCurrency(String note) {
  int freq = 0;
  if (note == "₹10") freq = 400;
  else if (note == "₹50") freq = 600;
  else if (note == "₹100") freq = 700;
  else if (note == "₹200") freq = 500;
  else if (note == "₹500") freq = 900;
  else if (note == "Blank Space") freq = 0;

  if (freq > 0) {
    tone(speakerPin, freq, 300);
    delay(350);
  }
}

// ---------- ThingSpeak ----------
void sendToThingSpeak(String note) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi disconnected, reconnecting...");
    connectWiFi();
  }

  int value = 0;
  if (note == "₹10") value = 10;
  else if (note == "₹50") value = 50;
  else if (note == "₹100") value = 100;
  else if (note == "₹200") value = 200;
  else if (note == "₹500") value = 500;
  else if (note == "Blank Space") value = 0;

  ThingSpeak.setField(1, value);
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

  if (x == 200) {
    Serial.println("📤 ThingSpeak Update Successful!");
  } else {
    Serial.print("❌ ThingSpeak Update Failed, Code: ");
    Serial.println(x);
  }
}
