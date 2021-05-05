/**********************************************************************
 * thrmod-1.0.0.cpp - THRMOD firmware version 1.0.0.
 * Copyright (c) 2021 Paul Reeve, <preeve@pdjr.eu>
 *
 * This firmware implements both ends of a thruster control combination
 * with the actual operating mode determined by the state of a hardware
 * switch. A practical implementation that makes of a physical user
 * control (like a joystick) will require two modules running this
 * firmware, one running in control mode and connected to the joystick
 * and one running in operate mode and connected to the thruster.
 * 
 * Communication between control and operator is implemented using the
 * NMEA 2000 Thruster Network Messages protocol based on PGNs 128006,
 * 128007 and 128008.
 * 
 * The firmware dynamically supports just two protocol properties:
 * Thruster Direction Control and Total Motor Operating Time. A number
 * of static properties (see below) which describe thruster hardware
 * can be defined at compile time and will be reported by an operator
 * module.
 */

#include <Arduino.h>
#include <EEPROM.h>
#include <NMEA2000_CAN.h>
#include <N2kTypes.h>
#include <N2kMessages.h>
#include <Debouncer.h>
#include <LedManager.h>
#include <DilSwitch.h>
#include <arraymacros.h>
#include "PGN128006.h"
#include "GroupFunctionHandlers.h"

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
#define TOTAL_MOTOR_OPERATING_TIME_EEPROM_ADDRESS 1

/**********************************************************************
 * MCU PIN DEFINITIONS
 * 
 * GPIO pin definitions for the Teensy 3.2 MCU
 */

#define GPIO_SB_COMMAND 0
#define GPIO_PS_COMMAND 1
#define GPIO_POWER_LED 2
#define GPIO_ENCODER_BIT7 5
#define GPIO_ENCODER_BIT6 6
#define GPIO_ENCODER_BIT5 7
#define GPIO_ENCODER_BIT4 8
#define GPIO_ENCODER_BIT3 9
#define GPIO_ENCODER_BIT2 10
#define GPIO_ENCODER_BIT1 11
#define GPIO_ENCODER_BIT0 12
#define GPIO_BOARD_LED 13
#define GPIO_MODE_SEL 14
#define GPIO_COMMON 15
#define GPIO_PS_RLY 16
#define GPIO_SB_RLY 17

#define GPIO_ENCODER_PINS { GPIO_ENCODER_BIT0, GPIO_ENCODER_BIT1, GPIO_ENCODER_BIT2, GPIO_ENCODER_BIT3, GPIO_ENCODER_BIT4, GPIO_ENCODER_BIT5, GPIO_ENCODER_BIT6, GPIO_ENCODER_BIT7 }
#define GPIO_INPUT_PINS { GPIO_SB_COMMAND, GPIO_PS_COMMAND, GPIO_ENCODER_BIT0, GPIO_ENCODER_BIT1, GPIO_ENCODER_BIT2, GPIO_ENCODER_BIT3, GPIO_ENCODER_BIT4, GPIO_ENCODER_BIT5, GPIO_ENCODER_BIT6, GPIO_ENCODER_BIT7, GPIO_MODE_SEL, GPIO_COMMON }
#define GPIO_OUTPUT_PINS { GPIO_POWER_LED, GPIO_BOARD_LED, GPIO_PS_RLY, GPIO_SB_RLY }

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
#define DEVICE_CLASS 75
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
#define PRODUCT_FIRMWARE_VERSION "1.0.0 (May 2021)"
#define PRODUCT_LEN 1
#define PRODUCT_N2K_VERSION 2101
#define PRODUCT_SERIAL_CODE "002-849" // PRODUCT_CODE + DEVICE_UNIQUE_NUMBER
#define PRODUCT_TYPE "THRMOD"
#define PRODUCT_VERSION "1.0 (May 2021)"

/**********************************************************************
 * THRUSTER PROGRAMMED DEFAULTS - these are for "MV Beatrice"
 */
