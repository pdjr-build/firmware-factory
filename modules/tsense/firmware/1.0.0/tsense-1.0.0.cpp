/**********************************************************************
 * tsense-1.0.0.cpp - TSENSE firmware version 1.0.0.
 * Copyright (c) 2021 Paul Reeve, <preeve@pdjr.eu>
 *
 * This firmware provides an 8-channel temperature senor interface
 * that reports sensor state over NMEA 2000 using PGN 130316
 * Temperature, Extended Range.
 * 
 * The firmware supports LM335Z sensors.
 */

#include <Arduino.h>
#include <ADC.h>
#include <EEPROM.h>
#include <NMEA2000_CAN.h>
#include <N2kTypes.h>
#include <N2kMessages.h>
#include <Debouncer.h>
#include <LedManager.h>
#include <DilSwitch.h>
#include <Sensor.h>
#include <arraymacros.h>

/**********************************************************************
 * SERIAL DEBUG
 * 
 * Define DEBUG_SERIAL to enable serial output and arrange for the
 * function debugDump() to be called from loop() every
 * DEBUG_SERIAL_INTERVAL ms. DEBUG_SERIAL_START_DELAY prevents data
 * being written to the serial port immediately after system boot.
 */

#define DEBUG_SERIAL
#define DEBUG_SERIAL_START_DELAY 4000
#define DEBUG_SERIAL_INTERVAL 1000UL

/**********************************************************************
 * MCU EEPROM (PERSISTENT) STORAGE
 * 
 * SOURCE_ADDRESS_EEPROM_ADDRESS: storage address for the device's
 * 1-byte N2K source address.
 * SENSORS_EEPROM_ADDRESS: storage address for SENSOR congigurations.
 * The length of this is variable, so make sure it remains as the last
 * item.
 */

#define SOURCE_ADDRESS_EEPROM_ADDRESS 0
#define SENSORS_EEPROM_ADDRESS 1

/**********************************************************************
 * MCU PIN DEFINITIONS
 * 
 * GPIO pin definitions for the Teensy 3.2 MCU
 */

#define GPIO_INSTANCE_LED 0
#define GPIO_SOURCE_LED 1
#define GPIO_SETPOINT_LED 2
#define GPIO_ENCODER_BIT7 5
#define GPIO_ENCODER_BIT6 6
#define GPIO_ENCODER_BIT5 7
#define GPIO_ENCODER_BIT4 8
#define GPIO_ENCODER_BIT3 9
#define GPIO_ENCODER_BIT2 10
#define GPIO_ENCODER_BIT1 11
#define GPIO_ENCODER_BIT0 12
#define GPIO_BOARD_LED 13
// Start of analogue pinns - AX addresses are defined by the ADC library.
#define GPIO_SENSOR0 A0
#define GPIO_SENSOR1 A1
#define GPIO_SENSOR2 A2
#define GPIO_SENSOR3 A3
#define GPIO_SENSOR4 A4
#define GPIO_SENSOR5 A5
#define GPIO_SENSOR6 A6
#define GPIO_SENSOR7 A7
// End of analogue pins
#define GPIO_PROGRAMME_SWITCH 22
#define GPIO_POWER_LED 23
#define GPIO_SENSOR_PINS { GPIO_SENSOR0, GPIO_SENSOR1, GPIO_SENSOR2, GPIO_SENSOR3, GPIO_SENSOR4, GPIO_SENSOR5, GPIO_SENSOR6, GPIO_SENSOR7 } 
#define GPIO_ENCODER_PINS { GPIO_ENCODER_BIT0, GPIO_ENCODER_BIT1, GPIO_ENCODER_BIT2, GPIO_ENCODER_BIT3, GPIO_ENCODER_BIT4, GPIO_ENCODER_BIT5, GPIO_ENCODER_BIT6, GPIO_ENCODER_BIT7 }
#define GPIO_INPUT_PINS { GPIO_PROGRAMME_SWITCH, GPIO_ENCODER_BIT0, GPIO_ENCODER_BIT1, GPIO_ENCODER_BIT2, GPIO_ENCODER_BIT3, GPIO_ENCODER_BIT4, GPIO_ENCODER_BIT5, GPIO_ENCODER_BIT6, GPIO_ENCODER_BIT7 }
#define GPIO_OUTPUT_PINS { GPIO_BOARD_LED, GPIO_POWER_LED, GPIO_INSTANCE_LED, GPIO_SOURCE_LED, GPIO_SETPOINT_LED }

