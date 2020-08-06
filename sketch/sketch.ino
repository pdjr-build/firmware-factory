#include <EEPROM.h>

#include <ActisenseReader.h>
#include <N2kMsg.h>
#include <N2kTypes.h>
#include <N2kMaretron.h>
#include <N2kGroupFunctionDefaultHandlers.h>
#include <NMEA2000_CAN.h>
#include <Seasmart.h>
#include <NMEA2000.h>
#include <NMEA2000_CompilerDefns.h>
#include <N2kGroupFunction.h>
#include <N2kDeviceList.h>
#include <N2kCANMsg.h>
#include <N2kStream.h>
#include <N2kMessagesEnumToStr.h>
#include <N2kMessages.h>
#include <N2kDef.h>

#include <string.h>

#include <NMEA2000_teensy.h>

#include <N2kSpudpole.h>


#include <EEPROM.h>

#define EEPROM_OPERATING_TIME 0       // EEPROM address (needs 5 bytes).

//********************************************************************************
// MCU GPIO pin definitions.
//
const byte GPIO_INSTANCE[] = { 13,14,15,16,17,18,19,20 }; // Pins 15-22
const byte GPIO_POLE_UP = 8;                              // Pin10
const byte GPIO_POLE_DOWN = 9;                            // Pin 11
const byte GPIO_PROXIMITY_SENSOR = 21;                    // Pin 23
const byte GPIO_POLE_DOCKED = 11;                         // Pin 13
const byte GPIO_POLE_STOPPED = 12;                        // Pin 14
const byte GPIO_RELAY_UP = 22;                            // Pin 24
const byte GPIO_RELAY_DOWN = 23;                          // Pin 25
const byte GPIO_TRANSMIT_LED = 10;                        // Pin 12
const byte CAN_TX = 3;                                    // Pin 5
const byte CAN_RX = 4;                                    // Pin 6

//********************************************************************************
// PRODUCT INFORMATION
//
// This poorly structured set of values is what NMEA expects a manufacturer's
// product description to be shoe-horned into. Any or all of these values can be
// overriden by the Makefile using settings supplied in the file "spudpole.cfg".
//
unsigned short 	        PRODUCT_CODE = 1;		        // Something or other
char         	          PRODUCT_TYPE[] = "MODINT";              // Hardware type
char         	          PRODUCT_VERSION[] = "1.0";              // Hardware version
char           	        PRODUCT_SERIAL_CODE[] = "107";       // Hardware serial number
char           	        PRODUCT_FIRMWARE_VERSION[] = "1.0";   // Firmware version
unsigned char  	        PRODUCT_LEN = 3;                        // Power consumption as LEN * 50mA
unsigned short 	        PRODUCT_N2K_VERSION = 2101;             // God knows what this means
unsigned char  	        PRODUCT_CERTIFICATION_LEVEL = 1;	// Or, indeed, this

//********************************************************************************
// DEVICE INFORMATION
//
// Because of NMEA's closed standard, most of this is fiction. Maybe it can be
// made better with more research. In particular, even recent releases of the
// NMEA function and class lists found using Google don't discuss anchor
// systems, so the proper values for DEVICE_CLASS and DEVICE_FUNCTION in this
// application are not known. Unsurprisingly, NMEA haven't assigned an anonymous
// manufacturer code, but 2046 is currently unassigned, so we use that.
// DEVICE_UNIQUE_NUMBER and DEVICE_MANUFACTURER_CODE together must make a unique
// value on any N2K bus and an easy way to achieve this is just to bump the device
// number for every software build (this is done automatically in the Makefile).
// 
const unsigned long     DEVICE_UNIQUE_NUMBER = 142;              // Magically changed on each build.
const unsigned char     DEVICE_FUNCTION = 130;                  // 130 says PC gateway
const unsigned char     DEVICE_CLASS = 25;                      // 25 says network device
const unsigned int      DEVICE_MANUFACTURER_CODE = 2046;        // 2046 is currently unassigned.
const unsigned char     DEVICE_INDUSTRY_GROUP = 4;              // 4 says Maritime industry.

//********************************************************************************
// SPUDPOLE_INFORMATION
//
// These settings define the operating characteristics of the spudpole to which
// the firmware build will relate.  Any or all of these values can be overriden by
// using build system configuration values and this is usually what will be
// required.
//
double                  SPUDPOLE_SPOOL_DIAMETER = 0.06;
double                  SPUDPOLE_LINE_DIAMETER = 0.01;
unsigned char           SPUDPOLE_TURNS_PER_LAYER = 10;
double                  SPUDPOLE_USABLE_LINE_LENGTH = 60.0;
double                  SPUDPOLE_NOMINAL_CONTROLLER_VOLTAGE = 24.0;
double                  SPUDPOLE_NOMINAL_MOTOR_CURRENT = 80.0;
double                  SPUDPOLE_NOMINAL_LINE_SPEED = 0.3;