#define THRUSTER_SPEED_CONTROL 100                      // Always 100%
#define THRUSTER_AZIMUTH_CONTROL 0.0                    // WTF should this be?
#define THRUSTER_MOTOR_TYPE 4                           // Hydraulic
#define THRUSTER_MOTOR_CURRENT 0                        // Not applicable
#define THRUSTER_MOTOR_TEMPERATURE 273                  // We don't measure this
#define THRUSTER_MOTOR_POWER_RATING 8000                // 8kW
#define THRUSTER_MAXIMUM_MOTOR_TEMPERATURE_RATING 373.0 // 100C
#define THRUSTER_MAXIMUM_ROTATIONAL_SPEED 2000          // RPM

/**********************************************************************
 * Include the build.h header file which can be used to override any or
 * all of the above  constant definitions.
 */
#include "build.h"

#define DEFAULT_SOURCE_ADDRESS 22         // Seed value for address claim
#define DEFAULT_TOTAL_MOTOR_OPERATING_TIME 0
#define BROADCAST_SOURCE_ADDRESS 255
#define THRUSTER_SOURCE_ADDRESS_TIMEOUT 12000
#define INSTANCE_UNDEFINED 255            // Flag value
#define STARTUP_SETTLE_PERIOD 5000        // Wait this many ms before processing switch inputs
#define SWITCH_PROCESS_INTERVAL 100       // Process switch inputs evety n ms
#define LED_MANAGER_HEARTBEAT 100         // Number of ms on / off
#define LED_MANAGER_INTERVAL 10           // Number of heartbeats between repeats

#define DEFAULT_COMMAND_UPDATE_INTERVAL 250     // N2K spec says every 250ms.
#define DEFAULT_COMMAND_TIMEOUT 0.35            // In seconds.
#define PGN128006_STATIC_UPDATE_INTERVAL 5000   // INTERFACE: every 5s when quiescent.
#define PGN128006_DYNAMIC_UPDATE_INTERVAL 500   // INTERFACE: every 0.5s when operating.
#define PGN128007_STATIC_UPDATE_INTERVAL 0      // INTERFACE: only transmit on request.
#define PGN128007_DYNAMIC_UPDATE_INTERVAL 0     // INTERFACE: only transmit on request.
#define PGN128008_STATIC_UPDATE_INTERVAL 5000   // INTERFACE: every 5s when quiescent.
#define PGN128008_DYNAMIC_UPDATE_INTERVAL 500   // INTERFACE: every 0.5s when operating.

/**********************************************************************
 * Declarations of local functions.
 */
#ifdef DEBUG_SERIAL
#endif
// CONTROL mode functions...
void processSwitchInputs();
void transmitDirectionControl();
void PGN128006Handler(const tN2kMsg &N2kMsg);
void checkConnection();
// OPERATING mode functions....
void transmitStatus();
void transmitPGN128006(unsigned char SID);
void transmitPGN128007(unsigned char SID);
void transmitPGN128008(unsigned char SID);
bool isOperating();
bool checkTimeout(unsigned long timeout);
void updatePGN128006(PGN128006_Field fields[]);
void PGN059904Handler(const tN2kMsg &N2kMsg);
// Generic functions...
void messageHandler(const tN2kMsg&);

/**********************************************************************
 * PGNs of messages transmitted by this program.
 * 
 * PGN 130316 Temperature, Extended Range is used to broadcast sensed
 * temperatures.
 */
const unsigned long TransmitMessages[] PROGMEM = { 126208UL, 128006UL, 128007UL, 128008UL, 0 };

/**********************************************************************
 * PGNs of messages handled by this program.
 * 
 * PGN126208 - In OPERATE mode we receive commands this way.
 * PGN128006 - In CONTROL mode we get the source address of the
 *             thruster we should control by listening.
 */
typedef struct { unsigned long PGN; void (*Handler)(const tN2kMsg &N2kMsg); } tNMEA2000Handler;
tNMEA2000Handler NMEA2000Handlers[] = { { 59904UL, &PGN059904Handler }, { 128006UL, &PGN128006Handler } };

/**********************************************************************
 * DIL_SWITCH switch decoder.
 */
int ENCODER_PINS[] = GPIO_ENCODER_PINS;
DilSwitch DIL_SWITCH (ENCODER_PINS, ELEMENTCOUNT(ENCODER_PINS));