/**********************************************************************
 * DEVICE INFORMATION
 * 
 * Because of NMEA's closed standard, most of this is fiction. Maybe it
 * can be made better with more research. In particular, even recent
 * releases of the NMEA function and class lists found using Google
 * don't discuss anchor systems, so the proper values for CLASS and
 * FUNCTION in this application are not known.  At the moment they are
 * set to 25 (network device) and 130 (PC gateway).
 * 
 * INDUSTRY_GROUP we can be confident about (4 says maritime). However,
 * MANUFACTURER_CODE is only allocated to subscribed NMEA members and,
 * unsurprisingly, an anonymous code has not been assigned: 2046 is
 * currently unused, so we adopt that.  
 * 
 * MANUFACTURER_CODE and UNIQUE_NUMBER together must make a unique
 * value on any N2K bus and an easy way to achieve this is just to
 * bump the unique number for every software build and this is done
 * automatically by the build system.
 */

#define DEVICE_CLASS 30
#define DEVICE_FUNCTION 130
#define DEVICE_INDUSTRY_GROUP 4
#define DEVICE_MANUFACTURER_CODE 2046
#define DEVICE_UNIQUE_NUMBER 849

/**********************************************************************
 * PRODUCT INFORMATION
 * 
 * This poorly structured set of values is what NMEA expects a product
 * description to be shoe-horned into.
 */

#define PRODUCT_CERTIFICATION_LEVEL 1
#define PRODUCT_CODE 002
#define PRODUCT_FIRMWARE_VERSION "1.0.0 (Sep 2020)"
#define PRODUCT_LEN 1
#define PRODUCT_N2K_VERSION 2101
#define PRODUCT_SERIAL_CODE "002-849" // PRODUCT_CODE + DEVICE_UNIQUE_NUMBER
#define PRODUCT_TYPE "TSENSE"
#define PRODUCT_VERSION "1.0 (Mar 2021)"

/**********************************************************************
 * Include the build.h header file which can be used to override any or
 * all of the above  constant definitions.
 */

#include "build.h"

#define DEFAULT_SOURCE_ADDRESS 22         // Seed value for address claim
#define INSTANCE_UNDEFINED 255            // Flag value
#define STARTUP_SETTLE_PERIOD 5000        // Wait this many ms before processing switch inputs
#define SWITCH_PROCESS_INTERVAL 250       // Process switch inputs evety n ms
#define LED_MANAGER_HEARTBEAT 300         // Number of ms on / off
#define LED_MANAGER_INTERVAL 10           // Number of heartbeats between repeats
#define PROGRAMME_TIMEOUT_INTERVAL 20000  // Allow 20s to complete each programme step
#define SENSOR_PROCESS_INTERVAL 4000      // Number of ms between N2K transmits
#define SENSOR_VOLTS_TO_KELVIN 0.0489     // Conversion factor for LM335 temperature sensors

/**********************************************************************
 * Declarations of local functions.
 */
#ifdef DEBUG_SERIAL
void debugDump();
void dumpSensorConfiguration();
#endif
unsigned char getPoleInstance();
void messageHandler(const tN2kMsg&);
void processSensors();
bool processSwitches();
void transmitPgn130316(Sensor sensor);
void configureSensor(Sensor *sensor, DilSwitch *dilSwitch);
bool revertMachineStateMaybe();
void processMachineState();



