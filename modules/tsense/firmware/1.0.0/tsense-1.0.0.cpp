/**********************************************************************
 * tsense-1.0.0.cpp - TSENSE firmware version 1.0.0.
 * Copyright (c) 2021 Paul Reeve, <preeve@pdjr.eu>
 *
 * This firmware provides an 8-channel temperature senor interface
 * that reports sensor state using NMEA 2000 Temperature, Extended
 * Range PGN 130316.
 * 
 * The firmware supports LM335Z sensors.
 */

#include <Arduino.h>
#include <EEPROM.h>
#include <NMEA2000_CAN.h>
#include <N2kTypes.h>
#include <N2kMessages.h>
#include <Debouncer.h>
#include <LedManager.h>
#include <Sensor.h>
#include <arraymacros.h>

/**********************************************************************
 * DEBUG AND TESTING
 * 
 * Defining DEBUG_SERIAL includes the function debugDump() and arranges
 * for it to be called from loop() every DEBUG_SERIAL_INTERVAL ms.
 * 
 * Defining DEBUG_USE_DEBUG_ADDRESSES disables normal instance number
 * recovery from EEPROM and remote windlass address recovery from the
 * network and instead forces the use of the values defined here. 
 */

#define DEBUG_SERIAL
#define DEBUG_USE_DEBUG_ADDRESSES

#define DEBUG_SERIAL_START_DELAY 4000
#define DEBUG_SERIAL_INTERVAL 1000UL

/**********************************************************************
 * MCU EEPROM STORAGE DEFINITIONS
 * 
 * These two addresses specify the persistent storage address that
 * should be used to store the 1-byte remote windlass instance numbers. 
 */

#define SOURCE_ADDRESS_EEPROM_ADDRESS 0
#define SENSORS_STATE_EEPROM_ADDRESS 2

/**********************************************************************
 * MCU DIGITAL IO PIN DEFINITIONS
 * 
 * GPIO pin definitions for the Teensy 3.2 MCU
 */

#define GPIO_PROGRAMME_LED1 0
#define GPIO_PROGRAMME_LED2 1
#define GPIO_PROGRAMME_SWITCH 2
#define GPIO_INSTANCE_BIT7 5
#define GPIO_INSTANCE_BIT6 6
#define GPIO_INSTANCE_BIT5 7
#define GPIO_INSTANCE_BIT4 8
#define GPIO_INSTANCE_BIT3 9
#define GPIO_INSTANCE_BIT2 10
#define GPIO_INSTANCE_BIT1 11
#define GPIO_INSTANCE_BIT0 12
#define GPIO_BOARD_LED 13
#define GPIO_T0 A0
#define GPIO_T1 A1
#define GPIO_T2 A2
#define GPIO_T3 A3
#define GPIO_T4 A4
#define GPIO_T5 A5
#define GPIO_T6 A6
#define GPIO_T7 A7
#define GPIO_POWER_LED 23
#define GPIO_INSTANCE_PINS { GPIO_INSTANCE_BIT0, GPIO_INSTANCE_BIT1, GPIO_INSTANCE_BIT2, GPIO_INSTANCE_BIT3, GPIO_INSTANCE_BIT4, GPIO_INSTANCE_BIT5, GPIO_INSTANCE_BIT6, GPIO_INSTANCE_BIT7 }
#define GPIO_INPUT_PINS { GPIO_PROGRAMME_SWITCH, GPIO_INSTANCE_BIT0, GPIO_INSTANCE_BIT1, GPIO_INSTANCE_BIT2, GPIO_INSTANCE_BIT3, GPIO_INSTANCE_BIT4, GPIO_INSTANCE_BIT5, GPIO_INSTANCE_BIT6, GPIO_INSTANCE_BIT7 }
#define GPIO_OUTPUT_PINS { GPIO_BOARD_LED, GPIO_POWER_LED, GPIO_PROGRAMME_LED1, GPIO_PROGRAMME_LED2 }

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
#define RELAY_UPDATE_INTERVAL 330         // Update outputs every n ms
#define STATUS_LED_MANAGER_HEARTBEAT 300  // Settings for LEDs on module case
#define STATUS_LED_MANAGER_INTERVAL 10    //

