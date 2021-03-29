/*
 * A simple program which allows for remote control of 3 RGB LEDs. Requires an ESP-12E (NodeMCU), 
 * circuit setup with 3 RGB LEDs(common anode, but the code can be modified to accommodate common cathode), 
 * an MQTT broker, Wifi access, and an MQTT client to publish to the respective topics. 
 * 
 * Sending any message with the characters 'r', 'g', and/or 'b' will toggle those colors for the respective 
 * LED. Publishing 'disco' to the topic will toggle a special mode where a random color is written to the
 * chosen LED. The random color in disco mode is represented by an integer in the range [0, 7] and has the 
 * following states:
 * 000 no color
 * 001 red
 * 010 green
 * 011 yellow
 * 100 blue
 * 101 magenta
 * 110 cyan
 * 111 white
 */
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Wifi credentials
const char* WIFI_SSID = "YOUR SSID";
const char* PASSWORD = "YOUR WIFI PASSWORD";

const int MAX_RETRIES = 20;

// MQTT information
const char* MQTT_IP = "YOUR BROKER IP";
const int MQTT_PORT = 1883;

// Topics
const char* RGB_1_TOPIC = "TOPIC FOR RGB 1";
const char* RGB_2_TOPIC = "TOPIC FOR RGB 2";
const char* RGB_3_TOPIC = "TOPIC FOR RGB 3";

const char* DISCO = "disco";
const char RED_LED = 'r';
const char GREEN_LED = 'g';
const char BLUE_LED = 'b';

// Pin constants
const int NUM_PINS = 3;

const int RED_PIN_INDEX = 0;
const int GREEN_PIN_INDEX = 1;
const int BLUE_PIN_INDEX = 2;

int RGB_1[NUM_PINS] = {D1, D2, D3};
int RGB_2[NUM_PINS] = {D5, D6, D7};
int RGB_3[NUM_PINS] = {D8, D9, D10};

volatile bool rgb1Disco = false;
volatile bool rgb2Disco = false;
volatile bool rgb3Disco = false;

const int R_MASK = 0b1;
const int G_MASK = 0b10;
const int B_MASK = 0b100;

const int MAX_RGB_INT = 8;
const int LOOP_DELAY = 250;

volatile int rgb1DiscoState = 0;
volatile int rgb2DiscoState = 0;
volatile int rgb3DiscoState = 0;

WiFiClient esp_client;
PubSubClient client(esp_client);

void setup() {
  Serial.begin(115200);
  delay(10);
  randomSeed(analogRead(A0));

  setupWifi();
  setupMqtt();

  setPinOutputs(RGB_1);
  setPinOutputs(RGB_2);
  setPinOutputs(RGB_3);
}

void setupWifi() {
  
  WiFi.begin(WIFI_SSID, PASSWORD);
  Serial.print("Connecting to network...");

  int attempt = 0;

  while (WiFi.status() != WL_CONNECTED && attempt <= MAX_RETRIES) {
    delay(1000);
    Serial.print("Attempt: ");
    Serial.println(++attempt);
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Could not connect to Wifi, shutting down...");
    delay(250);
    cli();
    ESP.deepSleep(0);
  } else {
    Serial.println("Connection established");  
    Serial.print("IP address:\t");
    Serial.println(WiFi.localIP());
    delay(250);
  }
}

void setupMqtt() {
  client.setServer(MQTT_IP, MQTT_PORT);
  client.setCallback(callback);

  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
 
    if (client.connect("rgb_led_client")) {
      Serial.println("connected");
      delay(250);
    } else {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }

  // Subscribe to the topics
  client.subscribe(RGB_1_TOPIC);
  client.subscribe(RGB_2_TOPIC);
  client.subscribe(RGB_3_TOPIC);
}

void callback(char* topic, byte* payload, unsigned int length) {
  // Copy the payload and convert to a c string
  char message[length + 1];
  memcpy(message, payload, length);
  message[length] = '\0';

  // Check which topic was invoked and take the appropriate action
  if(strcmp(topic, RGB_1_TOPIC) == 0) {
    if(strcmp(message, DISCO) == 0) {
      if(rgb1Disco == false) {
        rgb1Disco = true;
      } else {
        rgb1Disco = false;
      }
    } else {
      toggleOnMessage(RGB_1, message);
    }
  } else if(strcmp(topic, RGB_2_TOPIC) == 0) {
    if(strcmp(message, DISCO) == 0) {
      if(rgb2Disco == false) {
        rgb2Disco = true;
      } else {
        rgb2Disco = false;
      }
    } else {
      toggleOnMessage(RGB_2, message);
    }
  } else if(strcmp(topic, RGB_3_TOPIC) == 0) {
    if(strcmp(message, DISCO) == 0) {
      if(rgb3Disco == false) {
        rgb3Disco = true;
      } else {
        rgb3Disco = false;
      }
    } else {
      toggleOnMessage(RGB_3, message);
    }
  }
}

void toggleOnMessage(int pins[], char* message) {
  // if the message contains the character 'r', toggle the red LED
  if(strchr(message, RED_LED) != NULL) {
    togglePin(pins[RED_PIN_INDEX]);
  }

  // if the message contains the character 'g', toggle the green LED
  if(strchr(message, GREEN_LED) != NULL) {
    togglePin(pins[GREEN_PIN_INDEX]);
  }

  // if the message contiains the character 'b', toggle the blue LED
  if(strchr(message, BLUE_LED) != NULL) {
    togglePin(pins[BLUE_PIN_INDEX]);
  }
}

/*
 * Toggles the pin.
 */
void togglePin(int pin) {
  int val = digitalRead(pin);

  switch(val) {
    case HIGH:
      digitalWrite(pin, LOW);
      break;
    case LOW:
      digitalWrite(pin, HIGH);
    default:
      break;
  }
}

/*
 * Sets the pins in the provided array to type output and sets them high.
 */
void setPinOutputs(int arr[]) {
  for(int i = 0; i < NUM_PINS; ++i) {
    pinMode(arr[i], OUTPUT);
    digitalWrite(arr[i], HIGH);
  }
}

void loop() {
  client.loop();

  // Only execute the disco code if it is currently enabled
  if(rgb1Disco == true) {
    rgb1DiscoState = getRandomRgbSequence();
    setOutput(RGB_1, rgb1DiscoState);
  }

  if(rgb2Disco == true) {
    rgb2DiscoState = getRandomRgbSequence();
    setOutput(RGB_2, rgb2DiscoState);
  }

  if(rgb3Disco == true) {
    rgb3DiscoState = getRandomRgbSequence();
    setOutput(RGB_3, rgb3DiscoState); 
  }
  
  delay(LOOP_DELAY);
}

/*
 * Generates a random integer representation of the RGB writeout values.
 */
int getRandomRgbSequence() {
  return random(MAX_RGB_INT);
}

/*
 * Sets the output of pins in the provided array to the given value.
 */
void setOutput(int pins[], int val) {
  // Write the R, G, and B values depending on whether the respective bits are set in val
  if(val & R_MASK) {
    digitalWrite(pins[RED_PIN_INDEX], LOW);
  } else {
    digitalWrite(pins[RED_PIN_INDEX], HIGH);
  }

  if(val & G_MASK) {
    digitalWrite(pins[GREEN_PIN_INDEX], LOW);
  } else {
    digitalWrite(pins[GREEN_PIN_INDEX], HIGH);
  }

  if(val & B_MASK) {
    digitalWrite(pins[BLUE_PIN_INDEX], LOW);
  } else {
    digitalWrite(pins[BLUE_PIN_INDEX], HIGH);
  }
}