/**********************************************************************
 * PGNs of messages transmitted by this program.
 * 
 * PGN 130316 Temperature, Extended Range is used to broadcast sensed
 * temperatures.
 */

const unsigned long TransmitMessages[] PROGMEM={ 130316L, 0 };

/**********************************************************************
 * PGNs of messages handled by this program.
 * 
 * There are none.
 */

typedef struct { unsigned long PGN; void (*Handler)(const tN2kMsg &N2kMsg); } tNMEA2000Handler;
tNMEA2000Handler NMEA2000Handlers[]={ {0, 0} };


int ENCODER_PINS[] = GPIO_ENCODER_PINS;
DilSwitch DIL_SWITCH (ENCODER_PINS, ELEMENTCOUNT(ENCODER_PINS));

/**********************************************************************
 * Create a switch debouncer DEBOUNCER and associate with it the GPIO
 * pins that are connected to switches.
 */

int SWITCHES[DEBOUNCER_SIZE] = { GPIO_PROGRAMME_SWITCH, -1, -1, -1, -1, -1, -1, -1 };
Debouncer DEBOUNCER (SWITCHES);

/**********************************************************************
 * Create an array of defined sensor pin addresses and a corresponding
 * array of SENSOR objects, then initialise each sensor object and set
 * its pin address.
 */
unsigned char SENSOR_PINS[] = GPIO_SENSOR_PINS;
Sensor SENSORS[ELEMENTCOUNT(SENSOR_PINS)];

/**********************************************************************
 * Create an LED manager with operating characteristics that suit the
 * status LEDS mounted on the module PCB.
 */

LedManager LED_MANAGER (LED_MANAGER_HEARTBEAT, LED_MANAGER_INTERVAL);

// Create an Analogue to Digital Converter service.
ADC *adc = new ADC();

/**********************************************************************
 *
 */

enum MACHINE_STATES { NORMAL, PRG_START, PRG_ACCEPT_INSTANCE, PRG_ACCEPT_SOURCE, PRG_ACCEPT_SETPOINT, PRG_FINALISE, PRG_CANCEL };
static MACHINE_STATES MACHINE_STATE = NORMAL;
unsigned long MACHINE_PROCESS_TIMER = 0UL;
unsigned long MACHINE_RESET_TIMER = 0UL;

/**********************************************************************
 * MAIN PROGRAM - setup()
 */