/**********************************************************************
 * Declarations of local functions.
 */

#ifdef DEBUG_SERIAL
void debugDump();
#endif
unsigned char getPoleInstance();
void messageHandler(const tN2kMsg&);
void processSwitches(WindlassState **windlasses);
void transmitWindlassControl(WindlassState *windlass);
void operateOutputs(WindlassState *windlass);

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

/**********************************************************************
 * Create a switch debouncer DEBOUNCER and associate with it the GPIO
 * pins that are connected to switches.
 */

int SWITCHES[DEBOUNCER_SIZE] = { GPIO_PROGRAMME_SWITCHWITCH, -1, -1, -1, -1, -1, -1, -1 };
Debouncer DEBOUNCER (SWITCHES);

/**********************************************************************
 * Create an LED manager with operating characteristics that suit the
 * status LEDS mounted on the module PCB.
 */

LedManager STATUS_LED_MANAGER (STATUS_LED_MANAGER_HEARTBEAT, STATUS_LED_MANAGER_INTERVAL);

SENSOR SENSORS[8];
int SELECTED_SENSOR = -1;
for (int i = 0; i < 8; i++) SENSORS[i].invalidate(); 

enum { NORMAL, WAITINGFORINSTANCE, WAITINGFORSOURCE, WAITINGFORSETPOINT } MODE = NORMAL;

/**********************************************************************
 * MAIN PROGRAM - setup()
 */

void setup() {
  #ifdef DEBUG_SERIAL
  Serial.begin(9600);
  delay(DEBUG_SERIAL_START_DELAY);
  #endif

  int ipins[] = GPIO_INPUT_PINS;
  int opins[] = GPIO_OUTPUT_PINS;
  for (unsigned int i = 0 ; i < ELEMENTCOUNT(ipins); i++) { pinMode(ipins[i], INPUT_PULLUP); }
  for (unsigned int i = 0 ; i < ELEMENTCOUNT(opins); i++) { pinMode(opins[i], OUTPUT); }

  // The first time this thing runs, there will be no previously used
  // source address saved in EEPROM - in fact an EEPROM read will likely
  // return the network broadcast address, so if this is the case, then
  // we write an arbitrary, non-broadcast, source address to EEPROM and 
  // take the opportunity to also save our unconfigured SENSORS structure.
  if (EEPROM.read(SOURCE_ADDRESS_EEPROM_ADDRESS) == 255) {
    EEPROM.update(SOURCE_ADDRESS_EEPROM_ADDRESS, DEFAULT_SOURCE_ADDRESS);
    SENSORS.saveState(SENSORS_STATE_EEPROM_ADDRESS);
  }
  
  SENSORS.loadState(SENSORS_STATE_EEPROM_ADDRESS); 
  
  STATUS_LED_MANAGER.operate(GPIO_BOARD_LED, 0, 3);

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

  DEBOUNCER.debounce();
  if (!JUST_STARTED) processSwitches();

  // Process any received messages.
  NMEA2000.ParseMessages();
  // The above may have resulted in acquisition of a new source
  // address, so we check if there has been a change and if so save the
  // new address to EEPROM for future re-use.
  if (NMEA2000.ReadResetAddressChanged()) EEPROM.update(SOURCE_ADDRESS_EEPROM_ADDRESS, NMEA2000.GetN2kSource());

  STATUS_LED_MANAGER.loop();
  STATE_LED_MANAGER.loop();
  
  #ifdef DEBUG_SERIAL
  debugDump();
  #endif
}

/**********************************************************************
 * processSwitches() invokes various handlers dependent upon the state
 * of their associated switch input channels.
 */