/**********************************************************************
 * DEBOUNCER for the programme switch.
 */
int SWITCHES[DEBOUNCER_SIZE] = { GPIO_PS_COMMAND, GPIO_SB_COMMAND, -1, -1, -1, -1, -1, -1 };
Debouncer DEBOUNCER (SWITCHES);

/**********************************************************************
 * LED_MANAGER for all system LEDs.
 */
LedManager LED_MANAGER (LED_MANAGER_HEARTBEAT, LED_MANAGER_INTERVAL);

unsigned char THRUSTER_SOURCE_ADDRESS = BROADCAST_SOURCE_ADDRESS;
unsigned long THRUSTER_SOURCE_ADDRESS_UPDATE_TIMESTAMP = 0UL;
enum { SWITCH_INTERFACE, RELAY_INTERFACE } OPERATING_MODE = SWITCH_INTERFACE;
unsigned int COMMON_MODE = 0;

unsigned long COMMAND_UPDATE_INTERVAL = DEFAULT_COMMAND_UPDATE_INTERVAL;
unsigned long PGN128006_UPDATE_INTERVAL = PGN128006_STATIC_UPDATE_INTERVAL;
unsigned long PGN128007_UPDATE_INTERVAL = PGN128007_STATIC_UPDATE_INTERVAL;
unsigned long PGN128008_UPDATE_INTERVAL = PGN128008_STATIC_UPDATE_INTERVAL;
unsigned long THRUSTER_START_TIME = 0UL;

// Thruster properties
unsigned char                PGN128006_F02_THRUSTER_IDENTIFIER = 0xFF;
tN2kDD473                    PGN128006_F03_THRUSTER_DIRECTION_CONTROL = N2kDD473_ThrusterToPORT;
tN2kDD002                    PGN128006_F04_POWER_ENABLE = N2kDD002_Unknown;
tN2kDD474                    PGN128006_F05_THRUSTER_RETRACT_CONTROL = N2kDD474_OFF;
unsigned char                PGN128006_F06_SPEED_CONTROL = THRUSTER_SPEED_CONTROL;
tN2kDD475                    PGN128006_F07_THRUSTER_CONTROL_EVENTS;
double                       PGN128006_F08_COMMAND_TIMEOUT = (DEFAULT_COMMAND_TIMEOUT * 1000);
double                       PGN128006_F09_AZIMUTH_CONTROL = THRUSTER_AZIMUTH_CONTROL;
unsigned char                PGN128007_F01_THRUSTER_IDENTIFIER = 0xFF;
tN2kDD487                    PGN128007_F02_THRUSTER_MOTOR_TYPE = N2kDD487_Hydraulic;
uint16_t                     PGN128007_F04_MOTOR_POWER_RATING = THRUSTER_MOTOR_POWER_RATING;
double                       PGN128007_F05_MAXIMUM_MOTOR_TEMPERATURE_RATING = THRUSTER_MAXIMUM_MOTOR_TEMPERATURE_RATING;
uint16_t                     PGN128007_F06_MAXIMUM_ROTATIONAL_SPEED = THRUSTER_MAXIMUM_ROTATIONAL_SPEED;
unsigned char                PGN128008_F02_THRUSTER_IDENTIFIER = 0xFF;
tN2kDD471                    PGN128008_F03_THRUSTER_MOTOR_EVENTS;
unsigned char                PGN128008_F04_MOTOR_CURRENT = THRUSTER_MOTOR_CURRENT;
double                       PGN128008_F05_MOTOR_TEMPERATURE = THRUSTER_MOTOR_TEMPERATURE;
uint16_t                     PGN128008_F06_TOTAL_MOTOR_OPERATING_TIME = EEPROM.read(TOTAL_MOTOR_OPERATING_TIME_EEPROM_ADDRESS);
  
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
    EEPROM.write(TOTAL_MOTOR_OPERATING_TIME_EEPROM_ADDRESS, (uint16_t) DEFAULT_TOTAL_MOTOR_OPERATING_TIME);
  }

  // Get PCB switch settings //////////////////////////////////////////
  //
  DIL_SWITCH.sample();
  PGN128006_F02_THRUSTER_IDENTIFIER = PGN128007_F01_THRUSTER_IDENTIFIER = PGN128008_F02_THRUSTER_IDENTIFIER = DIL_SWITCH.value();
  OPERATING_MODE = digitalRead(GPIO_MODE_SEL)?SWITCH_INTERFACE:RELAY_INTERFACE;
  COMMON_MODE = digitalRead(GPIO_COMMON);

  // Initialise all the LEDs //////////////////////////////////////////
  //
  LED_MANAGER.operate(GPIO_BOARD_LED, 0, 3);
  switch (OPERATING_MODE) {
    case SWITCH_INTERFACE: LED_MANAGER.operate(GPIO_POWER_LED, 0, 10000); break;
    case RELAY_INTERFACE: LED_MANAGER.operate(GPIO_POWER_LED, 1); break;
  }

  // Initialise and start N2K services.
  NMEA2000.SetProductInformation(PRODUCT_SERIAL_CODE, PRODUCT_CODE, PRODUCT_TYPE, PRODUCT_FIRMWARE_VERSION, PRODUCT_VERSION);
  NMEA2000.SetDeviceInformation(DEVICE_UNIQUE_NUMBER, DEVICE_FUNCTION, DEVICE_CLASS, DEVICE_MANUFACTURER_CODE);
  NMEA2000.SetMode(tNMEA2000::N2km_ListenAndNode, EEPROM.read(SOURCE_ADDRESS_EEPROM_ADDRESS)); // Configure for sending and receiving.
  NMEA2000.EnableForward(false); // Disable all msg forwarding to USB (=Serial)
  NMEA2000.ExtendTransmitMessages(TransmitMessages); // Tell library which PGNs we transmit
  NMEA2000.AddGroupFunctionHandler(new GroupFunctionHandlerForPGN128006(&NMEA2000, &updatePGN128006));
  NMEA2000.SetMsgHandler(messageHandler);
  NMEA2000.Open();
}

