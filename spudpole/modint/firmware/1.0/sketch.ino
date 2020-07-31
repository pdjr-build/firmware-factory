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

#include <Spudpole.h>
#include <N2kSpudpole.h>


#include <EEPROM.h>

#define EEPROM_TotalMotorTime 0       // EEPROM address (needs 5 bytes).

//********************************************************************************
// MCU GPIO pin definitions.
//
const byte GPIO_INSTANCE[] = { 13,14,15,16,17,18,19,20 };                       // GPIO pins connected to the instance address switch.
const byte GPIO_POLE_UP = 14;                                           // GPIO pin connected to the pole up sensor.
const byte GPIO_POLE_DOWN = 13;
const byte GPIO_PROXIMITY_SENSOR = 21;
const byte GPIO_RELAY_UP = 22;
const byte GPIO_RELAY_DOWN = 23;

//********************************************************************************
// PRODUCT INFORMATION
//
// This poorly structured set of values is what NMEA expects a manufacturer's
// product description to be shoe-horned into. Any or all of these values can be
// overriden by the Makefile using settings supplied in the file "spudpole.cfg".
//
unsigned short 	        PRODUCT_CODE = 1;		        // Something or other
char         	        PRODUCT_TYPE[] = "MODINT";              // Hardware type
char         	        PRODUCT_VERSION[] = "1.0";              // Hardware version
char           	        PRODUCT_SERIAL_CODE[] = "78";       // Hardware serial number
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
const unsigned long     DEVICE_UNIQUE_NUMBER = 95;              // Magically changed on each build.
const unsigned char     DEVICE_FUNCTION = 130;                  // 130 says PC gateway
const unsigned char     DEVICE_CLASS = 25;                      // 25 says network device
const unsigned int      DEVICE_MANUFACTURER_CODE = 2046;        // 2046 is currently unassigned.
const unsigned char     DEVICE_INDUSTRY_GROUP = 4;              // 4 says Maritime industry.

//********************************************************************************
// SPUDPOLE_INFORMATION
//
// These settings define the operating characteristics of the spudpole to which
// the firmware build will relate.  Any or all of these values can be overriden by
// the Makefile using settings supplied in the file "spudpole.cfg". You will want
// to check / change these before every real build.

double                  SPUDPOLE_SPOOL_DIAMETER = 0.06;
double                  SPUDPOLE_LINE_DIAMETER = 0.01;
unsigned int            SPUDPOLE_SPOOL_WIDTH = 10;
unsigned int            SPUDPOLE_LINE_TURNS_WHEN_DOCKED = 60;
double                  SPUDPOLE_CONTROLLER_VOLTAGE = 24;
double                  SPUDPOLE_MOTOR_CURRENT = 80;

//********************************************************************************
// N2K PGNs of messages transmitted by this program.
//
const unsigned long TransmitMessages[] PROGMEM={ 128776L, 128777L, 128778L, 0L };

//********************************************************************************
// Rates of status message transmission expressed in milliseconds.
//
const unsigned long N2K_DYNAMIC_UPDATE_INTERVAL = 500;
const unsigned long N2K_STATIC_UPDATE_INTERVAL = 5000;

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

N2kSpudpole spudpole(getPoleInstance());

void setup() {
  pinMode(GPIO_RELAY_UP, OUTPUT);
  pinMode(GPIO_RELAY_DOWN, OUTPUT);

  unsigned long tmt = 0.00L; EEPROM.get(EEPROM_TotalMotorTime, tmt);
  spudpole.setControlCallback(setRelayOutput);
  spudpole.configureLineMeasurement(SPUDPOLE_SPOOL_DIAMETER, SPUDPOLE_LINE_DIAMETER, SPUDPOLE_SPOOL_WIDTH, SPUDPOLE_LINE_TURNS_WHEN_DOCKED);
  spudpole.configureRunTimeAccounting(tmt, motorTimer);
    
  if (digitalRead(GPIO_POLE_UP)) spudpole.setDocked();
  if (digitalRead(GPIO_POLE_DOWN)) spudpole.setStopped();
  
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
    spudpole.stop();
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
    SetN2kPGN128777(PGN128777Message, sed, spudpole.getInstance(), spudpole.getWindlassMotionStatus(), spudpole.getRodeTypeStatus(), spudpole.getRodeCounterValue(), spudpole.getWindlassLineSpeed(), spudpole.getAnchorDockingStatus(), spudpole.getWindlassOperatingEvents());   
    NMEA2000.SendMsg(PGN128777Message);

    tN2kMsg PGN128778Message;
    SetN2kPGN128778(PGN128778Message, sed, spudpole.getInstance(), spudpole.getWindlassMonitoringEvents(), spudpole.getControllerVoltage(), spudpole.getMotorCurrent(), spudpole.getMotorRunTime());
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
  
  if (ParseN2kPGN128776(N2kMsg, SID, instance, WindlassDirectionControl, AnchorDockingControl, SpeedControlType, SpeedControl, PowerEnable, MechanicalLock, DeckAndAnchorWash, AnchorLight, CommandTimeout, WindlassControlEvents)) {
    if (instance == spudpole.getInstance()) {
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
void setRelayOutput(SpudpoleControl action) {
  switch (action) {
    case SpudpoleControl_STOP: digitalWrite(GPIO_RELAY_UP, LOW); digitalWrite(GPIO_RELAY_DOWN, LOW); break;
    case SpudpoleControl_DEPLOY: digitalWrite(GPIO_RELAY_UP, LOW); digitalWrite(GPIO_RELAY_DOWN, HIGH); break;
    case SpudpoleControl_RETRIEVE: digitalWrite(GPIO_RELAY_UP, HIGH); digitalWrite(GPIO_RELAY_DOWN, LOW); break;
    default: break;
  }
}

void bumpCounter() { spudpole.bumpCounter(); }
void setDocked() { spudpole.setDocked(); }
void setStopped() { spudpole.setStopped(); }

/**
 * A simple millisecond timer with the option of saving its result to
 * permanent storage.
 */
unsigned long motorTimer(SpudpoleTimer mode, unsigned long motorRunTime) {
  static unsigned long motorTimeMillis = 0L;
  unsigned long retval = 0;
  switch (mode) {
    case SpudpoleTimer_START: // Start timing
      motorTimeMillis = millis();
      break;
    case SpudpoleTimer_STOP: // Stop timing and return elapsed milliseconds
      retval = motorRunTime = (millis() - motorTimeMillis);
      EEPROM.put(EEPROM_TotalMotorTime, retval);
      motorTimeMillis = 0L;
      break;
  }
  return(retval);
}
