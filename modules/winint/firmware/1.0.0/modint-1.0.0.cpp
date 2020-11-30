#include <Arduino.h>
#include <EEPROM.h>
#include <NMEA2000_CAN.h>
#include <N2kTypes.h>
#include <N2kMessages.h>
#include <string.h>
#include <Debouncer.h>
#include <LedManager.h>
#include <N2kSpudpole.h>
#include <arraymacros.h>

/**********************************************************************
 * DEBUG & TESTING
 * 
 * Defining DEBUG_SERIAL includes the function debugDump() and arranges
 * for it to be called from loop() every DEBUG_SERIAL_INTERVAL
 * milliseconds after an intial start delay of DEBUG_SERIAL_START_DELAY
 * milliseconds.
 * 
 * Defining DEBUG_USE_DEBUG_ADDRESSES disables normal instance number
 * recovery from hardware and instead forces the use of the instance
 * number defined here.
 */

#define DEBUG_SERIAL
#define DEBUG_USE_DEBUG_ADDRESSES

#define DEBUG_SERIAL_START_DELEAY 4000UL
#define DEBUG_SERIAL_INTERVAL 1000UL
#define DEBUG_INSTANCE_VALUE 0x33

/**********************************************************************
 * MCU EEPROM STORAGE DEFINITIONS
 */

#define EEPROM_OPERATING_TIME_STORAGE_ADDRESS 0

/**********************************************************************
 * MCU DIGITAL IO PIN DEFINITIONS
 * 
 * GPIO pin definitions for the Teensy 3.2 MCU
 */

#define GPIO_LED_BOARD 13
#define GPIO_LED_DOWN 9
#define GPIO_LED_POWER 7
#define GPIO_LED_UP 8
#define GPIO_PINS_INPUT { GPIO_SENSOR_DPD, GPIO_SENSOR_DPG, GPIO_SENSOR_OVL, GPIO_SENSOR_ROT, GPIO_SENSOR_RTD, GPIO_SENSOR_RTG, GPIO_SWITCH_ENABLE, GPIO_SWITCH_INSTANCE0, GPIO_SWITCH_INSTANCE1, GPIO_SWITCH_INSTANCE2, GPIO_SWITCH_INSTANCE3, GPIO_SWITCH_INSTANCE4, GPIO_SWITCH_INSTANCE5, GPIO_SWITCH_INSTANCE6 }
#define GPIO_PINS_INSTANCE { GPIO_SWITCH_INSTANCE0, GPIO_SWITCH_INSTANCE1, GPIO_SWITCH_INSTANCE2, GPIO_SWITCH_INSTANCE3, GPIO_SWITCH_INSTANCE4, GPIO_SWITCH_INSTANCE5, GPIO_SWITCH_INSTANCE6, GPIO_SWITCH_INSTANCE7 }
#define GPIO_PINS_OUTPUT { GPIO_RELAY_DOWN, GPIO_RELAY_UP, GPIO_LED_BOARD, GPIO_LED_DOWN, GPIO_LED_PWR, GPIO_LED_UP }
#define GPIO_RELAY_DOWN 6
#define GPIO_RELAY_UP 5
#define GPIO_SENSOR_DPD 21
#define GPIO_SENSOR_DPG 1
#define GPIO_SENSOR_OVL 2
#define GPIO_SENSOR_ROT 23
#define GPIO_SENSOR_RTD 22
#define GPIO_SENSOR_RTG 0
#define GPIO_SWITCH_ENABLE 12
#define GPIO_SWITCH_INSTANCE0 14
#define GPIO_SWITCH_INSTANCE1 15
#define GPIO_SWITCH_INSTANCE2 16
#define GPIO_SWITCH_INSTANCE3 17
#define GPIO_SWITCH_INSTANCE4 18
#define GPIO_SWITCH_INSTANCE5 19
#define GPIO_SWITCH_INSTANCE6 20

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
#define DEVICE_UNIQUE_NUMBER 565

/**********************************************************************
 * PRODUCT INFORMATION
 * 
 * This poorly structured set of values is what NMEA expects a product
 * description to be shoe-horned into.
 */