/**********************************************************************
 * MAIN PROGRAM - loop()
 * 
 * With the exception of NMEA2000.parseMessages() all of the functions
 * called from loop() implement interval timers which ensure that they
 * will mostly return immediately, only performing their substantive
 * tasks at intervals defined by program constants.
 * 
 * The global constant JUST_STARTED is used to delay acting on switch
 * inputs until a newly started system has stabilised and the GPIO
 * inputs have been debounced.
 */ 

void loop() {
  static bool JUST_STARTED = true;
  if (JUST_STARTED && (millis() > STARTUP_SETTLE_PERIOD)) {
    #ifdef DEBUG_SERIAL
    Serial.print("Starting (N2K Source address: "); Serial.print(NMEA2000.GetN2kSource()); Serial.println(")");
    #endif
    JUST_STARTED = false;
  }

  switch (OPERATING_MODE) {
    case SWITCH_INTERFACE:
      // Module is a CONTROL device. We debounce and process switch
      // inputs, perhaps transmitting control messages dependent upon
      // switch state. Finally, we check that the remote thruster is
      // still alive.
      DEBOUNCER.debounce();
      if (!JUST_STARTED) processSwitchInputs();
      checkConnection();
      break;
    case RELAY_INTERFACE: // Module is an OPERATING device
      switch (PGN128006_F03_THRUSTER_DIRECTION_CONTROL) {
        case N2kDD473_OFF:
        case N2kDD473_ThrusterReady:
          digitalWrite(GPIO_PS_RLY, 0);
          digitalWrite(GPIO_SB_RLY, 0);
          break;
        case N2kDD473_ThrusterToPORT:
          digitalWrite(GPIO_PS_RLY, 1);
          digitalWrite(GPIO_SB_RLY, (COMMON_MODE?1:0));
          checkTimeout((unsigned long) PGN128006_F08_COMMAND_TIMEOUT * 1000);
          if (THRUSTER_START_TIME == 0UL) THRUSTER_START_TIME = millis();
          break;
        case N2kDD473_ThrusterToSTARBOARD:
          digitalWrite(GPIO_PS_RLY, (COMMON_MODE?1:0));
          digitalWrite(GPIO_SB_RLY, 1);
          checkTimeout((unsigned long) PGN128006_F08_COMMAND_TIMEOUT * 1000);
          if (THRUSTER_START_TIME == 0UL) THRUSTER_START_TIME = millis();
          break;
      }
      break;
      // If we have stopped receiving operating commands, then stop
      // thrusters and update and save total run time.
      if (checkTimeout(0UL)) {
        PGN128006_F03_THRUSTER_DIRECTION_CONTROL = N2kDD473_OFF;
        PGN128008_F06_TOTAL_MOTOR_OPERATING_TIME += ((millis() - THRUSTER_START_TIME) / 1000);
        EEPROM.write(TOTAL_MOTOR_OPERATING_TIME_EEPROM_ADDRESS, PGN128008_F06_TOTAL_MOTOR_OPERATING_TIME);
      }
      transmitStatus();
    default:
      break;
  }
  
  // Process any received messages.
  NMEA2000.ParseMessages();
  // The above call may have resulted in acquisition of a new source
  // address, so we check if there has been a change and if so save the
  // new address to EEPROM for future re-use.
  if (NMEA2000.ReadResetAddressChanged()) EEPROM.update(SOURCE_ADDRESS_EEPROM_ADDRESS, NMEA2000.GetN2kSource());

  
  // Update the states of connected LEDs
  LED_MANAGER.loop();
}

