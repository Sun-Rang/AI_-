#include <DHT.h>

#define NUM_SENSORS 4
#define DHTTYPE DHT11

String currentMode = "Unknown";
String lastSentMode = "Unknown";
float dayTemperature = 0.0;
float nightTemperature = 0.0;

int dhtPins[NUM_SENSORS] = {2, 3, 4, 5};
DHT dhts[NUM_SENSORS] = {
  DHT(dhtPins[0], DHTTYPE),
  DHT(dhtPins[1], DHTTYPE),
  DHT(dhtPins[2], DHTTYPE),
  DHT(dhtPins[3], DHTTYPE)
};

String sensorNames[NUM_SENSORS] = {"S001", "S002", "S003", "S004"};

unsigned long previousSensorMillis = 0;
const long sensorInterval = 60000;  // 10 minutes in milliseconds

void setup() {
  Serial.begin(9600);
  initializeSensors();

  delay(1000);

  sendSensorData();
}

void loop() {
  handleIncomingData();

  // Send mode only if it has changed
  if (lastSentMode != currentMode) {
    Serial.println("Mode Changed Successfully");  // This line sends the acknowledgment message
    sendMode();
    lastSentMode = currentMode;
  }

  unsigned long currentMillis = millis();

  // Send sensor data every 10 minutes
  if (currentMillis - previousSensorMillis >= sensorInterval) {
    sendSensorData();
    previousSensorMillis = currentMillis;
  }



  delay(7500); // Maintain the existing delay
}



void initializeSensors() {
  for (int i = 0; i < NUM_SENSORS; i++) {
    dhts[i].begin();
  }
}

void handleIncomingData() {
  if (Serial.available()) {
    String received_data = Serial.readStringUntil('\n');
    received_data.trim(); // This will trim the string in place
    processReceivedData(received_data);
  }
}

String gatherSensorData() {
  String data = "";
  for (int i = 0; i < NUM_SENSORS; i++) {
    float humidity = dhts[i].readHumidity();
    float temperature = dhts[i].readTemperature();

    if (isnan(humidity) || isnan(temperature)) {
      Serial.println("Failed to read from DHT sensor!");
      continue;  // Skip this sensor and move to the next one
    }

    data += sensorNames[i] + "," + String(temperature) + "," + String(humidity) + ",";
  }
  return data;
}

void sendSensorData() {
  String sensorData = gatherSensorData();
  Serial.println("TRSensor," + sensorData);
}

void sendMode() {
  Serial.println("Mode:" + currentMode);
}

void sendAcknowledgment(const char* message) {
  Serial.println(message);
}

void processReceivedData(String data) {
  if (data.startsWith("RT")) {
    String mode = data.substring(2);
    setMode(mode);
  } else if (data.startsWith("BT")) {
    int firstComma = data.indexOf(',', 2);
    int secondComma = data.indexOf(',', firstComma + 1);
    
    dayTemperature = data.substring(3, firstComma).toFloat();
    nightTemperature = data.substring(firstComma + 1, secondComma).toFloat();

    sendAcknowledgment("ACK: Temperatures Set");
  }
}

void setMode(String mode) {
  if (mode == "True") {
    currentMode = "Day";
    enterDayMode();
    
  } else {
    currentMode = "Night";
    enterNightMode();
  }

}


void enterDayMode() {
  // Implement any changes needed for the day mode.
}

void enterNightMode() {
  // Implement any changes needed for the night mode.
}