//********************************************************************************
// N2K SPECIFIC DEFAULTS
//
double                  N2K_COMMAND_TIMEOUT = 0.4;
const unsigned long     N2K_DYNAMIC_UPDATE_INTERVAL = 500;
const unsigned long     N2K_STATIC_UPDATE_INTERVAL = 5000;

//********************************************************************************
// N2K PGNs of messages transmitted by this program.
//
const unsigned long TransmitMessages[] PROGMEM={ 128776L, 128777L, 128778L, 0L };

//********************************************************************************
// The windlass motor will only operate as long as up/down control messages are
// continuously received over N2K. If no such messages are received for the
// interval expressed by WINDLASS_COMMAND_TIMEOUT then the windlass motor will
// stop.
//
static unsigned long WINDLASS_COMMAND_TIMEOUT = 250;
static unsigned long WINDLASS_COMMAND_TIMESTAMP = millis();

//*********************************************************************************
// Some definitions for incoming message handling.   PGNs which are processed and
// the functions which process them must be declared here and registered in the
// NMEA2000Handlers jump vector.
//
typedef struct { unsigned long PGN; void (*Handler)(const tN2kMsg &N2kMsg); } tNMEA2000Handler;
void PGN128776(const tN2kMsg &N2kMsg);
tNMEA2000Handler NMEA2000Handlers[]={ {128776L, &PGN128776}, {0, 0} };

/**
 * Read the module instance address set by the hardware DIP switches.
 */
unsigned char getPoleInstance() {
  unsigned char instance = 0;
  for (byte i = 0; i < 8; i++) {
    instance = instance + (digitalRead(GPIO_INSTANCE[i]) << i);
  }
  return(instance);
}

double readTotalOperatingTime() {
  double retval = 0.0;
  EEPROM.get(EEPROM_OPERATING_TIME, retval);
  return(retval);
}

N2kSpudpoleSettings settings = {
  {
    {
      SPUDPOLE_SPOOL_DIAMETER,
      SPUDPOLE_LINE_DIAMETER,
      SPUDPOLE_TURNS_PER_LAYER,
      SPUDPOLE_USABLE_LINE_LENGTH,
      SPUDPOLE_NOMINAL_LINE_SPEED,
      readTotalOperatingTime(),
      timer
    },
  SPUDPOLE_NOMINAL_CONTROLLER_VOLTAGE,
  SPUDPOLE_NOMINAL_MOTOR_CURRENT
  },
  getPoleInstance(),
  setRelayOutput,
  0,
  N2K_COMMAND_TIMEOUT
};

N2kSpudpole spudpole(settings);

void setup() {

  
  pinMode(GPIO_RELAY_UP, OUTPUT);
  pinMode(GPIO_RELAY_DOWN, OUTPUT);
  
  spudpole.setDockedStatus((digitalRead(GPIO_POLE_UP))?SpudpoleStates_YES:SpudpoleStates_NO);
  spudpole.setFullyDeployedStatus((digitalRead(GPIO_POLE_DOWN))?SpudpoleStates_YES:SpudpoleStates_NO);
  
  attachInterrupt(GPIO_PROXIMITY_SENSOR, bumpCounter, FALLING);
  attachInterrupt(GPIO_POLE_UP, setDocked, FALLING);
  attachInterrupt(GPIO_POLE_DOWN, setStopped, FALLING);
  
  NMEA2000.SetProductInformation(PRODUCT_SERIAL_CODE, PRODUCT_CODE, PRODUCT_TYPE, PRODUCT_FIRMWARE_VERSION, PRODUCT_VERSION, PRODUCT_LEN, PRODUCT_N2K_VERSION, PRODUCT_CERTIFICATION_LEVEL);
  
  NMEA2000.SetDeviceInformation(DEVICE_UNIQUE_NUMBER, DEVICE_FUNCTION, DEVICE_CLASS, DEVICE_MANUFACTURER_CODE, DEVICE_INDUSTRY_GROUP);

  NMEA2000.SetMode(tNMEA2000::N2km_ListenAndNode, 22); // Configure for sending and receiving.
  NMEA2000.EnableForward(false); // Disable all msg forwarding to USB (=Serial)
  NMEA2000.ExtendTransmitMessages(TransmitMessages); // Tell library which PGNs we transmit
  NMEA2000.SetMsgHandler(messageHandler);
  NMEA2000.Open();                             
}

void loop() {
  stopPole();
  transmitStatus();
  NMEA2000.ParseMessages();
}

void messageHandler(const tN2kMsg &N2kMsg) {
  int iHandler;
  for (iHandler=0; NMEA2000Handlers[iHandler].PGN!=0 && !(N2kMsg.PGN==NMEA2000Handlers[iHandler].PGN); iHandler++);
  if (NMEA2000Handlers[iHandler].PGN!=0) {
    NMEA2000Handlers[iHandler].Handler(N2kMsg); 
  }
}