#define PRODUCT_CERTIFICATION_LEVEL 1
#define PRODUCT_CODE 001
#define PRODUCT_FIRMWARE_VERSION "1.0.0 (Sep 2020)"
#define PRODUCT_LEN 2
#define PRODUCT_N2K_VERSION 2101
#define PRODUCT_SERIAL_CODE "001-565"
#define PRODUCT_TYPE "WININT"
#define PRODUCT_VERSION "1.0 (Sep 2020)"

/**********************************************************************
 * SPUDPOLE_INFORMATION
 * 
 * These settings define the operating characteristics of the spudpole
 * to which the firmware build will relate. All values in SI units.
 */

#define SPUDPOLE_LINE_DIAMETER 0.01
#define SPUDPOLE_NOMINAL_CONTROLLER_VOLTAGE 24.0
#define SPUDPOLE_NOMINAL_LINE_SPEED 0.3
#define SPUDPOLE_NOMINAL_MOTOR_CURRENT 80.0
#define SPUDPOLE_SPOOL_DIAMETER 0.06
#define SPUDPOLE_TURNS_PER_LAYER 10
#define SPUDPOLE_USABLE_LINE_LENGTH 60.0

/**********************************************************************
 * Include the build.h header file which would normally be generated
 * by the firmware build system. Note that this file may well override
 * some or all of the above constant definitions.
 */

#include "build.h"

/**********************************************************************
 * Miscelaneous
 */

#define INSTANCE_UNDEFINED 255
#define N2K_COMMAND_TIMEOUT 0.4 // seconds
#define N2K_DYNAMIC_UPDATE_INTERVAL 0.25 // seconds
#define N2K_STATIC_UPDATE_INTERVAL 5.0 // seconds
#define STARTUP_SETTLE_PERIOD 5000
#define STATUS_LED_MANAGER_HEARTBEAT 300
#define STATUS_LED_MANAGER_INTERVAL 10
#define TRANSMIT_LED_TIMEOUT 200 // milliseconds

/**********************************************************************
 * Declarations of local functions.
 */

#ifdef DEBUG_SERIAL
void debugDump();
#endif
unsigned char getPoleInstance();
void messageHandler(const tN2kMsg&);
void PGN128776(const tN2kMsg &N2kMsg);
double readTotalOperatingTime();
void transmitStatus();
void commandTimeout(long timeout = 0L);
void setRelayOutput(int action = 0, long timeout = 0L);
void provisionPGN128776(tN2kMsg &msg, byte sed = 0);
void provisionPGN128777(tN2kMsg &msg, byte sed = 0);
void provisionPGN128778(tN2kMsg &msg, byte sed = 0);
double windlassOperatingTimer(Windlass::OperatingTimerMode mode, Windlass::OperatingTimerFunction func);

/**********************************************************************
 * PGNs of messages transmitted by this program.
 * 
 * PGN 128776 Windlass Control Status
 * PGN 128777 Windlass Operating Status
 * PGN 128778 Windlass ????????? Status
 */

const unsigned long TransmitMessages[] PROGMEM = { 128776L, 128777L, 128778L, 0L };

/**********************************************************************
 * PGNs of messages handled by this program.
 * 
 * PGN 126602 Request Group Function contains the windlass operating
 * commands.
 */

typedef struct { unsigned long PGN; void (*Handler)(const tN2kMsg &N2kMsg); } tNMEA2000Handler;
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

int SWITCHES[] = { GPIO_SENSOR_DPD, GPIO_SENSOR_DPG, GPIO_SENSOR_OVL, GPIO_SENSOR_ROT, GPIO_SENSOR_RTD, GPIO_SENSOR_RTG };
Debouncer *DEBOUNCER = new Debouncer(SWITCHES);

/**********************************************************************
 * Create an LED manager STATUS_LED_MANAGER which can be used to manage
 * the status LEDS mounted on the module PCB.
 */

LedManager *STATUS_LED_MANAGER = new LedManager(STATUS_LED_MANAGER_HEARTBEAT, STATUS_LED_MANAGER_INTERVAL);



/**********************************************************************
 * MAIN PROGRAM - setup()
 */