///////////////////////////////////////////////////////////////////////
// START OF CONTROL MODE FUNCTIONS
///////////////////////////////////////////////////////////////////////

/**********************************************************************
 * processSwitchInputs() should be called directly from loop(). If a
 * switch input is active (i.e. a joystick or whatever is being
 * operated) the function uses a simple elapse timer to repeatedly
 * transmit direction controls to the configured remote thruster at the
 * frequency determined by the control update frequency.
 */
void processSwitchInputs() {
  static unsigned long deadline = 0UL;
  unsigned long now = millis();

  if ((COMMAND_UPDATE_INTERVAL != 0UL) && (now > deadline)) {
    if (!DEBOUNCER.channelState(GPIO_PS_COMMAND)) {
      PGN128006_F03_THRUSTER_DIRECTION_CONTROL = N2kDD473_ThrusterToPORT;
      transmitDirectionControl();
    }
    if (!DEBOUNCER.channelState(GPIO_SB_COMMAND)) {
      PGN128006_F03_THRUSTER_DIRECTION_CONTROL = N2kDD473_ThrusterToSTARBOARD;
      transmitDirectionControl();
    }
    deadline = (now + COMMAND_UPDATE_INTERVAL);
  }
}

/**********************************************************************
 * transmitDirectionControl(direction) transmits a PGN 126208 Command
 * Group Function containing a PGN 128006 Thruster Control Status
 * update to the device identified by THRUSTER_SOURCE. The message is
 * only sent if THRUSTER_SOURCE is set to an address other than the
 * broadcast address and this can only have happened if the module has
 * previously received a PGN128006 status message from the thruster
 * whose id is configured by the module's hardware DIP switch.
 */
void transmitDirectionControl() {
  if (THRUSTER_SOURCE_ADDRESS != BROADCAST_SOURCE_ADDRESS) {
    tN2kMsg N2kMsg;
    N2kMsg.SetPGN(126208UL);                // Command Group Function 
    N2kMsg.Priority = 2;                    // High priority
    N2kMsg.Destination = THRUSTER_SOURCE_ADDRESS;   // Send direct to the configured thruster module
    N2kMsg.AddByte(0x01);                   // This is a command message
    N2kMsg.Add3ByteInt(128006UL);           // Thruster Control Status PGN
    N2kMsg.AddByte(0xF8);                   // Retain existing priority
    N2kMsg.AddByte(0x02);                   // Two parameter pairs follow
    N2kMsg.AddByte(0x02);                   // Parameter 1 - Field 2 (Thruster Identifier)
    N2kMsg.AddByte(PGN128006_F02_THRUSTER_IDENTIFIER);    // 
    N2kMsg.AddByte(0x03);                   // Parameter 2 - Field 3 (Thruster Direction Control)
    N2kMsg.AddByte(PGN128006_F03_THRUSTER_DIRECTION_CONTROL);              //
    NMEA2000.SendMsg(N2kMsg);
    LED_MANAGER.operate(GPIO_POWER_LED, 1, 1);
  }
}

