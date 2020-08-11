#include <Arduino.h>
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

/**********************************************************************
 * Declarations for local functions.
 */

unsigned char getPoleInstance();
double readTotalOperatingTime();
void onRotationSensor();
void onDockedSensor();
void onDeployedSensor();
void onRetrievingSensor();
void onDeployingSensor();
void messageHandler(const tN2kMsg&);
void transmitStatus();
void commandTimeout(long timeout = 0L);
void setRelayOutput(int action = 0, long timeout = 0L);
void provisionPGN128776(tN2kMsg &msg, byte sed = 0);
void provisionPGN128777(tN2kMsg &msg, byte sed = 0);
void provisionPGN128778(tN2kMsg &msg, byte sed = 0);
double windlassOperatingTimer(Windlass::OperatingTimerMode mode, Windlass::OperatingTimerFunction func);
void operateTransmitLED(long timeout = 0L);

// EEPROM storage address (stored data value is a double)
#define EEPROM_OPERATING_TIME_ADDRESS 0

// switch TX LED off after this many milliseconds.
#define TRANSMIT_LED_TIMEOUT 200

/**********************************************************************
 * MCU GPIO pin definitions.
 */

const byte              GPIO_INSTANCE[] = { 13,14,15,16,17,18,19,20 }; // Pins 15-22
const byte              GPIO_RETRIEVE_RELAY = 8;                              // Pin10
const byte              GPIO_DEPLOY_RELAY = 9;                            // Pin 11
const byte              GPIO_ROTATION_SENSOR = 21;                    // Pin 23
const byte              GPIO_DOCKED_SENSOR = 11;                         // Pin 13
const byte              GPIO_DEPLOYED_SENSOR = 12;                        // Pin 14
const byte              GPIO_DEPLOYING_SENSOR = 22;                            // Pin 24
const byte              GPIO_RETRIEVING_SENSOR = 23;                          // Pin 25
const byte              GPIO_TRANSMIT_LED = 10;                        // Pin 12
const byte              GPIO_CAN_TX = 3;                                    // Pin 5
const byte              GPIO_CAN_RX = 4;                                    // Pin 6

/**********************************************************************
 * PRODUCT INFORMATION
 * 
 * This poorly structured set of values is what NMEA expects a product
 * description to be shoe-horned into.
 */

const unsigned char     PRODUCT_CERTIFICATION_LEVEL = 1;  // Or, indeed, this
const unsigned short    PRODUCT_CODE = 1;            // Something or other
const char              PRODUCT_FIRMWARE_VERSION[] = "1.0";   // Firmware version
const unsigned char     PRODUCT_LEN = 3;                        // Power consumption as LEN * 50mA
const unsigned short    PRODUCT_N2K_VERSION = 2101;             // God knows what this means
const char              PRODUCT_SERIAL_CODE[] = "340";       // Hardware serial number
const char              PRODUCT_TYPE[] = "MODINT";              // Hardware type
const char              PRODUCT_VERSION[] = "1.0";              // Hardware version

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

const unsigned char     DEVICE_CLASS = 25;
const unsigned char     DEVICE_FUNCTION = 130;
const unsigned char     DEVICE_INDUSTRY_GROUP = 4;
const unsigned int      DEVICE_MANUFACTURER_CODE = 2046;
const unsigned long     DEVICE_UNIQUE_NUMBER = 490;

/**********************************************************************
 * SPUDPOLE_INFORMATION
 * 
 * These settings define the operating characteristics of the spudpole
 * to which the firmware build will relate. All values in SI units.
 */

const double            SPUDPOLE_LINE_DIAMETER = 0.01;
const double            SPUDPOLE_NOMINAL_CONTROLLER_VOLTAGE = 24.0;
const double            SPUDPOLE_NOMINAL_LINE_SPEED = 0.3;
const double            SPUDPOLE_NOMINAL_MOTOR_CURRENT = 80.0;
const double            SPUDPOLE_SPOOL_DIAMETER = 0.06;
const unsigned char     SPUDPOLE_TURNS_PER_LAYER = 10;
const double            SPUDPOLE_USABLE_LINE_LENGTH = 60.0;