void processSwitches() {
  if (DEBOUNCER.channelState(GPIO_PROGRAMME_SWITCH)) {
    int s = getDipSetting();
    switch MODE {
      case NORMAL:
        if (s & (s - 1)) == 0) {
          for (SELECTED_SENSOR = 0; ((SELECTED_SENSOR < 8) && ((2^SELECTED_SENSOR) != s)); SELECTED_SENSOR++);
          MODE = WAITINGFORINSTANCE;
          STATUS_LED_MANAGER.operate(GPIO_PROGRAMME_LED1, 0, 1);
          // Set programmme timeout
        } else {
          STATUS_LED_MANAGER.operate(GPIO_PROGRAMME_LED1, 3, 0);
        }
        break;
      case WAITINGFORINSTANCE:
        PROGRAMME_SENSOR_INSTANCE = s;
        MODE = WAITINGFORSOURCE;
        // Set PROG LED state to single flash
        // Set programmme timeout
        break;
      case WAITINGFORSOURCE:
        PROGRAMME_SENSOR_SOURCE = s;
        MODE = WAITINGFORSETPOINT;
        // Set PROG LED state to single flash
        // Set programmme timeout
        break;
      case WAITINGFORSETPOINT:
        PROGRAM_SET_POINT = s;
        MODE = NORMAL;
        // Set SENSORS to programmed value and save to EEPROM
        // Set PROG LED to confirm success
        // Cancel programme timeout
        break;
    }
  }
}

/**********************************************************************
 * transmitWindlassControl() sends a Group Function control message for
 * PGN128776 Windlass Control Status to the device identified by
 * <windlass>, setting the Windlass Direction Control property to
 * reflect the state of the <up> and <down> GPIO inputs.
 * 
 * transmitWindlassControl() will not operate if a status transmission
 * has not previously been received from the network node defined by
 * <windlass> since the CAN address of the target windlass will be
 * unknown.
 * 
 * The value of the global constant SWITCH_PROCESS_INTERVAL is
 * important because it defines the frequency at which windlass control
 * messages will be issued: this is defined by the NMEA standard as
 * 250ms.
 */

void transmitWindlassControl(WindlassState *windlass) {
  tN2kMsg N2kMsg;
  N2kMsg.SetPGN(126208UL);
  N2kMsg.Priority = 2;
  N2kMsg.Destination = windlass->address;
  N2kMsg.AddByte(0x01); // Command message
  N2kMsg.Add3ByteInt(128776UL); // Windlass Control Status PGN
  N2kMsg.AddByte(0xF8); // Retain existing priority
  N2kMsg.AddByte(0x01); // Just one parameter pair to follow
  N2kMsg.AddByte(0x03); // Parameter 1 - Field 3 is Windlass Direction Control
  N2kMsg.AddByte((windlass->pDebouncer->channelState(windlass->upSwitchGPIO))?0x02:((windlass->pDebouncer->channelState(windlass->downSwitchGPIO))?0x01:0x00));
  NMEA2000.SendMsg(N2kMsg);
}  

/**********************************************************************
 * getPoleInstance() returns the 8-bit instance address set by the
 * hardware DIP switches defined in GPIO_INSTANCE (the pin sequence
 * supplied must be lo-bit to hi-bit). If GPIO_INSTANCE is not defined
 * then returns 0xFF.
 */

unsigned char getDipSettinig() {
  unsigned char instance = 0xFF;
  #ifdef GPIO_INSTANCE_PINS
  instance = 0x00;
  int ipins[] = GPIO_INSTANCE_PINS; 
  for (byte i = 0; i < ELEMENTCOUNT(ipins); i++) {
    instance = instance + (digitalRead(ipins[i]) << i);
  }
  #endif
  return(instance);
}

void messageHandler(const tN2kMsg &N2kMsg) {
  int iHandler;
  for (iHandler=0; NMEA2000Handlers[iHandler].PGN!=0 && !(N2kMsg.PGN==NMEA2000Handlers[iHandler].PGN); iHandler++);
  if (NMEA2000Handlers[iHandler].PGN!=0) {
    NMEA2000Handlers[iHandler].Handler(N2kMsg); 
  }
}

/**********************************************************************
 * PGN128777() parses the supplied N2K message and uses the contained
 * windlass operating status data to update the WINDLASSES global's
 * upRelayState and dnRelayState fields.
 */