/**********************************************************************
 * PGN128006Handler() processes PGN 128006 (Thruster Control Status)
 * messages looking for a message from the thruster configured by the
 * hardware DIP switch.  When such a message is found, the power LED is
 * set to steady to indicate that the configured thruster has been
 * identified on the network and the N2K source address of the message
 * sender is copied into THRUSTER_SOURCE_ADDRESS thus signalling that
 * thruster control messages can be sent. 
 */
void PGN128006Handler(const tN2kMsg &N2kMsg) {
  unsigned char SID;
  unsigned char ThrusterIdentifier;
  tN2kThrusterDirectionControl ThrusterDirectionControl;
  tN2kGenericStatusPair PowerEnable;
  tN2kThrusterRetraction ThrusterRetractControl;
  unsigned char SpeedControl;
  tN2kThrusterControlEvents ThrusterControlEvents;
  double CommandTimeout;
  double AzimuthControl;

  if (parseN2kPGN128006(N2kMsg, SID, ThrusterIdentifier, ThrusterDirectionControl, PowerEnable, ThrusterRetractControl, SpeedControl, ThrusterControlEvents, CommandTimeout, AzimuthControl)) {
    if (ThrusterIdentifier == PGN128006_F02_THRUSTER_IDENTIFIER) {
      THRUSTER_SOURCE_ADDRESS = (unsigned char) N2kMsg.Source;
      THRUSTER_SOURCE_ADDRESS_UPDATE_TIMESTAMP = millis();
      LED_MANAGER.operate(GPIO_POWER_LED, 1, 0);
    }
  }
}

/**********************************************************************
 * checkConnection() should be called directly from loop(). It tries to
 * make sure that a remote thruster connection is still viable by
 * checking that PGN128006Handler() has received status updates from
 * the remote within the period defined by THRUSTER_SOURCE_ADDRESS_TIMEOUT.
 * If the status updates have dried up then THRUSTER_SOURCE_ADDRESS is
 * reset to the broadcast address and the power LED is returned to
 * continuous flashing to indicate no connection.
 */
void checkConnection() {
  if (millis() > (THRUSTER_SOURCE_ADDRESS_UPDATE_TIMESTAMP + THRUSTER_SOURCE_ADDRESS_TIMEOUT)) {
    if (THRUSTER_SOURCE_ADDRESS != BROADCAST_SOURCE_ADDRESS) {
      THRUSTER_SOURCE_ADDRESS = BROADCAST_SOURCE_ADDRESS;
      LED_MANAGER.operate(GPIO_POWER_LED, 0, 10000);
    }
  }
}

///////////////////////////////////////////////////////////////////////
// END OF CONTROL MODE FUNCTIONS
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
// START OF OPERATOR MODE FUNCTIONS
///////////////////////////////////////////////////////////////////////

/**********************************************************************
 * transmitStatus() should be called directly from loop(). It arranges
 * to transmit PGNs 128007, 128007 and 128008 at the frequencies set by
 * their current, respective, operating intervals and makes sure that
 * each operating interval is adjusted to reflect the current thruster
 * operating state (PGNs are issued more frequently when thrusters are
 * operating). PGNs transmitted at the same time are all assigned the
 * same SID. 
 */
