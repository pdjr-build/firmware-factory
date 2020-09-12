/**********************************************************************
 * modctl-1.0.0.cpp - MODCTL firmware version 1.0.0.
 * Copyright (c) 2020 Paul Reeve, <preeve@pdjr.eu>
 *
 * This firmware provides a switch control interface for a maximum of
 * two windlasses using the NMEA 2000 Windlass Network Messages
 * protocol based around PGNs 128776, 128777 and 128778.
 * 
 * Remote windlasses are associated by their instance number which is
 * stored in EEPROM.
 * 
 * UP and DOWN control channels are provided for each windlass: if a
 * control channel becomes active then the firmware will output a
 * PGN 126208 Group Function Control message commanding the associated
 * windlass to operate in the same sense as the control. PGN 126208
 * messages will continue to be output every 250ms until the control
 * channel becomes inactive.
 * 
 * Five status outputs are provided and conditioned by PGN 128777
 * Windlass Operating Status messages received from the assocaited
 * windlasses.
 */

#include <Arduino.h>
#include <EEPROM.h>
#include <N2kTypes.h>
#include <N2kMessages.h>
#include <NMEA2000_teensy.h>
#include <LedManager.h>
#include "arraymacros.h"
#include "WindlassState.h"

#define SERIAL_DEBUG  // Write debug output to the USB port

/**********************************************************************
 * MCU EEPROM STORAGE ADDRESSES
 */

#define EEPROMADDR_W0_INSTANCE 0
#define EEPROMADDR_W1_INSTANCE 1

/**********************************************************************
 * MCU DIGITAL IO PIN ALLOCATION
 * 
 * GPIO pin definitions for the Teensy 3.2 MCU
 */

#define GPIO_INSTANCE_PINS { 12,11,10,9,8,7,6 }
#define GPIO_W0_PRG_SWITCH 23
#define GPIO_W1_PRG_SWITCH 0
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
#define GPIO_POWER_LED 5
#define GPIO_W0_LED 2
#define GPIO_W1_LED 1

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
 * bump the device number for every software build and this is done
 * automatically by the build system.
 */

#define DEVICE_CLASS 25
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
#define PRODUCT_CODE 2
#define PRODUCT_FIRMWARE_VERSION "1.0"
#define PRODUCT_LEN 3
#define PRODUCT_N2K_VERSION 2101
#define PRODUCT_SERIAL_CODE "MODCTL-1.0"
#define PRODUCT_TYPE "MODCTL"
#define PRODUCT_VERSION "1.0"

/**********************************************************************
 * Include the build.h header file which would normally be generated
 * by the firmware build system. Note that this file may well override
 * some or all of the above #definitions.
 */

#include "build.h"

/**********************************************************************
 * Miscellaneous
 */

#define STARTUP_SETTLE_PERIOD 5000
#define STARTUP_CHECK_CYCLE_COUNT 3
#define STARTUP_CHECK_CYCLE_ON_PERIOD 250 // miliseconds
#define STARTUP_CHECK_CYCLE_OFF_PERIOD 250 // miliseconds
#define POWER_LED_TIMEOUT 200 // milliseconds
#define SWITCH_DEBOUNCE_INTERVAL 5 // milliseconds
#define SWITCH_PROCESS_INTERVAL 250 // milliseconds
#define RELAY_UPDATE_INTERVAL 330 // milliseconds
#define CONFIGURATION_CHECK_INTERVAL 5000
#define WINDLASS_INSTANCE_DISABLE_ADDRESS 127
#define STATUS_LED_MANAGER_HEARTBEAT 300
#define STATUS_LED_MANAGER_INTERVAL 10
#define STATE_LED_MANAGER_HEARTBEAT 300
#define STATE_LED_MANAGER_INTERVAL 1

/**********************************************************************
 * Output relays can either be on, off or flashing...
 */

enum OUTPUT_STATE_T { OSON, OSOFF, OSFLASH };


/**********************************************************************
 * Convenience structure used to provide the debounced state of all the
 * input pins.
 */