void PGN128777(const tN2kMsg &N2kMsg) {
  unsigned char SID;
  unsigned char WindlassIdentifier;
  double RodeCounterValue;
  double WindlassLineSpeed;
  tN2kWindlassMotionStates WindlassMotionStatus;
  tN2kRodeTypeStates RodeTypeStatus;
  tN2kAnchorDockingStates AnchorDockingStatus;
  tN2kWindlassOperatingEvents WindlassOperatingEvents;
  WindlassState *windlass = NULL;

  if (ParseN2kPGN128777(N2kMsg, SID, WindlassIdentifier, RodeCounterValue, WindlassLineSpeed, WindlassMotionStatus, RodeTypeStatus, AnchorDockingStatus, WindlassOperatingEvents)) {
    for (unsigned int i = 0; i < ELEMENTCOUNT(WINDLASSES); i++) {
      if ((!WINDLASSES[i]->isDisabled()) && (WINDLASSES[i]->instance == WindlassIdentifier)) {
        windlass = WINDLASSES[i];
      }
    }

    if (windlass != NULL) {
      if (!windlass->isConfigured()) windlass->address = N2kMsg.Source;
      if (windlass->isReady()) {
        // And now set the relay states
        if (AnchorDockingStatus == N2kDD482_FullyDocked) {
          windlass->state = WindlassState::DOCKED;
        } else {
          switch (WindlassMotionStatus) {
            case N2kDD480_DeploymentOccurring:
              windlass->state = WindlassState::DEPLOYING;
              break;
            case N2kDD480_RetrievalOccurring:
              windlass->state = WindlassState::RETRIEVING;
              break;
            default:
              windlass->state = WindlassState::DEPLOYED;
              break;
          }
        }
        operateOutputs(windlass);
      }
    }
  }
}

/**********************************************************************
 * Operates the UP and DOWN LED relays associated with the specified
 * <windlass> in responce to the value of the windlass state property.
 */

void operateOutputs(WindlassState *windlass) {
  switch (windlass->state) {
    case WindlassState::DOCKED:
      windlass->pStatusLedManager->operate(windlass->upLedGPIO, 1);
      windlass->pStatusLedManager->operate(windlass->downLedGPIO, 0);
      break;
    case WindlassState::DEPLOYING:
      windlass->pStatusLedManager->operate(windlass->upLedGPIO, 0);
      windlass->pStatusLedManager->operate(windlass->downLedGPIO, 0, -1);
      break;
    case WindlassState::RETRIEVING:
      windlass->pStatusLedManager->operate(windlass->upLedGPIO, 0, -1);
      windlass->pStatusLedManager->operate(windlass->downLedGPIO, 0);
      break;
    case WindlassState::DEPLOYED:
      windlass->pStatusLedManager->operate(windlass->upLedGPIO, 0);
      windlass->pStatusLedManager->operate(windlass->downLedGPIO, 1);
      break;
    case WindlassState::UNKNOWN:
      windlass->pStatusLedManager->operate(windlass->upLedGPIO, 0);
      windlass->pStatusLedManager->operate(windlass->downLedGPIO, 0);
      break;
  }
}

#ifdef DEBUG_SERIAL
void debugDump() {
  static unsigned long deadline = 0UL;
  unsigned long now = millis();
  if (now > deadline) {
    Serial.print("DEBUG DUMP @ "); Serial.println(now);
    Serial.print("W0 instance:  "); Serial.println(WINDLASS0.instance, HEX);
    Serial.print("W0 address:   "); Serial.println(WINDLASS0.address, HEX);
    Serial.print("W0 UP switch: "); Serial.println(WINDLASS0.pDebouncer->channelState(WINDLASS0.upSwitchGPIO));
    Serial.print("W0 DN switch: "); Serial.println(WINDLASS0.pDebouncer->channelState(WINDLASS0.downSwitchGPIO));
    deadline = (now + DEBUG_SERIAL_INTERVAL);
  }
}
#endif