/**********************************************************************
 * N2K SPECIFIC DEFAULTS
 */

const double            N2K_COMMAND_TIMEOUT = 0.4;                  // Seconds
const unsigned long     N2K_DYNAMIC_UPDATE_INTERVAL = 0.25;         // Seconds
const unsigned long     N2K_STATIC_UPDATE_INTERVAL = 5.0;           // Seconds

/**********************************************************************
 * N2K PGNs of messages transmitted by this program.
 */

const unsigned long TransmitMessages[] PROGMEM={ 128776L, 128777L, 128778L, 0L };

//*********************************************************************************
// Some definitions for incoming message handling.   PGNs which are processed and
// the functions which process them must be declared here and registered in the
// NMEA2000Handlers jump vector.
//
typedef struct { unsigned long PGN; void (*Handler)(const tN2kMsg &N2kMsg); } tNMEA2000Handler;
void PGN128776(const tN2kMsg &N2kMsg);
tNMEA2000Handler NMEA2000Handlers[]={ {128776L, &PGN128776}, {0, 0} };

//*************************************************************************
// Spudpole configuration

N2kSpudpole::Settings settings = {
  {
    {
      SPUDPOLE_SPOOL_DIAMETER,
      SPUDPOLE_LINE_DIAMETER,
      SPUDPOLE_TURNS_PER_LAYER,
      SPUDPOLE_USABLE_LINE_LENGTH,
      SPUDPOLE_NOMINAL_LINE_SPEED,
      readTotalOperatingTime(),
      windlassOperatingTimer
    },
    SPUDPOLE_NOMINAL_CONTROLLER_VOLTAGE,
    SPUDPOLE_NOMINAL_MOTOR_CURRENT
  },
  getPoleInstance(),
  N2K_COMMAND_TIMEOUT
};

/**********************************************************************
 * GLOBAL VARIABLES
 *
 * The NMEA2000 global is created automatically to suit selected
 * hardware.  Otherwise there are only two: an instance of N2kSpudpole
 * scratch storage for the last operating command received over N2K. 
 */

N2kSpudpole spudpole(settings);
tN2kDD484 N2K_LAST_COMMAND = N2kDD484_Reserved;

void setup() {
  // Set pin modes...
  pinMode(GPIO_ROTATION_SENSOR, INPUT);
  pinMode(GPIO_DOCKED_SENSOR, INPUT_PULLUP);
  pinMode(GPIO_DEPLOYED_SENSOR, INPUT_PULLUP);
  pinMode(GPIO_DEPLOYING_SENSOR, INPUT_PULLUP);
  pinMode(GPIO_RETRIEVING_SENSOR, INPUT_PULLUP);
  pinMode(GPIO_DEPLOY_RELAY, OUTPUT);
  pinMode(GPIO_RETRIEVE_RELAY, OUTPUT);
  
  // Set default pin states...
  digitalWrite(GPIO_DEPLOY_RELAY, LOW);
  digitalWrite(GPIO_RETRIEVE_RELAY, LOW);

  // Attach interrupts...
  attachInterrupt(digitalPinToInterrupt(GPIO_ROTATION_SENSOR), onRotationSensor, FALLING);
  attachInterrupt(digitalPinToInterrupt(GPIO_DOCKED_SENSOR), onDockedSensor, CHANGE);
  attachInterrupt(digitalPinToInterrupt(GPIO_DEPLOYED_SENSOR), onDeployedSensor, CHANGE);
  attachInterrupt(digitalPinToInterrupt(GPIO_DEPLOYING_SENSOR), onDeployingSensor, CHANGE);
  attachInterrupt(digitalPinToInterrupt(GPIO_RETRIEVING_SENSOR), onRetrievingSensor, CHANGE);


  NMEA2000.SetProductInformation(PRODUCT_SERIAL_CODE, PRODUCT_CODE, PRODUCT_TYPE, PRODUCT_FIRMWARE_VERSION, PRODUCT_VERSION, PRODUCT_LEN, PRODUCT_N2K_VERSION, PRODUCT_CERTIFICATION_LEVEL);
  
  NMEA2000.SetDeviceInformation(DEVICE_UNIQUE_NUMBER, DEVICE_FUNCTION, DEVICE_CLASS, DEVICE_MANUFACTURER_CODE, DEVICE_INDUSTRY_GROUP);

  NMEA2000.SetMode(tNMEA2000::N2km_ListenAndNode, 22); // Configure for sending and receiving.
  NMEA2000.EnableForward(false); // Disable all msg forwarding to USB (=Serial)
  NMEA2000.ExtendTransmitMessages(TransmitMessages); // Tell library which PGNs we transmit
  NMEA2000.SetMsgHandler(messageHandler);
  NMEA2000.Open();                             
}