union DEBOUNCED_SWITCHES_T {
  unsigned char states;
  struct {
    unsigned char W0Prog:1;
    unsigned char W1Prog:1;
    unsigned char W0Up:1;
    unsigned char W0Dn:1;
    unsigned char W1Up:1;
    unsigned char W1Dn:1;
  } state;
  DEBOUNCED_SWITCHES_T(): states(0XFF) {};
};

/**********************************************************************
 * Declarations for local functions.
 */

unsigned char debounce(unsigned char sample);
void debounceSwitches(DEBOUNCED_SWITCHES_T &switches);
unsigned char getPoleInstance();
void messageHandler(const tN2kMsg&);
void PGN128777(const tN2kMsg&);
void processSwitches(DEBOUNCED_SWITCHES_T &switches, WindlassState *w0, WindlassState *w1);
void transmitWindlassControl(WindlassState *windlass, unsigned char up, unsigned char down);

/**********************************************************************
 * N2K PGNs of messages transmitted by this program.
 */

const unsigned long TransmitMessages[] PROGMEM={ 126208UL, 0UL };

/**********************************************************************
 * Some definitions for incoming message handling. PGNs which are
 * processed and the functions which process them must be declared here
 * and registered in the NMEA2000Handlers jump vector.
 */

typedef struct { unsigned long PGN; void (*Handler)(const tN2kMsg &N2kMsg); } tNMEA2000Handler;
void PGN128777(const tN2kMsg &N2kMsg);
tNMEA2000Handler NMEA2000Handlers[]={ {128777L, &PGN128777}, {0, 0} };

/**********************************************************************
 * GLOBAL VARIABLES
 */

tNMEA2000_teensy NMEA2000;

// LED manager with long interval for status leds.
LedManager *statusLedManager = new LedManager(STATUS_LED_MANAGER_HEARTBEAT, STATUS_LED_MANAGER_INTERVAL);
// LED manager with short interval for state output relays (leds).
LedManager *stateLedManager = new LedManager(STATE_LED_MANAGER_HEARTBEAT, STATE_LED_MANAGER_INTERVAL);


WindlassState *Windlass0 = new WindlassState(EEPROM.read(EEPROMADDR_W0_INSTANCE), GPIO_W0_LED, GPIO_W0_UP_RELAY, GPIO_W0_DN_RELAY);
WindlassState *Windlass1 = new WindlassState(EEPROM.read(EEPROMADDR_W1_INSTANCE), GPIO_W1_LED, GPIO_W1_UP_RELAY, GPIO_W1_DN_RELAY);

DEBOUNCED_SWITCHES_T DEBOUNCED_SWITCHES;

bool JUST_STARTED = true;

/**********************************************************************
 * MAIN PROGRAM - setup()
 */