void transmitStatus() {
  unsigned char SID = 0;

  static unsigned long PGN128006Deadline = 0UL;
  static unsigned long PGN128007Deadline = 0UL;
  static unsigned long PGN128008Deadline = 0UL;
  unsigned long now = millis();

  if ((PGN128006_UPDATE_INTERVAL != 0UL) && (now > PGN128006Deadline)) {
    transmitPGN128006(SID);
    PGN128006_UPDATE_INTERVAL = isOperating()?PGN128006_DYNAMIC_UPDATE_INTERVAL:PGN128006_STATIC_UPDATE_INTERVAL;
    PGN128006Deadline = (now + PGN128006_UPDATE_INTERVAL);
  }

  if ((PGN128007_UPDATE_INTERVAL != 0UL) && (now > PGN128007Deadline)) {
    transmitPGN128007(SID);
    PGN128007_UPDATE_INTERVAL = isOperating()?PGN128007_DYNAMIC_UPDATE_INTERVAL:PGN128007_STATIC_UPDATE_INTERVAL;
    PGN128007Deadline = (now + PGN128007_UPDATE_INTERVAL);
  }

  if ((PGN128008_UPDATE_INTERVAL != 0UL) && (now > PGN128008Deadline)) {
    transmitPGN128008(SID);
    PGN128008_UPDATE_INTERVAL = isOperating()?PGN128008_DYNAMIC_UPDATE_INTERVAL:PGN128008_STATIC_UPDATE_INTERVAL;
    PGN128008Deadline = (now + PGN128008_UPDATE_INTERVAL);
  }

  SID++;
}

/**********************************************************************
 * transmitPGN128006() provisions a Thruster Control Status message
 * with values drawn from the current state and transmits it using the
 * supplied SID (sequence id).
 */
void transmitPGN128006(unsigned char SID) {
  tN2kMsg N2kMsg;

  SetN2kPGN128006(
    N2kMsg,
    SID,
    PGN128006_F02_THRUSTER_IDENTIFIER,
    PGN128006_F03_THRUSTER_DIRECTION_CONTROL,
    PGN128006_F04_POWER_ENABLE,
    PGN128006_F05_THRUSTER_RETRACT_CONTROL,
    PGN128006_F06_SPEED_CONTROL,
    PGN128006_F07_THRUSTER_CONTROL_EVENTS,
    PGN128006_F08_COMMAND_TIMEOUT,
    PGN128006_F09_AZIMUTH_CONTROL
  );
  NMEA2000.SendMsg(N2kMsg);
}

/**********************************************************************
 * transmitPGN128007() provisions a Thruster Information message with
 * values drawn from the current state and transmits it (the supplied
 * SID (sequence id) is not specified for this PGN).
 */
void transmitPGN128007(unsigned char SID) {
  tN2kMsg N2kMsg;

  SetN2kPGN128007(
    N2kMsg,
    PGN128007_F01_THRUSTER_IDENTIFIER,
    PGN128007_F02_THRUSTER_MOTOR_TYPE,
    PGN128007_F04_MOTOR_POWER_RATING,
    PGN128007_F05_MAXIMUM_MOTOR_TEMPERATURE_RATING,
    PGN128007_F06_MAXIMUM_ROTATIONAL_SPEED
  );

  NMEA2000.SendMsg(N2kMsg);
}

/**********************************************************************
 * transmitPGN128008() provisions a Thruster Motor Status message with
 * values drawn from the current state and transmits it using the
 * supplied SID (sequence id).
 */
void transmitPGN128008(unsigned char SID) {
  tN2kMsg N2kMsg;

  setN2kPGN128008(
    N2kMsg,
    SID,
    PGN128008_F02_THRUSTER_IDENTIFIER,
    PGN128008_F03_THRUSTER_MOTOR_EVENTS,
    PGN128008_F04_MOTOR_CURRENT,
    PGN128008_F05_MOTOR_TEMPERATURE,
    PGN128008_F06_TOTAL_MOTOR_OPERATING_TIME
  );

  NMEA2000.SendMsg(N2kMsg);
}

/**********************************************************************
 * isOperating() is a convenience function which returns true if the
 * thruster is being commanded to operate and otherwise false.
 */
bool isOperating() {
  return((PGN128006_F03_THRUSTER_DIRECTION_CONTROL == N2kDD473_ThrusterToPORT) ||  (PGN128006_F03_THRUSTER_DIRECTION_CONTROL == N2kDD473_ThrusterToSTARBOARD));
}