void setup() {
  #ifdef DEBUG_SERIAL
  Serial.begin(9600);
  delay(DEBUG_SERIAL_START_DELAY);
  #endif

  // Set the mode of all digital GPIO pins.
  int ipins[] = GPIO_INPUT_PINS;
  int opins[] = GPIO_OUTPUT_PINS;
  for (unsigned int i = 0 ; i < ELEMENTCOUNT(ipins); i++) pinMode(ipins[i], INPUT_PULLUP);
  for (unsigned int i = 0 ; i < ELEMENTCOUNT(opins); i++) pinMode(opins[i], OUTPUT);

  for (unsigned int i = 0; i < ELEMENTCOUNT(SENSOR_PINS); i++) SENSORS[i].invalidate(SENSOR_PINS[i]); 

  
  // We assume that a new host system has its EEPROM initialised to all
  // 0xFF. We test by reading a byte that in a configured system should
  // never be this value and if it indicates a scratch system then we
  // set EEPROM memory up in the following way.
  //
  // Address | Value                                    | Size in bytes
  // --------+------------------------------------------+--------------
  // 0x00    | N2K source address                       | 1
  // 0x10    | Sensor configuration (the SENSORS array) | Lots
  //
  if (EEPROM.read(SOURCE_ADDRESS_EEPROM_ADDRESS) == 0xff) {
    EEPROM.write(SOURCE_ADDRESS_EEPROM_ADDRESS, DEFAULT_SOURCE_ADDRESS);
    for (unsigned int i = 0; i < ELEMENTCOUNT(SENSORS); i++) SENSORS[i].save(SENSORS_EEPROM_ADDRESS + (i * SENSORS[i].getConfigSize()));
  }

  // Load sensor configurations from EEPROM  
  for (unsigned int i = 0; i < ELEMENTCOUNT(SENSORS); i++) SENSORS[i].load(SENSORS_EEPROM_ADDRESS + (i * SENSORS[i].getConfigSize()));

  
  // Flash the board n times (where n = number of configured sensors)
  int n = 0;
  for (unsigned int i = 0; i < ELEMENTCOUNT(SENSORS); i++) if (SENSORS[i].getInstance() != 0xff) n++;
  LED_MANAGER.operate(GPIO_BOARD_LED, 0, n);

  // Initialise and start N2K services.
  NMEA2000.SetProductInformation(PRODUCT_SERIAL_CODE, PRODUCT_CODE, PRODUCT_TYPE, PRODUCT_FIRMWARE_VERSION, PRODUCT_VERSION);
  NMEA2000.SetDeviceInformation(DEVICE_UNIQUE_NUMBER, DEVICE_FUNCTION, DEVICE_CLASS, DEVICE_MANUFACTURER_CODE);
  NMEA2000.SetMode(tNMEA2000::N2km_ListenAndNode, EEPROM.read(SOURCE_ADDRESS_EEPROM_ADDRESS)); // Configure for sending and receiving.
  NMEA2000.EnableForward(false); // Disable all msg forwarding to USB (=Serial)
  NMEA2000.ExtendTransmitMessages(TransmitMessages); // Tell library which PGNs we transmit
  NMEA2000.SetMsgHandler(messageHandler);
  NMEA2000.Open();

}

/**********************************************************************
 * MAIN PROGRAM - loop()
 * 
 * With the exception of NMEA2000.parseMessages() all of the functions
 * called from loop() implement interval timers which ensure that they
 * will mostly return immediately, only performing their substantive
 * tasks at the defined intervals.
 * 
 * The global constant JUST_STARTED is used to delay acting on switch
 * inputs until a newly started system has stabilised and the GPIO
 * inputs have been debounced.
 */ 

void loop() {
  static bool JUST_STARTED = true;
  if (JUST_STARTED && (millis() > STARTUP_SETTLE_PERIOD)) JUST_STARTED = false;
  
  // Debounce all switch inputs.
  DEBOUNCER.debounce();

  // If the system has settled (had time to debounce) then handle any
  // changes to MACHINE_STATE by reading the current value of DIL_SWITCH
  // and calling processMachineState(). Changes to MACHINE_STATE are
  // only made by processSwitches() and revertMachineStateMaybe(), both
  // of which return true if the make a change.
  if (!JUST_STARTED) {
    if (processSwitches() || revertMachineStateMaybe()) { DIL_SWITCH.sample(); processMachineState(); }
  }

  // Process any received messages.
  NMEA2000.ParseMessages();
  // The above call may have resulted in acquisition of a new source
  // address, so we check if there has been a change and if so save the
  // new address to EEPROM for future re-use.
  if (NMEA2000.ReadResetAddressChanged()) EEPROM.update(SOURCE_ADDRESS_EEPROM_ADDRESS, NMEA2000.GetN2kSource());

  // If the device isn't currently being programmed, then process
  // temperature sensors and transmit readings on N2K. 
  if (MACHINE_STATE == NORMAL) processSensors();

  // Update the states of connected LEDs
  LED_MANAGER.loop();

  // Implement programming timeout
  
  #ifdef DEBUG_SERIAL
  debugDump();
  #endif
}

/**********************************************************************
 * processSensors() recovers temperature data from all configured
 * temperature sensors and transmits it directly over N2K. The function
 * should be called directly from loop(): it will only process sensors
 * once per SENSOR_PROCESS_INTERVAL.
 */