void setup() {
  #ifdef SERIAL_DEBUG
  Serial.begin(9600);
  #endif

  //EEPROM.update(EEPROMADDR_W0_INSTANCE,0x01);
  Windlass0->setAddress(0xEF);
  // Set pin modes...
  int ipins[] = GPIO_INSTANCE_PINS;
  for (unsigned int i = 0 ; i < ELEMENTCOUNT(ipins); i++) { pinMode(ipins[i], INPUT_PULLUP); }
  pinMode(GPIO_BOARD_LED, OUTPUT);
  pinMode(GPIO_POWER_LED, OUTPUT);
  pinMode(GPIO_POWER_RELAY, OUTPUT);

  pinMode(GPIO_W0_DN_RELAY, OUTPUT);
  pinMode(GPIO_W0_DN_SWITCH, INPUT_PULLUP);
  pinMode(GPIO_W0_LED, OUTPUT);
  pinMode(GPIO_W0_PRG_SWITCH, INPUT_PULLUP);
  pinMode(GPIO_W0_UP_RELAY, OUTPUT);
  pinMode(GPIO_W0_UP_SWITCH, INPUT_PULLUP);

  pinMode(GPIO_W1_DN_RELAY, OUTPUT);
  pinMode(GPIO_W1_DN_SWITCH, INPUT_PULLUP);
  pinMode(GPIO_W1_LED, OUTPUT);
  pinMode(GPIO_W1_PRG_SWITCH, INPUT_PULLUP);
  pinMode(GPIO_W1_UP_RELAY, OUTPUT);
  pinMode(GPIO_W1_UP_SWITCH, INPUT_PULLUP);

  Windlass0->setLedManagers(statusLedManager, stateLedManager);
  Windlass1->setLedManagers(statusLedManager, stateLedManager);

  statusLedManager->operate(GPIO_BOARD_LED, 0, 3);
  
  NMEA2000.SetProductInformation(PRODUCT_SERIAL_CODE, PRODUCT_CODE, PRODUCT_TYPE, PRODUCT_FIRMWARE_VERSION, PRODUCT_VERSION, PRODUCT_LEN, PRODUCT_N2K_VERSION, PRODUCT_CERTIFICATION_LEVEL);
  NMEA2000.SetDeviceInformation(DEVICE_UNIQUE_NUMBER, DEVICE_FUNCTION, DEVICE_CLASS, DEVICE_MANUFACTURER_CODE, DEVICE_INDUSTRY_GROUP);

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
 * tasks when their globally defined interval timers expire.
 * 
 * debounceSwitches() reads the MCU switch input pins and debounces
 * them. The interval defined in SWITCH_DEBOUNCE_INTERVAL needs to
 * ensure a fairly high sampling rate for the debounce to be effective
 * and a value around 5ms is recommended.
 * 
 * processSwitches() takes the debounced switch inputs and if an input
 * is active generates an appropriate N2K message to signal the switch
 * control. The windlass control protocol requires that control
 * messages are issued every 250ms and SWITCH_PROCESS_INTERVAL is set
 * to this value by default.  Typically windlass equipment will stop
 * if it does not receive a control message at least every half-second
 * or thereabouts (the standard defines a maximum of 1.2s), so setting
 * the process interval much higher could lead to control stuttering.
 * 
 * NMEA2000.parseMessages() arranges for any incoming N2K messages to
 * be processed into putative output state changes.
 * 
 * updateRelayOutput() updates the output channels using data received
 * by NMEA.parseMessages(). The frequency of update is set by
 * RELAY_UPDATE_INTERVAL which defaults to 330ms.
 * 
 * operatePowerLED() sets the state of the power LED which is occulted
 * each time a PGN128777 Windlass Operating Status message is received
 * from an associated windlass.
 */

void loop() {
  if (JUST_STARTED && (millis() > STARTUP_SETTLE_PERIOD)) JUST_STARTED = false;
  debounceSwitches(DEBOUNCED_SWITCHES);
  if (!JUST_STARTED) processSwitches(DEBOUNCED_SWITCHES, Windlass0, Windlass1);
  //NMEA2000.ParseMessages();
  statusLedManager->loop();
  stateLedManager->loop();
}

/**********************************************************************
 * debounceSwitches() reads the current state of pins connected to
 * physical switches into <switches> and the debounces the recovered
 * values using debounce().
 */

void debounceSwitches(DEBOUNCED_SWITCHES_T &switches) {
  static unsigned long deadline = 0L;
  unsigned long now = millis();
  if (now > deadline) {
    switches.state.W0Prog = digitalRead(GPIO_W0_PRG_SWITCH);
    switches.state.W1Prog = digitalRead(GPIO_W1_PRG_SWITCH);
    switches.state.W0Up = digitalRead(GPIO_W0_UP_SWITCH);
    switches.state.W0Dn = digitalRead(GPIO_W0_DN_SWITCH);
    switches.state.W1Up = digitalRead(GPIO_W1_UP_SWITCH);
    switches.state.W1Dn = digitalRead(GPIO_W1_DN_SWITCH);
    switches.states = debounce(switches.states);
    deadline = (now + SWITCH_DEBOUNCE_INTERVAL);
  }
}

unsigned char debounce(unsigned char sample) {
  static unsigned char state, cnt0, cnt1;
  unsigned char delta;

  delta = sample ^ state;
  cnt1 = (cnt1 ^ cnt0) & (delta & sample);
  cnt0 = ~cnt0 & (delta & sample);
  state ^= (delta & ~(cnt0 | cnt1));
  return state;
}

/**********************************************************************
 * processSwitches() invokes various handlers dependent upon the state
 * of their associated switch input channels.
 */

void processSwitches(DEBOUNCED_SWITCHES_T &switches, WindlassState *w0, WindlassState *w1) {
  static unsigned long deadline = 0L;
  unsigned long now = millis();
  unsigned char instance;
  if (now > deadline) {
    if (!switches.state.W0Prog) {
      instance = getPoleInstance();
      w0->setInstance(instance);
      EEPROM.update(EEPROMADDR_W0_INSTANCE, instance);
    }
    if (!switches.state.W1Prog) {
      instance = getPoleInstance();
      w1->setInstance(instance);
      EEPROM.update(EEPROMADDR_W1_INSTANCE, instance);
    }
    if (w0->isReady()) {
      if ((!switches.state.W0Up) ^ (!switches.state.W0Dn)) {
        statusLedManager->operate(GPIO_W0_LED, HIGH);
        transmitWindlassControl(w0, switches.state.W0Up, switches.state.W0Dn);
      } else {
        statusLedManager->operate(GPIO_W0_LED, LOW);
      }
    }
    if (w1->isReady()) {
      if ((!switches.state.W1Up) ^ (!switches.state.W1Dn)) {
        statusLedManager->operate(GPIO_W1_LED, HIGH);
        transmitWindlassControl(w1, switches.state.W1Up, switches.state.W1Dn);
      } else {
        statusLedManager->operate(GPIO_W1_LED, LOW);
      }
    }   
    deadline = (now + SWITCH_PROCESS_INTERVAL);
  }
}

/**********************************************************************
 * transmitWindlassControl() sends a Group Function control message for
 * PGN128776 Windlass Control Status to the device identified by
 * <windlass>, setting the Windlass Direction Control property to
 * reflect the state of <up> and <down>.
 * 
 * transmitWindlassControl() will not operate if a status transmission
 * has not previously been received from the network node defined by
 * <windlass> since the CAN address of the target windlass will be
 * unknown.
 */

void transmitWindlassControl(WindlassState *windlass, unsigned char up, unsigned char down) {
  // FIX FOR TESTING
  // windlass.address = 0x20;
  // FIX FOR TESTING
  tN2kMsg N2kMsg;
  N2kMsg.SetPGN(126208UL);
  N2kMsg.Priority = 2;
  N2kMsg.Destination = windlass->getAddress();
  N2kMsg.AddByte(0x01); // Command message
  N2kMsg.Add3ByteInt(128776UL); // Windlass Control Status PGN
  N2kMsg.AddByte(0xF8); // Retain existing priority
  N2kMsg.AddByte(0x01); // Just one parameter pair to follow
  N2kMsg.AddByte(0x03); // Parameter 1 - Field 3 is Windlass Direction Control
  N2kMsg.AddByte((up != 0)?0x02:((down != 0)?0x01:0x00));
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
    if ((!Windlass0->isDisabled()) && (Windlass0->getInstance() == WindlassIdentifier)) {
      windlass = Windlass0;
    } else {
      if ((!Windlass1->isDisabled()) && (Windlass1->getInstance() == WindlassIdentifier)) {
        windlass = Windlass1;
      }
    }
    if (windlass != NULL) {
      if (!windlass->isConfigured()) windlass->setAddress(N2kMsg.Source);
      if (windlass->isReady()) {
        // And now set the relay states
        if (AnchorDockingStatus == N2kDD482_FullyDocked) {
          windlass->setState(WindlassState::DOCKED);
        } else {
          switch (WindlassMotionStatus) {
            case N2kDD480_DeploymentOccurring:
              windlass->setState(WindlassState::DEPLOYING);
              break;
            case N2kDD480_RetrievalOccurring:
              windlass->setState(WindlassState::RETRIEVING);
              break;
            default:
              windlass->setState(WindlassState::DEPLOYED);
              break;
          }
        }
      }
    }
  }
}