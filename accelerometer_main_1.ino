/* Main code run on accelerometer when testing model */
#include <Wire.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>
#include <Keyboard.h>
#include <NanoEdgeAI.h>
#include "knowledge.h"
float input_user_buffer[DATA_INPUT_USER * AXIS_NUMBER]; // Buffer of input values
float output_class_buffer[CLASS_NUMBER]; // Buffer of class probabilities
uint16_t id_class = 0;
  
/* Macros definitions */
#define SERIAL_BAUD_RATE  115200

#define SENSOR_I2C_ADDR 0x18

#define SENSOR_DATA_RATE	LIS3DH_DATARATE_100_HZ

#define SENSOR_RANGE	LIS3DH_RANGE_4_G

#define NEAI_MODE 1
#define SENSOR_SAMPLES	256
#define AXIS  3

Adafruit_LIS3DH lis = Adafruit_LIS3DH(&Wire1);

/* Global variables definitions */
static uint16_t neai_ptr = 0;
static float neai_buffer[SENSOR_SAMPLES * AXIS] = {0.0};


/* Initialization function */
void setup() {
  /* Init serial at baud rate 115200 */
  Serial.begin(SERIAL_BAUD_RATE);

  /* Init I2C connection between board & sensor */
  if (!lis.begin(SENSOR_I2C_ADDR)) {
    Serial.println("Can't initialize I2C comm with LIS3DH sensor...\n");
    while(1);
  }
  Serial.println("OK");
  /* Init LIS3DH with desired settings: odr & range */
  lis.setRange(SENSOR_RANGE);
  lis.setDataRate(SENSOR_DATA_RATE);

  /* Initialize NanoEdgeAI AI */
  enum neai_state error_code = neai_classification_init(knowledge);
  if (error_code != NEAI_OK) {
    Serial.println("Error starting NanoEdge AI lib");
  }

  Keyboard.begin();
  delay(1000);
}

void loop() {
  /* Get data in the neai buffer */
  while (neai_ptr < SENSOR_SAMPLES) {
     /* Check if new data if available */
    if (lis.haveNewData()) {
      /* If new data is available read */
      lis.read();
      /* Fill neai buffer with new accel data */
      neai_buffer[AXIS * neai_ptr] = (float) lis.x;
      neai_buffer[(AXIS * neai_ptr) + 1] = (float) lis.y;
      neai_buffer[(AXIS * neai_ptr) + 2] = (float) lis.z;
      /* Increment neai pointer */
      neai_ptr++;
    }
  }
  /* Reset pointer */
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
    /* Print the whole buffer to the serial */
    for (uint16_t i = 0; i < AXIS * SENSOR_SAMPLES; i++) {
      Serial.print((String)neai_buffer[i] + " ");
    }
    Serial.print("\n");
  }

  /* Clean neai buffer */
  memset(neai_buffer, 0.0, AXIS * SENSOR_SAMPLES * sizeof(float));
}