/**********************************************************************
 * The process loop's work is fairly constrained.
 *
 * commandTimeout() ensures that any active UP or DOWN commands that
 *     have been received over N2K (and will have operated the output
 *     relays) are cancelled if they have timed out.
 * operateTransmitLED() similarly ensures that the TX LED which is
 *     turned on each time a status report is issued gets turned off
 *     a few hundred milliseconds later.
 * transmitStatus() makes sure that the module transmits windlass
 *     status PGNs at intervals appropriate to its current operating
 *     mode.
 * NMEA2000.parseMessages() processes any incoming control messages
 *     that may have arrived since the last turn around the loop.
 */

void loop() {
  commandTimeout();
  transmitStatus();
  operateTransmitLED();
  NMEA2000.ParseMessages();
}

/**********************************************************************
 * getPoleInstance() returns the module instance address set by the
 * hardware DIP switches defined in the GPIO_INSTANCE array.  The pin
 * sequence supplied in the array is lo-bit to hi-bit, left to right.
 */

unsigned char getPoleInstance() {
  unsigned char instance = 0;
  for (byte i = 0; i < 8; i++) {
    instance = instance + (digitalRead(GPIO_INSTANCE[i]) << i);
  }
  return(instance);
}

/**********************************************************************
 * readTotalOperatingTime() returns the total windlass operating time
 * from EEPROM at address EEPROM_OPERATING_TIME_ADDRESS.
 */

double readTotalOperatingTime() {
  double retval = 0.0;
  #ifdef EEPROM_OPERATING_TIME_ADDRESS
  EEPROM.get(EEPROM_OPERATING_TIME_ADDRESS, retval);
  #endif
  return(retval);
}

void messageHandler(const tN2kMsg &N2kMsg) {
  int iHandler;
  for (iHandler=0; NMEA2000Handlers[iHandler].PGN!=0 && !(N2kMsg.PGN==NMEA2000Handlers[iHandler].PGN); iHandler++);
  if (NMEA2000Handlers[iHandler].PGN!=0) {
    NMEA2000Handlers[iHandler].Handler(N2kMsg); 
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
    tN2kMsg m776; provisionPGN128776(m776, sed); NMEA2000.SendMsg(m776);
    tN2kMsg m777; provisionPGN128777(m777, sed); NMEA2000.SendMsg(m777);
    tN2kMsg m778; provisionPGN128778(m778, sed); NMEA2000.SendMsg(m778);
    operateTransmitLED(TRANSMIT_LED_TIMEOUT);
    sed++;
  }
}

void provisionPGN128776(tN2kMsg &msg, byte sed) {
  tN2kWindlassControlEvents events;
  SetN2kPGN128776(
    msg,
    sed,
    spudpole.getN2kSpudpoleSettings().instance,
    N2K_LAST_COMMAND,
    100,                        // Always single speed maximum
    N2kDD488_SingleSpeed,       // These spudpoles are always single speed
    N2kDD002_On,                // Anchor docking control is always enabled
    N2kDD002_Unavailable,       // Power is always enabled
    N2kDD002_Unavailable,       // Mechanical locking is unavailable
    N2kDD002_Unavailable,       // Deck and anchor wash is unavailable
    N2kDD002_Unavailable,       // Anchor light is unavailable
    spudpole.getCommandTimeout(),
    events
  );
}

