// Terrier Motorsport - Cooling Loop Code
// Michael Waetzman, mwae@bu.edu
// October 2024

// ! TEST PLANNED FOR SATURDAY 10/12 (maybe that Sunday)

// Attributions
// SD and FreeRTOS Library Example Code

// To-Do
// 1. Fan Control

#include <SPI.h>
#include <SD.h>
#include <Wire.h>  // i2c included in here
#include <Arduino_FreeRTOS.h>
#include <semphr.h>

SemaphoreHandle_t xSerialSemaphore;

// Pin Definitions
// SPI: 10-13 are reserved for SPI (used by the SD card)
const int chipSelect = 10;
// Analog: for the NTC temperature sensor
const int NTC_PIN = 14;  // A0

const String filename = "cooling_test_data.csv";


// ==============================================
// ||                 SET-UP                   ||
// ==============================================

void setup() {
  Serial.begin(9600);
  while (!Serial) { ; }

  pinMode(chipSelect, OUTPUT);
  pinMode(NTC_PIN, INPUT);

  // Initialize the SD cars, check info
  SD_init();

  // Init I2C Wire
  Wire.begin();

}

void loop() {
  float ntc_temp, pt_temp, pt_pressure;

  // Read Data
  ntc_temp = read_NTC_temp();
  pt_temp = read_PT_temp();
  pt_pressure = read_PT_pressure();

  // Show Data on Serial Plot/Monitor
  String serial_str = "NTC TEMP: " + String(ntc_temp) + "°C\n" + "PT TEMP: " + String(pt_temp) + "°C\n" + "PT PRESSURE: " + String(pt_pressure) + "units...\n\n";
  Serial.print(serial_str);

  // Log Data
  String data_csv_form = String(ntc_temp) + "," + String(pt_temp) + "," + String(pt_pressure);

  if (SD.exists(filename)) {
    File sd = SD.open(filename, FILE_WRITE);
    sd.println(data_csv_form);
  } else {
    Serial.println("ERROR: File not found, creating now.");
    SD.open(filename, FILE_WRITE);
    File sd = SD.open(filename, FILE_WRITE);
    sd.println(data_csv_form);
  }

}

// ==============================================
// ||                FUNCTIONS                  ||
// ==============================================

void SD_init() {
  // Run checks to ensure the SD card is connected

  Serial.print("Initializing SD card...");
  if (!SD.begin(chipSelect)) {
    Serial.println("initialization failed. Things to check:");
    Serial.println("1. is a card inserted?");
    Serial.println("2. is your wiring correct?");
    Serial.println("3. did you change the chipSelect pin to match your shield or module?");
    Serial.println("Note: Talk to Mike. Process Terminated.");
    while (true)
      ;
  }

  Serial.println("initialization done.");
}

float read_PT_pressure() {
  // Uses I2C to send 14bit temperature value (in PSI/bar--idk...)
    //! ADDRESS FOR OUR PT???
  float pt_pressure;

  return pt_pressure;
}

float read_PT_temp() {
  // Uses I2C to send 11bit temperature value (in °C)
  float pt_temp;

  return pt_temp;
}

float read_NTC_temp() {
  // Finds resistance from sensor, assuming an "R2" value of 1k Ohm
  // Assuming linear ΔR between increments of 10°C
  float ntc_temp;
  int ntc_temp_map;
  float ntc_raw_voltage = analogRead(NTC_PIN);
  int ntc_resistance = (5000 / ntc_raw_voltage) - 1000;

  if (ntc_resistance > 3792) {
    // 0°C to 10°C
    ntc_temp_map = map(ntc_resistance, 3792, 5896, 0, 10);
    ntc_temp = 10 - ntc_temp_map;
  } else if (ntc_resistance > 2500) {
    // 10°C to 20°C
    ntc_temp_map = map(ntc_resistance, 2500, 3792, 0, 10);
    ntc_temp = 20 - ntc_temp_map;
  } else if (ntc_resistance > 1707) {
    // 20°C to 30°C
    ntc_temp_map = map(ntc_resistance, 2500, 3792, 0, 10);
    ntc_temp = 30 - ntc_temp_map;
  } else if (ntc_resistance > 1175) {
    // 30°C to 40°C
    ntc_temp_map = map(ntc_resistance, 1175, 3792, 0, 10);
    ntc_temp = 40 - ntc_temp_map;
  } else if (ntc_resistance > 834) {
    // 40°C to 50°C
    ntc_temp_map = map(ntc_resistance, 834, 1175, 0, 10);
    ntc_temp = 50 - ntc_temp_map;
  } else if (ntc_resistance > 596) {
    // 50°C to 60°C
    ntc_temp_map = map(ntc_resistance, 596, 834, 0, 10);
    ntc_temp = 60 - ntc_temp_map;
  } else if (ntc_resistance > 436) {
    // 60°C to 70°C
    ntc_temp_map = map(ntc_resistance, 436, 596, 0, 10);
    ntc_temp = 70 - ntc_temp_map;
  } else if (ntc_resistance > 323) {
    // 70°C to 80°C
    ntc_temp_map = map(ntc_resistance, 323, 436, 0, 10);
    ntc_temp = 80 - ntc_temp_map;
  } else if (ntc_resistance > 243) {
    // 80°C to 90°C
    ntc_temp_map = map(ntc_resistance, 243, 323, 0, 10);
    ntc_temp = 90 - ntc_temp_map;
  } else if (ntc_resistance > 187) {
    // 90°C to 100°C
    ntc_temp_map = map(ntc_resistance, 187, 243, 0, 10);
    ntc_temp = 100 - ntc_temp_map;
  } else {
    // indicates error
    ntc_temp = -999;
  }

  return ntc_temp;
}
