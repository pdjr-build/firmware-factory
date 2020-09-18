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

#define DEBUG_SERIAL                    // Write debug output to the USB port
//#define DEBUG_USE_FAKE_INSTANCES        // Force use of the following INSTANCE values
#define DEBUG_FAKE_W0_INSTANCE 0x22
#define DEBUG_FAKE_W1_INSTANCE 0x23

/**********************************************************************
 * MCU EEPROM STORAGE ADDRESSES
 */

#define INSTANCE_UNDEFINED 255
#define INSTANCE_DISABLED 127
#define W0_INSTANCE_EEPROM_ADDRESS 0
#define W1_INSTANCE_EEPROM_ADDRESS 1

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

#define PRODUCT_SERIAL_CODE "200-849" // PRODUCT_CODE + DEVICE_UNIQUE_NUMBER
#define PRODUCT_CODE 200
#define PRODUCT_TYPE "WINCTL"
#define PRODUCT_FIRMWARE_VERSION "1.0.0 (Sep 2020)"
#define PRODUCT_VERSION "1.0 (Sep 2020)"
#define PRODUCT_CERTIFICATION_LEVEL 1
#define PRODUCT_LEN 1
#define PRODUCT_N2K_VERSION 2101

/**********************************************************************
 * Include the build.h header file which would normally be generated
 * by the firmware build system. Note that this file may well override
 * some or all of the above #definitions.
 */

//#include "build.h"

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
 * Declarations for local functions.
 */

unsigned char getPoleInstance();
void messageHandler(const tN2kMsg&);
void PGN128777(const tN2kMsg&);
void processSwitches(WindlassState **windlasses, unsigned int count);
void transmitWindlassControl(WindlassState *windlass);

/**********************************************************************
 * PGNs of messages transmitted by this program.
 * 
 * PGN 126208 Request Group Function is used to transmit windlass
 * operating commands to remote windlasses.
 */

const unsigned long TransmitMessages[] PROGMEM={ 126208L, 0 };

/**********************************************************************
 * PGNs of messages handles by this program.
 * 
 * PGN 128777 Windlass Operating Status is accepted and passed to the
 * function PGN128777 which will use received data to update the status
 * outputs for configured remote windlasses.
 */

typedef struct { unsigned long PGN; void (*Handler)(const tN2kMsg &N2kMsg); } tNMEA2000Handler;
tNMEA2000Handler NMEA2000Handlers[]={ {128777L, &PGN128777}, {0, 0} };

/**********************************************************************
 * Create state definitions for two windlass control channels and
 * parcel them up in the WINDLASSES array so that the array index is
 * the channel selector.
 */

WindlassState* WINDLASSES[2] = {
  new WindlassState(), // This is channel 0
  new WindlassState()  // And this is channel 1
};

/**********************************************************************
 * Create a switch debouncer DEBOUNCER and associate with it the GPIO
 * pins that are connected to switches.
 */

int SWITCHES[] = { GPIO_W0_PRG_SWITCH, GPIO_W0_UP_SWITCH, GPIO_W0_DN_SWITCH, GPIO_W1_PRG_SWITCH, GPIO_W1_UP_SWITCH, GPIO_W1_DN_SWITCH };
Debouncer *DEBOUNCER = new Debouncer(SWITCHES);

/**********************************************************************
 * Create an LED manager STATUS_LED_MANAGER which can be used to manage
 * the status LEDS mounted on the module PCB.
 */

LedManager *STATUS_LED_MANAGER = new LedManager(STATUS_LED_MANAGER_HEARTBEAT, STATUS_LED_MANAGER_INTERVAL);

/**********************************************************************
 * And create another LED manager with operating characteristics that
 * suit the module's state output relays.
 */

LedManager *STATE_LED_MANAGER = new LedManager(STATE_LED_MANAGER_HEARTBEAT, STATE_LED_MANAGER_INTERVAL);

/**********************************************************************
 * Keep track of whether this is a new boot, because we may want to do
 * some fancy stuff before we start work.
 */

bool JUST_STARTED = true;

/**********************************************************************
 * MAIN PROGRAM - setup()
 */

void setup() {
  #ifdef SERIAL_DEBUG
  Serial.begin(9600);
  #endif

  #ifdef DEBUG_USE_FAKE_INSTANCES
    WINDLASSES[0]->instance = DEBUG_FAKE_W0_INSTANCE;
    WINDLASSES[1]->instance = DEBUG_FAKE_W1_INSTANCE;
  #else
    WINDLASSES[0]->instance = EEPROM.read(W0_INSTANCE_EEPROM_ADDRESS);
    WINDLASSES[1]->instance = EEPROM.read(W1_INSTANCE_EEPROM_ADDRESS);
  #endif

  WINDLASSES[0]->programmeSwitchGPIO = GPIO_W0_PRG_SWITCH;
  WINDLASSES[0]->upSwitchGPIO = GPIO_W0_UP_SWITCH;
  WINDLASSES[0]->downSwitchGPIO = GPIO_W0_DN_SWITCH;
  WINDLASSES[0]->statusLedGPIO = GPIO_W0_LED;
  WINDLASSES[0]->upLedGPIO = GPIO_W0_UP_RELAY;
  WINDLASSES[0]->downLedGPIO = GPIO_W0_DN_RELAY;
  WINDLASSES[1]->programmeSwitchGPIO = GPIO_W1_PRG_SWITCH;
  WINDLASSES[1]->upSwitchGPIO = GPIO_W1_UP_SWITCH;
  WINDLASSES[1]->downSwitchGPIO = GPIO_W1_DN_SWITCH;
  WINDLASSES[1]->statusLedGPIO = GPIO_W1_LED;
  WINDLASSES[1]->upLedGPIO = GPIO_W1_UP_RELAY;
  WINDLASSES[1]->downLedGPIO = GPIO_W1_DN_RELAY;

  WINDLASSES[0]->pDebouncer = DEBOUNCER;
  WINDLASSES[1]->pDebouncer = DEBOUNCER;

  WINDLASSES[0]->pStatusLedManager = STATUS_LED_MANAGER;
  WINDLASSES[1]->pStatusLedManager = STATUS_LED_MANAGER;

  WINDLASSES[0]->pStateLedManager = STATE_LED_MANAGER;
  WINDLASSES[1]->pStateLedManager = STATE_LED_MANAGER;

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

  STATUS_LED_MANAGER->operate(GPIO_BOARD_LED, 0, 3);

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
  DEBOUNCER->debounce();
  if (!JUST_STARTED) processSwitches(WINDLASSES, ELEMENTCOUNT(WINDLASSES));
  NMEA2000.ParseMessages();
  STATUS_LED_MANAGER->loop();
  STATE_LED_MANAGER->loop();
}

/**********************************************************************
 * processSwitches() invokes various handlers dependent upon the state
 * of their associated switch input channels.
 */

void processSwitches(WindlassState **windlasses, unsigned int count) {
  static unsigned long deadline = 0L;
  unsigned long now = millis();
  if (now > deadline) {
    for (unsigned int i = 0; i < count; i++) {
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
 * reflect the state of <up> and <down>.
 * 
 * transmitWindlassControl() will not operate if a status transmission
 * has not previously been received from the network node defined by
 * <windlass> since the CAN address of the target windlass will be
 * unknown.
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
      }
    }
  }
}
