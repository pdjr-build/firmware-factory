/**********************************************************************
 * winctl-1.0.0.cpp - WINCTL firmware version 1.0.0.
 * Copyright (c) 2020 Paul Reeve, <preeve@pdjr.eu>
 *
 * This firmware allows control of one or two remote windlasses using
 * the NMEA 2000 Windlass Network Messages protocol based around PGNs
 * 128776, 128777 and 128778.
 * 
 * The firmware supports two control channels W0 and W1. Each control
 * channel is configured by a single unsigned byte which supplies the
 * instance number of the remote windlass that should be associated
 * with the channel. These instance numbers are recovered from EEPROM
 * when the firmware boots. A set of seven GPIO inputs allow the
 * connection of a seven bit DIP switch that can be used to specify
 * an instance number. A further two GPIO inputs save the configured
 * instance number to EEPROM when set high.
 * 
 * The firmware continuously monitors the NMEA bus for PGN 128777
 * Windlass Operating Status messages decorated with an instance number
 * that corresponds to one or other of the configured values. These
 * messages are used to (i) update the firmware's notion of the NMEA
 * network address of each configured remote windlass (allowing control
 * messages to be directed to the remote device) and (ii) condition
 * the status output channels associated with a particular channel.
 * Continuous update of remote windlass network addresses is required
 * because the NMEA bus may require nodes to dynamically change their
 * network address.
 *  
 * UP and DOWN GPIO inputs are provided for each control channel: if a
 * control channel becomes active then the firmware will output a
 * PGN 126208 Group Function Control message commanding the associated
 * windlass to operate in the same sense as the control. PGN 126208
 * messages will continue to be output every 250ms until the control
 * channel becomes inactive.
 * 
 * Five GPIO status outputs are provided (conditioned by PGN 128777
 * Windlass Operating Status messages).
 */

#include <Arduino.h>
#include <EEPROM.h>
#include <NMEA2000_CAN.h>
#include <N2kTypes.h>
#include <N2kMessages.h>
#include <Debouncer.h>
#include <LedManager.h>
#include <WindlassState.h>
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
#define DEBUG_W0_INSTANCE_VALUE 0x22
#define DEBUG_W0_ADDRESS_VALUE 0x22
#define DEBUG_W1_INSTANCE_VALUE 127
#define DEBUG_W1_ADDRESS_VALUE 255

/**********************************************************************
 * MCU EEPROM STORAGE DEFINITIONS
 * 
 * These two addresses specify the persistent storage address that
 * should be used to store the 1-byte remote windlass instance numbers. 
 */

#define W0_INSTANCE_EEPROM_ADDRESS 0
#define W1_INSTANCE_EEPROM_ADDRESS 1

/**********************************************************************
 * MCU DIGITAL IO PIN DEFINITIONS
 * 
 * GPIO pin definitions for the Teensy 3.2 MCU
 */

#define GPIO_W1_PRG_SWITCH 0
#define GPIO_W1_LED 1
#define GPIO_W0_LED 2
#define GPIO_POWER_LED 5
#define GPIO_INSTANCE_BIT6 6
#define GPIO_INSTANCE_BIT5 7
#define GPIO_INSTANCE_BIT4 8
#define GPIO_INSTANCE_BIT3 9
#define GPIO_INSTANCE_BIT2 10
#define GPIO_INSTANCE_BIT1 11
#define GPIO_INSTANCE_BIT0 12
#define GPIO_BOARD_LED 13
#define GPIO_W0_UP_SWITCH 14
#define GPIO_W0_DN_SWITCH 15
#define GPIO_W1_UP_SWITCH 16
#define GPIO_W1_DN_SWITCH 17
#define GPIO_W0_UP_RELAY 18
#define GPIO_W0_DN_RELAY 19
#define GPIO_W1_UP_RELAY 20
#define GPIO_W1_DN_RELAY 21
#define GPIO_POWER_RELAY 22
#define GPIO_W0_PRG_SWITCH 23
#define GPIO_INSTANCE_PINS { GPIO_INSTANCE_BIT0, GPIO_INSTANCE_BIT1, GPIO_INSTANCE_BIT2, GPIO_INSTANCE_BIT3, GPIO_INSTANCE_BIT4, GPIO_INSTANCE_BIT5, GPIO_INSTANCE_BIT6 }
#define GPIO_INPUT_PINS { GPIO_W0_PRG_SWITCH, GPIO_W1_PRG_SWITCH, GPIO_W0_UP_SWITCH, GPIO_W0_DN_SWITCH, GPIO_W1_UP_SWITCH, GPIO_W1_DN_SWITCH, GPIO_INSTANCE_BIT0, GPIO_INSTANCE_BIT1, GPIO_INSTANCE_BIT2, GPIO_INSTANCE_BIT3, GPIO_INSTANCE_BIT4, GPIO_INSTANCE_BIT5, GPIO_INSTANCE_BIT6 }
#define GPIO_OUTPUT_PINS { GPIO_BOARD_LED, GPIO_POWER_LED, GPIO_W0_LED, GPIO_W1_LED, GPIO_POWER_RELAY, GPIO_W0_UP_RELAY, GPIO_W0_DN_RELAY, GPIO_W1_UP_RELAY, GPIO_W1_DN_RELAY }

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
#define PRODUCT_TYPE "WINCTL"
#define PRODUCT_VERSION "1.0 (Sep 2020)"