/**********************************************************************
 * checkTimeout() manages the automatic stopping of a thruster when
 * operating control commands dry up. It has two call conventions.
 * 
 * checkTimeout(0UL) should be called directly from loop(). It will
 * return true if a previously set timeout period has expired.
 * 
 * checkTimeout(t) can be called to establish a timeout of <t> milli-
 * seconds. The function returns false.
 */
bool checkTimeout(unsigned long timeout) {
  static unsigned long deadline = 0UL;
  unsigned long now = millis();
  bool retval = false;

  if (timeout == 0UL) {
    if ((deadline != 0UL) && (now > deadline)) {
      deadline = 0UL;
      retval = true;
    }
  } else {
    deadline = (now + timeout);
  }
  return(retval);
}

/**********************************************************************
 * updatePGN128006() is used as a callback by handlers in the
 * GroupFunctionHandlerForPGN128006 class. It accepts a sparse array
 * of values for properties in the eponymous PGN and, if a value is
 * marked as modified, it updates the local property with the supplied
 * value iff the ThrusterIdentifier in the update array matches the
 * identifier defined for this module.
 */
void updatePGN128006(PGN128006_Field fields[]) {
  bool canUpdate = false;
  
  for (int i = 0; i < (PGN128006_FieldCount + 1); i++) {
    switch (i) {
      case PGN128006_ThrusterIdentifier_FieldIndex:
        if (fields[i].modified && (PGN128006_F02_THRUSTER_IDENTIFIER == fields[i].F02)) canUpdate = true;
        break;
      case PGN128006_ThrusterDirectionControl_FieldIndex:
        if (canUpdate && fields[i].modified) PGN128006_F03_THRUSTER_DIRECTION_CONTROL = fields[i].F03;
        break;
      case PGN128006_PowerEnable_FieldIndex:
        if (canUpdate && fields[i].modified) PGN128006_F04_POWER_ENABLE = fields[i].F04;
        break;
      case PGN128006_ThrusterRetractControl_FieldIndex:
        if (canUpdate) PGN128006_F05_THRUSTER_RETRACT_CONTROL = fields[i].F05;
        break;
      case PGN128006_SpeedControl_FieldIndex:
        if (canUpdate && fields[i].modified) PGN128006_F06_SPEED_CONTROL = fields[i].F06;
        break;
      case PGN128006_ThrusterControlEvents_FieldIndex:
        if (canUpdate && fields[i].modified) PGN128006_F07_THRUSTER_CONTROL_EVENTS = fields[i].F07;
        break;
      case PGN128006_CommandTimeout_FieldIndex:
        if (canUpdate && fields[i].modified) PGN128006_F08_COMMAND_TIMEOUT = fields[i].F08;
        break;
      case PGN128006_AzimuthControl_FieldIndex:
        if (canUpdate && fields[i].modified) PGN128006_F09_AZIMUTH_CONTROL = fields[i].F09;
        break;
      default:
        break;
    }
  }
}

/**********************************************************************
 * Some MFDs issue PGN 059904 ISO Request PGNs in order to get data
 * from a device, so we might as well pander to them.
 */

void PGN059904Handler(const tN2kMsg &N2kMsg) {
  unsigned long PGN;

  if ((OPERATING_MODE == RELAY_INTERFACE) && ParseN2kPGN059904(N2kMsg, PGN)) {
    switch (PGN) {
      case 128006UL: transmitPGN128006(0); break;
      case 128007UL: transmitPGN128007(0); break;
      case 128008UL: transmitPGN128008(0); break;
      default: break;
    }
  }
}

///////////////////////////////////////////////////////////////////////
// END OF OPERATOR MODE FUNCTIONS
///////////////////////////////////////////////////////////////////////

void messageHandler(const tN2kMsg &N2kMsg) {
  int iHandler;
  for (iHandler=0; NMEA2000Handlers[iHandler].PGN!=0 && !(N2kMsg.PGN==NMEA2000Handlers[iHandler].PGN); iHandler++);
  if (NMEA2000Handlers[iHandler].PGN!=0) {
    NMEA2000Handlers[iHandler].Handler(N2kMsg); 
  }
}

#ifdef DEBUG_SERIAL

#endif