/**
 * stopPole() stops the spudpole movement if no command has been received
 * within the last N2K_OPERATE_INTERVAL.
 */

void stopPole() {
  if ((WINDLASS_COMMAND_TIMESTAMP + WINDLASS_COMMAND_TIMEOUT) < millis()) {
    spudpole.setState(WindlassStates_STOPPED);
  }
}
  
/**
 * transmitStatus() transmits a spudpole status update (consisting of
 * message PGN128777 and PGN128778), but only if a context dependent
 * time interval has elapsed since the previous transmission.
 * 
 * N2K requires that the frequency of transmission of status messages be
 * dependent upon the operating state of the host device: if the device is
 * idle (N2K calls this static), then status messages must issued less
 * frequently than when the device is active (N2K calls this dynamic).
 * The required intervals must be defined in N2K_DYNAMIC_UPDATE_INTERVAL
 * and N2K_STATIC_UPDATE_INTERVAL by values expressed in milliseconds.
 * 
 * This function should be executed directly from loop().
 */
 void transmitStatus() {
  static byte sed = 0;
  static unsigned long lastUpdate = millis();
  unsigned long updateInterval = spudpole.isWorking()?N2K_DYNAMIC_UPDATE_INTERVAL:N2K_STATIC_UPDATE_INTERVAL;
  if ((lastUpdate + updateInterval) < millis()) {
    lastUpdate = millis();

    //tN2kMsg PGN128776Message;
    //SetN2kPGN128776(PGN128776Message, sed, spudpole.getInstance(), 
  
    tN2kMsg PGN128777Message;
    spudpole.getPGN128777(PGN128777Message);   
    NMEA2000.SendMsg(PGN128777Message);

    tN2kMsg PGN128778Message;
    spudpole.getPGN128778(PGN128778Message);
    NMEA2000.SendMsg(PGN128778Message);

    sed++;
  }
}

/**
 * PGN128776 is a message handler for messages of the eponymous type whose
 * only substantive function is to turn the spudpole motor on and off in
 * response to values in the WindlassDirectionControl field.  All other
 * command fields and options are ignored.
 */
void PGN128776(const tN2kMsg &N2kMsg) {  
  unsigned char SID;
  unsigned char instance;
  tN2kWindlassDirectionControl WindlassDirectionControl;
  tN2kGenericStatusPair AnchorDockingControl;
  tN2kSpeedType SpeedControlType;
  unsigned char SpeedControl;
  tN2kGenericStatusPair PowerEnable;
  tN2kGenericStatusPair MechanicalLock;
  tN2kGenericStatusPair DeckAndAnchorWash;
  tN2kGenericStatusPair AnchorLight;
  double CommandTimeout;
  tN2kWindlassControlEvents WindlassControlEvents;
  
  if (ParseN2kPGN128776(N2kMsg, SID, instance, WindlassDirectionControl, SpeedControl, SpeedControlType, AnchorDockingControl, PowerEnable, MechanicalLock, DeckAndAnchorWash, AnchorLight, CommandTimeout, WindlassControlEvents)) {
    if (instance == spudpole.getSettings().instance) {
      switch (WindlassDirectionControl) {
        case N2kDD484_Down:
          WINDLASS_COMMAND_TIMESTAMP = millis();
          spudpole.deploy();
          break;
        case N2kDD484_Up:
          WINDLASS_COMMAND_TIMESTAMP = millis();
          spudpole.retrieve();
          break;
        default:
          break;
      }
    }
  }
}

/**
 * setRelayOutput() operates the GPIO pins associated with the module's
 * control relays dependent upon the value of <action>: 0 says stop, 1
 * says deploy, 2 says retrieve.
 */
void setRelayOutput(int action) {
  switch (action) {
    case 0: digitalWrite(GPIO_RELAY_UP, LOW); digitalWrite(GPIO_RELAY_DOWN, LOW); break;
    case -1: digitalWrite(GPIO_RELAY_UP, LOW); digitalWrite(GPIO_RELAY_DOWN, HIGH); break;
    case +1: digitalWrite(GPIO_RELAY_UP, HIGH); digitalWrite(GPIO_RELAY_DOWN, LOW); break;
    default: break;
  }
}

void bumpCounter() { spudpole.bumpRotationCount(); }
void setDocked() { spudpole.setDockedStatus(SpudpoleStates_YES); }
void setStopped() { spudpole.setStopped(); }

/**
 * A simple millisecond timer with the option of saving its result to
 * permanent storage.
 */
unsigned long timer(int mode, unsigned long runTime) {
  static unsigned long runTimeMillis = 0L;
  unsigned long retval = 0;
  switch (mode) {
    case 0: // Stop timing and return elapsed milliseconds
      retval = runTime + (millis() - runTimeMillis);
      EEPROM.put(EEPROM_TotalMotorTime, retval);
      motorTimeMillis = 0L;
      break;
    default:
      motorTimeMillis = millis();
      break;
  }
  return(retval);
}