void provisionPGN128777(tN2kMsg &msg, byte sed) { 
  tN2kWindlassOperatingEvents events;
  events.Event.SystemError = 0;
  events.Event.SensorError = 0;
  events.Event.NoWindlassMotionDetected = (spudpole.getRotationCount() == 0)?1:0;
  events.Event.RetrievalDockingDistanceReached = (spudpole.getRotationCount() < (int) spudpole.getWindlassSettings().turnsPerLayer)?1:0;
  events.Event.EndOfRodeReached = (spudpole.getDeployedLineLength() >= spudpole.getWindlassSettings().usableLineLength)?1:0;
  tN2kWindlassMotionStates motionStates;
  switch (spudpole.getOperatingState()) {
    case Windlass::STOPPED: motionStates = N2kDD480_WindlassStopped; break;
    case Windlass::RETRIEVING: motionStates = N2kDD480_RetrievalOccurring; break;
    case Windlass::DEPLOYING: motionStates = N2kDD480_DeploymentOccurring; break;
    default: motionStates = N2kDD480_Unavailable; break;
  }
  SetN2kPGN128777(
    msg,
    sed,
    spudpole.getN2kSpudpoleSettings().instance,
    spudpole.getDeployedLineLength(),
    spudpole.getLineSpeed(),
    motionStates,
    N2kDD481_RopePresentlyDetected,
    (spudpole.isDocked())?N2kDD482_FullyDocked:N2kDD482_NotDocked,
    events
  );
}

void provisionPGN128778(tN2kMsg &msg, byte sed) { 
  tN2kWindlassMonitoringEvents events;
  SetN2kPGN128778(
    msg,
    sed,
    spudpole.getN2kSpudpoleSettings().instance,
    spudpole.getOperatingTime(),
    spudpole.getControllerVoltage(),
    spudpole.getMotorCurrent(),
    events
  );
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
    if (instance == spudpole.getN2kSpudpoleSettings().instance) {
      switch (WindlassDirectionControl) {
        case N2kDD484_Down:
          setRelayOutput(+1, spudpole.getCommandTimeout());
          break;
        case N2kDD484_Up:
          setRelayOutput(-1, spudpole.getCommandTimeout());
          break;
        case N2kDD484_Off:
          setRelayOutput(0, spudpole.getCommandTimeout());
          break;
        default:
          break;
      }
    }
  }
}

/**********************************************************************
 * setRelayOutput() operates the GPIO pins associated with the module's
 * control relays dependent upon the value of <action>: 0 says stop, 1
 * says deploy, 2 says retrieve.
 * 
 * Setting either relay on or off initialises a cancel timer which will
 * switch relays off after <timeout> milliseconds.
 */

void setRelayOutput(int action, long timeout) {
  switch (action) {
    case 0: // Set relays OFF
      commandTimeout(-1); // cancel any operating timeout
      digitalWrite(GPIO_RETRIEVE_RELAY, LOW);
      digitalWrite(GPIO_DEPLOY_RELAY, LOW);
      N2K_LAST_COMMAND = N2kDD484_Off;
      break;
    case -1: // Set relays for RETRIEVE
      commandTimeout(timeout);
      digitalWrite(GPIO_RETRIEVE_RELAY, HIGH);
      digitalWrite(GPIO_DEPLOY_RELAY, LOW);
      N2K_LAST_COMMAND = N2kDD484_Up;
      break;
    case +1: // Set relays for DEPLOY
      commandTimeout(timeout);
      digitalWrite(GPIO_RETRIEVE_RELAY, LOW);
      digitalWrite(GPIO_DEPLOY_RELAY, HIGH);
      N2K_LAST_COMMAND = N2kDD484_Down;
      break;
    default:
      break;
  }
}

