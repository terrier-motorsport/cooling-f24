// Terrier Motorsport - Cooling Loop Code
// Michael Waetzman, mwae@bu.edu
// November 2024

// Attributions
  // SD Example Code

// NOTE: NTC pullup R = 1kΩ, PT/I2C pullup R=4.7kΩ

#include <SPI.h>
#include <SD.h>

// Pin Definitions //

// SPI: 10-13 are reserved for SPI (used by the SD card)
const int chipSelect = 10;

// Analog: for the NTC temperature sensor & M3200 PT
const int NTC_PIN = A0;
const int PT_PIN = A1;

File data_file;
const String filename = "data.csv";

float maxPressure = 250;
const int PTV_MAX = 4.5,
          PTV_MIN = 0.5,
          PTP_MAX = 250,
          PTP_MIN = 0;

// ==============================================
// ||              SET-UP & LOOP               ||
// ==============================================

void setup() {
  Serial.begin(9600);
  while (!Serial) { ; }

  pinMode(chipSelect, OUTPUT);
  pinMode(NTC_PIN, INPUT);
  pinMode(PT_PIN, INPUT);

  // Initialize the SD cars, check info
  SD_init();

  // Header of CSV Table
  File dataFile = SD.open(filename, FILE_WRITE);  
  String dataString = "Time (ms),NTC Temp (F),PT1 Pres (PSI)";
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
    Serial.println(dataString);
  } else {
    Serial.println("file couldn't open :(");
    while(true);
  }
}

void loop() {
  Serial.println("Logging");
  float ntc_temp, pt_temp, pt_pres;
  File dataFile = SD.open(filename, FILE_WRITE);

  // Gather Data
  ntc_temp = read_NTC_temp();
  pt_pres = read_PT_analog();

  // Store Data as string -> SD card
  String dataString = String(millis()) + "," + String(ntc_temp) + "," + String(pt_pres);

  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
    Serial.println(dataString);
  } else {
    Serial.println("file couldn't open :(");
    while(true);
  }

  delay(500);
}

// ==============================================
// ||                FUNCTIONS                  ||
// ==============================================

void SD_init(){
  Serial.print("Initializing SD card...");

  if (!SD.begin(chipSelect)) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("initialization done.");
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

float read_PT_analog(){
  float pres, pres_raw;

  pres_raw = analogRead(PT_PIN);
  Serial.println("Pressure Digital: " + String(pres_raw));
  pres = ((15000-1000) / (PTP_MAX-PTP_MIN)) * (pres_raw-PTP_MIN) + 1000;

  return pres;
}



