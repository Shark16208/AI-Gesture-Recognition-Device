// Simple data logger: read LIS3DH and print raw x/y/z over Serial
#include <Wire.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>

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

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);

  // Initialize LIS3DH over I2C
  if(!lis.begin(SENSOR_I2C_ADDR)) {
    Serial.println("Can't initialize I2C comm with LIS3DH sensor...\n");
    while(1);
  }
  Serial.println("OK");

  lis.setRange(SENSOR_RANGE);
  lis.setDataRate(SENSOR_DATA_RATE);

  delay(1000);
}

void loop() {
  // Collect one window of samples
  while(neai_ptr < SENSOR_SAMPLES) {
     // Wait for a fresh sample
    if(lis.haveNewData()) {
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

  
  // Dump the window over Serial (space-separated)
  for(uint16_t i = 0; i < AXIS * SENSOR_SAMPLES; i++) {
    Serial.print((String)neai_buffer[i] + " ");
  }
  Serial.print("\n");

  // Clear buffer for the next pass
  memset(neai_buffer, 0.0, AXIS * SENSOR_SAMPLES * sizeof(float));
}