void setup() {
  #ifdef SERIAL_DEBUG
  Serial.begin(9600);
  delay(DEBUG_SERIAL_START_DELAY);
  #endif
  
  int ipins[] = GPIO_PINS_INPUT;
  int opins[] = GPIO_PINS_OUTPUT;
  for (unsigned int i = 0 ; i < ARRAYSIZE(ipins); i++) { pinMode(ipins[i], INPUT_PULLUP); }
  for (unsigned int i = 0 ; i < ARRAYSIZE(opins); i++) { pinMode(opins[i], OUTPUT); digitalWrite(opins[i], LOW); }
  
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
  static bool JUST_STARTED = true;
  if (JUST_STARTED && (millis() > STARTUP_SETTLE_PERIOD)) JUST_STARTED = false;

  DEBOUNCER->debounce();
  if (!JUST_STARTED) processSensors();
  
  commandTimeout();
  transmitStatus();

  NMEA2000.ParseMessages();

  #ifdef DEBUG_SERIAL
  debugDump();
  #endif
} 

/**********************************************************************
 * Processes sensor inputs every SWITCH_PROCESS_INTERVAL milliseconds.
 * Must be called directly from loop(). The defined time interval must
 * be less than the rotation period of the windlass being monitored.
 */

void processSensors() {
  static bool rotationSensorProcessed = false;
  static unsigned long deadline = 0UL;
  unsigned long now = millis();
  if (now > deadline) {  
    switch (!DEBOUNCER->channelState(GPIO_ROTATION_SENSOR)) {
      case true:
        if (!rotationSensorProcessed) {
          spudpole.bumpRotationCount();
          rotationSensorProcessed = true;
        }
        break;
      case false:
        rotationSensorProcessed = false;
        break;
    }
    spudpole.setDockedStatus((!DEBOUNCER->channelState(GPIO_SENSOR_RTD))?Spudpole::YES:Spudpole::NO);
    spudpole.setDeployedStatus((!DEBOUNCER->channelState(GPIO_SENSOR_DPD))?Spudpole::YES:Spudpole::NO);
    spudpole.setOperatingState((!DEBOUNCER->channelState(GPIO_SENSOR_DPG))?Windlass::DEPLOYING:Windlass::STOPPED);
    spudpole.setOperatingState((!DEBOUNCER->channelState(GPIO_SENSOR_RTG))?Windlass::RETRIEVING:Windlass::STOPPED);
    deadline = (now + SWITCH_PROCESS_INTERVAL);
  }
}

/**********************************************************************
 * getPoleInstance() returns the module instance address set by the
 * hardware DIP switches defined in GPIO_INSTANCE (the pin sequence
 * supplied must be lo-bit to hi-bit). If GPIO_INSTANCE is not defined
 * then returns 0.
 */

unsigned char getPoleInstance() {
  unsigned char instance = 0;
  #ifdef GPIO_INSTANCE
  int ipins[GPIO_INSTANCE_PINS]; 
  for (byte i = 0; i < 8; i++) {
    instance = instance + (digitalRead(ipins[i]) << i);
  }
  #endif
  return(instance);
}

/**********************************************************************
 * readTotalOperatingTime() returns the total windlass operating time
 * from EEPROM at address EEPROM_OPERATING_TIME_ADDRESS.
 */

double readTotalOperatingTime() {
  double retval = 0.0;
  #ifdef EEPROMADDR_OPERATING_TIME
  EEPROM.get(EEPROMADDR_OPERATING_TIME, retval);
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
      digitalWrite(GPIO_UP_RELAY, LOW);
      digitalWrite(GPIO_DN_RELAY, LOW);
      N2K_LAST_COMMAND = N2kDD484_Off;
      break;
    case -1: // Set relays for RETRIEVE
      commandTimeout(timeout);
      digitalWrite(GPIO_UP_RELAY, HIGH);
      digitalWrite(GPIO_DN_RELAY, LOW);
      N2K_LAST_COMMAND = N2kDD484_Up;
      break;
    case +1: // Set relays for DEPLOY
      commandTimeout(timeout);
      digitalWrite(GPIO_UP_RELAY, LOW);
      digitalWrite(GPIO_DN_RELAY, HIGH);
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