void processSensors() {
  static unsigned long deadline = 0UL;
  unsigned long now = millis();

  if (now > deadline) {
    for (unsigned int sensor = 0; sensor < 8; sensor++) {
      if (SENSORS[sensor].getInstance() != 0xff) {
        int value = adc->analogRead(SENSORS[sensor].getGpio());
        if (value != ADC_ERROR_VALUE) {
          SENSORS[sensor].setTemperature(value * SENSOR_VOLTS_TO_KELVIN);
          transmitPgn130316(SENSORS[sensor]); 
        }
      }
    }
    deadline = (now + SENSOR_PROCESS_INTERVAL);
  }
}

/**********************************************************************
 * switchPressed() should be called directly from loop(). Most of the
 * time it will return false, but once every SWITCH_PROCESS_INTERVAL it
 * will revover the state of GPIO_PROGRAMME_SWITCH from DEBOUNCER and 
 * in this case return true if GPIO_PROGRAMME_SWITCH is depressed and
 * as a side-effect, advance the value of MACHINE_STATE.
 */
boolean processSwitches() {
  static unsigned long deadline = 0UL;
  unsigned long now = millis();
  unsigned retval = false;
  if (now > deadline) {
    retval = (DEBOUNCER.channelState(GPIO_PROGRAMME_SWITCH) == 0);
    deadline = (now + SWITCH_PROCESS_INTERVAL);
    if (retval) {
      switch (MACHINE_STATE) {
        case NORMAL: MACHINE_STATE = PRG_START; break;
        case PRG_START: MACHINE_STATE = PRG_ACCEPT_INSTANCE; break;
        case PRG_ACCEPT_INSTANCE: MACHINE_STATE = PRG_ACCEPT_SOURCE; break;
        case PRG_ACCEPT_SOURCE: MACHINE_STATE = PRG_ACCEPT_SETPOINT; break;
        case PRG_ACCEPT_SETPOINT: MACHINE_STATE = PRG_FINALISE; break;
        default: break;
      }
    }
  }
  return(retval);
}

/**********************************************************************
 * revertMachineStateMaybe() should be called directly from loop().
 * If the programming mode time window has elapsed because of user
 * inactivity, then MACHINE_STATE will be set to PRG_CANCEL and true
 * will be returned.
 */
boolean revertMachineStateMaybe() {
  boolean retval = false;
  if ((MACHINE_RESET_TIMER != 0UL) && (millis() > MACHINE_RESET_TIMER)) {
    MACHINE_STATE = PRG_CANCEL;
    retval = true;
  }
  return(retval);
}

/**********************************************************************
 * Should be called each time MACHINE_STATE is updated to implement any
 * necessary state change processing.
 */
