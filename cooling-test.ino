// Terrier Motorsport - Cooling Loop Code
// Michael Waetzman, mwae@bu.edu
// November 2024

// Attributions
// SD and FreeRTOS Library Example Code
// hedaselish's "M3200 Pressure Transducer Arduino"
  // https://www.instructables.com/M3200-Pressure-Transducer-Arduino/

// NOTE: NTC pullup R = 1kΩ, PT/I2C pullup R=4.7kΩ

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

// I2C: Address for M3200 Pressure Transducer
const int PT_ADDR = 0x28;

const String filename = "cooling_test_data.csv";

// Basically creating a "new variable type" that can store two values
struct PT_Data{
  uint16_t pt_pressure;
  uint16_t pt_temperature;
};
byte status;
float maxPressure = 100;    //! VERIFY THIS

// ==============================================
// ||              SET-UP & LOOP               ||
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
  float ntc_temp, pt_temp, pt_pres;

  // Read Data
  ntc_temp = read_NTC_temp();

  PT_Data pt_data = read_PT();
  pt_temp = pt_data.pt_temperature;
  pt_pres = pt_data.pt_pressure;

  // Show Data on Serial Plot/Monitor
  String serial_str = "NTC TEMP: " + String(ntc_temp) + "°C\n" + "PT TEMP: " + String(pt_temp) + "°C\n" + "PT PRESSURE: " + String(pt_pres) + "units...\n\n";
  Serial.print(serial_str);

  // Log Data w/ time stamp (milis)
  int long current_time = millis();
  String data_csv_form = String(current_time) + "," + String(ntc_temp) + "," + String(pt_temp) + "," + String(pt_pres);

  if (SD.exists(filename)) {
    File sd = SD.open(filename, FILE_WRITE);
    sd.println(data_csv_form);
  } else {
    Serial.println("ERROR: File not found, creating now.");
    SD.open(filename, FILE_WRITE);
    File sd = SD.open(filename, FILE_WRITE);
    sd.println(data_csv_form);
  }

  // Half second delay so we're not collecting more data than the SD card can store 
  delay(500);
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

PT_Data read_PT() {
  // Read from I2C  temperature and pressure values (in C? and PSI)

  // Storing data in a struct (could use pointers, but this is easier for non CompE/CS to understand)
  PT_Data current_data = {0,0}; // pressure, temperature

  Wire.requestFrom(PT_ADDR, 4);   // See fig 1.6.1, shows read commands

  int n = Wire.available();
  if(n == 4){
    status = 0;
    uint16_t rawP;     // pressure data from sensor
    uint16_t rawT;     // temperature data from sensor
    rawP = (uint16_t) Wire.read();    // upper 8 bits
    rawP <<= 8;
    rawP |= (uint16_t) Wire.read();    // lower 8 bits
    rawT = (uint16_t)  Wire.read();    // upper 8 bits
    rawT <<= 8;
    rawT |= (uint16_t) Wire.read();   // lower 8 bits

    status = rawP >> 14;   // The status is 0, 1, 2 or 3
    rawP &= 0x3FFF;   // keep 14 bits, remove status bits

    rawT >>= 5;     // the lowest 5 bits are not used
    float pressure = ((rawP - 1000.0) / (15000.0 - 1000.0)) * maxPressure;
    float temperature = ((rawT - 512.0) / (1075.0 - 512.0)) * 55.0;
    
    current_data.pt_pressure = pressure;
    current_data.pt_temperature = temperature;

  } else {
    Serial.println("Pressure Transducer not detected.");
  }

  return current_data;
}


float read_NTC_temp() {
  // Finds resistance from sensor, assuming a pullup R = 1kΩ
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