/**********************************************************************
 * Include the build.h header file which can be used to override any or
 * all of the above  constant definitions.
 */

#include "build.h"

/**********************************************************************
 * Miscellaneous
 */

#define INSTANCE_UNDEFINED 255            // Flag value
#define INSTANCE_DISABLED 127             // Flag value
#define STARTUP_SETTLE_PERIOD 5000        // Wait this many ms before processing switch inputs
#define SWITCH_PROCESS_INTERVAL 250       // Process switch inputs evety n ms
#define RELAY_UPDATE_INTERVAL 330         // Update outputs every n ms
#define STATUS_LED_MANAGER_HEARTBEAT 300  // Settings for LEDs on module case
#define STATUS_LED_MANAGER_INTERVAL 10    //
#define STATE_LED_MANAGER_HEARTBEAT 300   // Settings for relay output (LEDs)
#define STATE_LED_MANAGER_INTERVAL 1      //

/**********************************************************************
 * Declarations of local functions.
 */

#ifdef DEBUG_SERIAL
void debugDump();
#endif
unsigned char getPoleInstance();
void messageHandler(const tN2kMsg&);
void PGN128777(const tN2kMsg&);
void processSwitches(WindlassState **windlasses);
void transmitWindlassControl(WindlassState *windlass);
void operateOutputs(WindlassState *windlass);

/**********************************************************************
 * PGNs of messages transmitted by this program.
 * 
 * PGN 126208 Request Group Function is used to transmit windlass
 * operating commands to remote windlasses.
 */

const unsigned long TransmitMessages[] PROGMEM={ 126208L, 0 };

/**********************************************************************
 * PGNs of messages handled by this program.
 * 
 * PGN 128777 Windlass Operating Status is accepted and passed to the
 * function PGN128777 which will use received data to update the status
 * outputs for configured remote windlasses.
 */

typedef struct { unsigned long PGN; void (*Handler)(const tN2kMsg &N2kMsg); } tNMEA2000Handler;
tNMEA2000Handler NMEA2000Handlers[]={ {128777L, &PGN128777}, {0, 0} };

/**********************************************************************
 * Create a switch debouncer DEBOUNCER and associate with it the GPIO
 * pins that are connected to switches.
 */

int SWITCHES[DEBOUNCER_SIZE] = { GPIO_W0_PRG_SWITCH, GPIO_W0_UP_SWITCH, GPIO_W0_DN_SWITCH, GPIO_W1_PRG_SWITCH, GPIO_W1_UP_SWITCH, GPIO_W1_DN_SWITCH, -1, -1 };
Debouncer DEBOUNCER (SWITCHES);

/**********************************************************************
 * Create an LED manager with operating characteristics that suit the
 * status LEDS mounted on the module PCB.
 */

LedManager STATUS_LED_MANAGER (STATUS_LED_MANAGER_HEARTBEAT, STATUS_LED_MANAGER_INTERVAL);

/**********************************************************************
 * And create another LED manager with operating characteristics that
 * suit the module's state output relays.
 */

LedManager STATE_LED_MANAGER (STATE_LED_MANAGER_HEARTBEAT, STATE_LED_MANAGER_INTERVAL);

/**********************************************************************
 * Create state definitions for two windlass control channels and
 * parcel them up in the WINDLASSES array so that the array index is
 * the channel selector.
 */