void processMachineState() {
  static int selectedSensorIndex = -1;

  switch (MACHINE_STATE) {
    case NORMAL:
      break;
    case PRG_START:
      if (DIL_SWITCH.selectedSwitch()) {
        selectedSensorIndex = (DIL_SWITCH.selectedSwitch() - 1);
        LED_MANAGER.operate(GPIO_INSTANCE_LED, 0, -1);
        MACHINE_RESET_TIMER = (millis() + PROGRAMME_TIMEOUT_INTERVAL);
      } else {
        MACHINE_STATE = NORMAL;
        MACHINE_RESET_TIMER = 0UL;
      }
      break;
    case PRG_ACCEPT_INSTANCE:
      SENSORS[selectedSensorIndex].setInstance(DIL_SWITCH.value());
      LED_MANAGER.operate(GPIO_INSTANCE_LED, 1);
      LED_MANAGER.operate(GPIO_SOURCE_LED, 0, -1);
      MACHINE_RESET_TIMER = (millis() + PROGRAMME_TIMEOUT_INTERVAL);
      break;
    case PRG_ACCEPT_SOURCE:
        SENSORS[selectedSensorIndex].setSource(DIL_SWITCH.value());
        LED_MANAGER.operate(GPIO_SOURCE_LED, 1);
        LED_MANAGER.operate(GPIO_SETPOINT_LED, 0, -1);
        MACHINE_RESET_TIMER = (millis() + PROGRAMME_TIMEOUT_INTERVAL);
        break;
    case PRG_ACCEPT_SETPOINT:
      SENSORS[selectedSensorIndex].setSetPoint((double) DIL_SWITCH.value());
      LED_MANAGER.operate(GPIO_SETPOINT_LED, 1);
    case PRG_FINALISE:
      // Save in-memory configuration to EEPROM, flash LEDs to confirm
      // programming and return to normal operation.
      #ifdef DEBUG_SERIAL
      Serial.println("PRG_FINALISE: finalising programme and saving configuration");
      #endif
      SENSORS[selectedSensorIndex].save(SENSORS_EEPROM_ADDRESS + (selectedSensorIndex * SENSORS[selectedSensorIndex].getConfigSize()));
      MACHINE_STATE = NORMAL;
      MACHINE_RESET_TIMER = 0UL;
      LED_MANAGER.operate(GPIO_INSTANCE_LED, 0, 3);
      LED_MANAGER.operate(GPIO_SOURCE_LED, 0, 3);
      LED_MANAGER.operate(GPIO_SETPOINT_LED, 0, 3);
      break;
    case PRG_CANCEL:
      // Restore in-memory configuration from EEPROM and return to
      // normal operation.
      #ifdef DEBUG_SERIAL
      Serial.println("PRG_CANCEL: cancelling programme mode and restoring configuration");
      #endif
      SENSORS[selectedSensorIndex].load(SENSORS_EEPROM_ADDRESS + (selectedSensorIndex * SENSORS[selectedSensorIndex].getConfigSize()));
      MACHINE_STATE = NORMAL;
      MACHINE_RESET_TIMER = 0UL;
      break;
  }
  #ifdef DEBUG_SERIAL
    Serial.print("Operating configuration = ");
    dumpSensorConfiguration();
  #endif
}


/**********************************************************************
 */

void transmitPgn130316(Sensor sensor) {
  tN2kMsg N2kMsg;
  SetN2kPGN130316(N2kMsg, 0, sensor.getInstance(), sensor.getSource(), sensor.getTemperature(), sensor.getSetPoint());
  NMEA2000.SendMsg(N2kMsg);
}  

/**********************************************************************
 * Return the integer value represented by the state of the digital
 * inputs passed in the <pins> array. Pin addresses are assumed to be
 * in the order lsb through msb.
 */
unsigned char getEncodedByte(int *pins) {
  unsigned char retval = 0x00;
  for (unsigned int i = 0; i < ELEMENTCOUNT(pins); i++) {
    retval = retval + (digitalRead(pins[i] << i));
  }
  return(retval);
}

void messageHandler(const tN2kMsg &N2kMsg) {
  int iHandler;
  for (iHandler=0; NMEA2000Handlers[iHandler].PGN!=0 && !(N2kMsg.PGN==NMEA2000Handlers[iHandler].PGN); iHandler++);
  if (NMEA2000Handlers[iHandler].PGN!=0) {
    NMEA2000Handlers[iHandler].Handler(N2kMsg); 
  }
}

#ifdef DEBUG_SERIAL
void debugDump() {
  static unsigned long deadline = 0UL;
  unsigned long now = millis();
  if (now > deadline) {
    deadline = (now + DEBUG_SERIAL_INTERVAL);
  }
}

void dumpSensorConfiguration() {
  Serial.print("[");
  for (unsigned int i = 0; i < ELEMENTCOUNT(SENSORS); i++) {
    if (i != 0) Serial.print(",");
    Serial.print(" {");
    Serial.print("\"instance\": "); Serial.print(SENSORS[i].getInstance()); Serial.print(",");
    Serial.print("\"source\": "); Serial.print(SENSORS[i].getSource()); Serial.print(",");
    Serial.print("\"setPoint\": "); Serial.print(SENSORS[i].getSetPoint());
    Serial.print("}");
  }
  Serial.println(" ]");
}
#endif
