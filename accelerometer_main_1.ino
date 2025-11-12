// Main firmware: sample LIS3DH, classify gestures, send keystrokes over USB
#include <Wire.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>
#include <Keyboard.h>
#include <NanoEdgeAI.h>
#include "knowledge.h"
float input_user_buffer[DATA_INPUT_USER * AXIS_NUMBER]; // Classifier input buffer
float output_class_buffer[CLASS_NUMBER]; // Class probabilities
uint16_t id_class = 0;
  
// Config
#define SERIAL_BAUD_RATE  115200

#define SENSOR_I2C_ADDR 0x18

#define SENSOR_DATA_RATE	LIS3DH_DATARATE_100_HZ

#define SENSOR_RANGE	LIS3DH_RANGE_4_G

#define NEAI_MODE 1
#define SENSOR_SAMPLES	256
#define AXIS  3

Adafruit_LIS3DH lis = Adafruit_LIS3DH(&Wire1);

// State
static uint16_t neai_ptr = 0;
static float neai_buffer[SENSOR_SAMPLES * AXIS] = {0.0};


// Initialization function
void setup() {
  // Serial console
  Serial.begin(SERIAL_BAUD_RATE);

  // Initialize LIS3DH over I2C
  if (!lis.begin(SENSOR_I2C_ADDR)) {
    Serial.println("Can't initialize I2C comm with LIS3DH sensor...\n");
    while(1);
  }
  Serial.println("OK");
  // Set range and data rate
  lis.setRange(SENSOR_RANGE);
  lis.setDataRate(SENSOR_DATA_RATE);

  // Initialize NanoEdge AI
  enum neai_state error_code = neai_classification_init(knowledge);
  if (error_code != NEAI_OK) {
    Serial.println("Error starting NanoEdge AI lib");
  }

  Keyboard.begin();
  delay(1000);
}

void loop() {
  // Collect one window of samples
  while (neai_ptr < SENSOR_SAMPLES) {
     // Wait for a fresh sample
    if (lis.haveNewData()) {
      // Read the sample
      lis.read();
      // Copy x/y/z into the window
      neai_buffer[AXIS * neai_ptr] = (float) lis.x;
      neai_buffer[(AXIS * neai_ptr) + 1] = (float) lis.y;
      neai_buffer[(AXIS * neai_ptr) + 2] = (float) lis.z;
      // Advance index
      neai_ptr++;
    }
  }
  // Start next window
  neai_ptr = 0;

  if (NEAI_MODE) {
    neai_classification(neai_buffer, output_class_buffer, &id_class);

    if (id_class == 1) {
      Keyboard.write(KEY_PAGE_DOWN);
      delay(100);  
    } else if (id_class == 2) {
      Keyboard.write(KEY_PAGE_UP);
      delay(100); 
    }
  } else {
    // Debug: dump raw samples
    for (uint16_t i = 0; i < AXIS * SENSOR_SAMPLES; i++) {
      Serial.print((String)neai_buffer[i] + " ");
    }
    Serial.print("\n");
  }

  // Clear buffer for the next pass
  memset(neai_buffer, 0.0, AXIS * SENSOR_SAMPLES * sizeof(float));
}