WindlassState WINDLASS0;
WindlassState WINDLASS1;
WindlassState *WINDLASSES[2] = { &WINDLASS0, &WINDLASS1 };

/**********************************************************************
 * Keep track of whether this is a new boot, because we may want to do
 * some fancy stuff before we start work.
 */



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

  WINDLASS0.instance = EEPROM.read(W0_INSTANCE_EEPROM_ADDRESS);
  WINDLASS0.programmeSwitchGPIO = GPIO_W0_PRG_SWITCH;
  WINDLASS0.upSwitchGPIO = GPIO_W0_UP_SWITCH;
  WINDLASS0.downSwitchGPIO = GPIO_W0_DN_SWITCH;
  WINDLASS0.statusLedGPIO = GPIO_W0_LED;
  WINDLASS0.upLedGPIO = GPIO_W0_UP_RELAY;
  WINDLASS0.downLedGPIO = GPIO_W0_DN_RELAY;
  WINDLASS0.pDebouncer = &DEBOUNCER;
  WINDLASS0.pStatusLedManager = &STATUS_LED_MANAGER;
  WINDLASS0.pStateLedManager = &STATE_LED_MANAGER;

  WINDLASS1.instance = EEPROM.read(W1_INSTANCE_EEPROM_ADDRESS);
  WINDLASS1.programmeSwitchGPIO = GPIO_W1_PRG_SWITCH;
  WINDLASS1.upSwitchGPIO = GPIO_W1_UP_SWITCH;
  WINDLASS1.downSwitchGPIO = GPIO_W1_DN_SWITCH;
  WINDLASS1.statusLedGPIO = GPIO_W1_LED;
  WINDLASS1.upLedGPIO = GPIO_W1_UP_RELAY;
  WINDLASS1.downLedGPIO = GPIO_W1_DN_RELAY;
  WINDLASS1.pDebouncer = &DEBOUNCER;
  WINDLASS1.pStatusLedManager = &STATUS_LED_MANAGER;
  WINDLASS1.pStateLedManager = &STATE_LED_MANAGER;

  #ifdef DEBUG_USE_DEBUG_ADDRESSES
    WINDLASS0.instance = DEBUG_W0_INSTANCE_VALUE;
    WINDLASS0.address = DEBUG_W0_ADDRESS_VALUE;
    WINDLASS1.instance = DEBUG_W1_INSTANCE_VALUE;
    WINDLASS1.address = DEBUG_W1_ADDRESS_VALUE;
  #endif
  
  STATUS_LED_MANAGER.operate(GPIO_BOARD_LED, 0, 3);

  NMEA2000.SetProductInformation(PRODUCT_SERIAL_CODE, PRODUCT_CODE, PRODUCT_TYPE, PRODUCT_FIRMWARE_VERSION, PRODUCT_VERSION);
  NMEA2000.SetDeviceInformation(DEVICE_UNIQUE_NUMBER, DEVICE_FUNCTION, DEVICE_CLASS, DEVICE_MANUFACTURER_CODE);
  NMEA2000.SetMode(tNMEA2000::N2km_ListenAndNode, 22); // Configure for sending and receiving.
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
  if (!JUST_STARTED) processSwitches(WINDLASSES);

  NMEA2000.ParseMessages();
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

void processSwitches(WindlassState **windlasses) {
  static unsigned long deadline = 0UL;
  unsigned long now = millis();
  if (now > deadline) {
    for (unsigned int i = 0; i < ELEMENTCOUNT(windlasses); i++) {
      if (!windlasses[i]->pDebouncer->channelState(windlasses[i]->programmeSwitchGPIO)) {
        windlasses[i]->instance = getPoleInstance();
        EEPROM.update(windlasses[i]->instanceStorageAddress, windlasses[i]->instance);
      }
      if (windlasses[i]->isReady()) {
        if ((!windlasses[i]->pDebouncer->channelState(windlasses[i]->upSwitchGPIO)) ^ (!windlasses[i]->pDebouncer->channelState(windlasses[i]->downSwitchGPIO))) {
          windlasses[i]->pStatusLedManager->operate(windlasses[i]->statusLedGPIO, HIGH);
          transmitWindlassControl(windlasses[i]);
        } else {
          windlasses[i]->pStatusLedManager->operate(windlasses[i]->statusLedGPIO, LOW);
        }
      }
    }  
    deadline = (now + SWITCH_PROCESS_INTERVAL);
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

unsigned char getPoleInstance() {
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