/**********************************************************************
 * commandTimeout() switches relay outputs off after some timout
 * interval has expired.  There are two call options.
 * 
 * commandTimeout() should be called from inside loop().  It checks to
 * see if a previously initialised timeout interval has expired and if
 * so sets relay outputs OFF and cancels the timeout.
 * 
 * commandTimeout(timeout) if timeout is greater than zero initialises
 * a timeout session, if timeout is -1, then cancels any operating
 * timeout.
 */

void commandTimeout(long timeout) {
  static unsigned long _end = 0L;

  switch (timeout) {
    case 0L: // Normal loop tick
      if ((_end > 0L) && (_end < millis())) {
        setRelayOutput(0);
        _end = 0L;
      }
      break;
    case -1L: // Cancel timeout
      _end = 0L;
      break;
    default: // Start new timeout
      _end = (millis() + timeout);
      break;
  }
}


/**********************************************************************
 * Interrup service routines triggered directly from disgital I/O pin
 * state changes
 */

void onDockedSensor() {
  spudpole.setDockedStatus(digitalRead(GPIO_DOCKED_SENSOR)?Spudpole::YES:Spudpole::NO);
}

void onDeployedSensor() {
  spudpole.setDeployedStatus(digitalRead(GPIO_DEPLOYED_SENSOR)?Spudpole::YES:Spudpole::NO);
}

void onDeployingSensor() {
  spudpole.setOperatingState(digitalRead(GPIO_DEPLOYING_SENSOR)?Windlass::DEPLOYING:Windlass::STOPPED);
}

void onRetrievingSensor() {
  spudpole.setOperatingState(digitalRead(GPIO_RETRIEVING_SENSOR)?Windlass::RETRIEVING:Windlass::STOPPED);
}

void onRotationSensor() {
  spudpole.bumpRotationCount();
}

/**********************************************************************
 * windlassOperatingTimer is a callback function for use by the
 * Windlass class in support of operating time accounting. Support is
 * provided for both NORMAL and STORAGE operating modes, but note that
 * STORAGE mode is only supported if the global EEPROM_OPERATING_TIME
 * is defined with a EEPROM storage address. See Windlass.h for more
 * information.
 */
double windlassOperatingTimer(Windlass::OperatingTimerMode mode, Windlass::OperatingTimerFunction func) {
  static unsigned long startMillis = 0L;
  double retval = 0.0;
  switch (func) {
    case Windlass::STOP: // Stop timing and return elapsed seconds
      if (startMillis != 0L) {
        retval = (double) ((millis() - startMillis) * 1000);
        startMillis = 0L;
        #ifdef EEPROM_OPERATING_TIME_ADDRESS
        if (mode == Windlass::STORAGE) EEPROM.put(EEPROM_OPERATING_TIME_ADDRESS, retval);
        #endif
      }
      break;
    case Windlass::START:
      startMillis = millis();
      break;
    default:
      break;
  }
  return(retval);
}

/**********************************************************************
 * operateTransmitLED() switches on an LED and switches it off after a
 * defined period.  The function requires the definition of
 * GPIO_TRANSMIT_LED and GPIO_TRANSMIT_LED_TITMEOUT.
 * 
 * a call to operateTransmitLED() should be made from loop() in order
 * to manage switching the LED off.  A call to operateTransmitLED(timeout)
 *  will switch the LED on and arrange for it to be turned off after
 * <timeout> milliseconds.
 */

void operateTransmitLED(long timeout) {
  #ifdef GPIO_TRANSMIT_LED
  static void _end = 0L;
  switch (timeout) {
    case 0L:
      if ((_end) && (_end < millis())) {
        digitalWrite(GPIO_TRANSMIT_LED, LOW);
        _end = 0L;
      }
      break;
    default:
      digitalWrite(GPIO_TRANSMIT_LED, HIGH);
      _end = (millis() + timeout);
      break;      
  }
  #endif
}
