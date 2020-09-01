/**********************************************************************
 * This firmware provides a mechanical switch control interface for a
 * maximum of two windlasses using the NMEA 2000 Windlass Network
 * Messages protocol based around PGNs 128776, 128777 and 128778.
 * 
 * UP and DOWN control channels are provided for each windlass and the
 * firmware treats a LOW signal as active. Hardware supporting this
 * code will usually opto-isolate physical switch inputs, so typically
 * the hardware inputs will be inverted from active HIGH.
 * 
 * The method of control is straightforwards: if any control channel
 * becomes active then the firmware will output a PGN126208 Group
 * Function Control message commanding the associated windlass to
 * operate in the same sense as the control. PGN126208 messages will
 * continue to be output every 250ms until the control channel becomes
 * inactive.
 * 
 * The firmware supports five status outputs which are active HIGH.
 */

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

#include "../lib/arraymacros.h"

#define SERIAL_DEBUG

//*********************************************************************
// LOCAL DEFINES - THESE MAY BE OVERRIDDEN BY THE BUILD SYSTEM
//*********************************************************************

#define EEPROMADDR_W0_INSTANCE 0
#define EEPROMADDR_W1_INSTANCE 1

#define GPIO_INSTANCE_PINS (12,11,10,9,8,7,6,5)
#define GPIO_INSTANCE ARGN(7, GPIO_INSTANCE_PINS)
#define GPIO_W0_PRG_SWITCH 22
#define GPIO_W1_PRG_SWITCH 23
#define GPIO_W0_UP_SWITCH 13
#define GPIO_W0_DN_SWITCH 14
#define GPIO_W1_UP_SWITCH 15
#define GPIO_W1_DN_SWITCH 16
#define GPIO_PWR_RELAY 21
#define GPIO_W0_UP_RELAY 20
#define GPIO_W0_DN_RELAY 19
#define GPIO_W1_UP_RELAY 18
#define GPIO_W1_DN_RELAY 17
#define GPIO_POWER_LED 2
#define GPIO_W0_LED 1
#define GPIO_W1_LED 0

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

//*********************************************************************
// END OF LOCAL DEFINES
//*********************************************************************

#include "build.h"

/**********************************************************************
 * Miscellaneous
 */

#define STARTUP_CHECK_CYCLE_COUNT 3
#define STARTUP_CHECK_CYCLE_ON_PERIOD 250 // miliseconds
#define STARTUP_CHECK_CYCLE_OFF_PERIOD 250 // miliseconds
#define POWER_LED_TIMEOUT 200 // milliseconds
#define SWITCH_DEBOUNCE_INTERVAL 5 // milliseconds
#define SWITCH_PROCESS_INTERVAL 250 // milliseconds
#define RELAY_UPDATE_INTERVAL 330 // milliseconds

/**********************************************************************
 * LOCAL TYPES...
 */

/**********************************************************************
 * Output relays can either be on, off or flashing...
 */
enum OUTPUT_STATE_T { OSON, OSOFF, OSFLASH };

/**********************************************************************
 * Convenience structure holding key properties of a controlled
 * windlass:
 * 
 * instance     instance number of the remote windlass device. This
 *              value must be stored in EEPROM before the firmware can
 *              begin to communicate with the remote windlass device.
 * address      CAN address of the remote windlass device. This value
 *              is set automatically as soon as a PGN128777 Windlass
 *              Operating Status message is received from a windlass
 *              with the defined instance number.
 * upRelayGpio  address of the pin to which the UP output relay is
 *              connected.
 * dnRelayGpio  address of the pin to which the DOWN output relay is
 *              connected.
 * upRelayState the state of the UP relay derived from PGN 128777
 *              messages received from the remote windlass device.
 * dnRelayState the state of the DOWN relay derived from PGN 128777
 *              messages received from the remote windlass device.
 */

struct WINDLASS_T {
  unsigned char instance; 
  unsigned char address;
  int upRelayGpio;
  int dnRelayGpio;
  OUTPUT_STATE_T upRelayState;
  OUTPUT_STATE_T dnRelayState;
};

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
  DEBOUNCED_SWITCHES_T(): states(0) {};
};

/**********************************************************************
 * Declarations for local functions.
 */

unsigned char debounce(unsigned char sample);
void debounceSwitches(DEBOUNCED_SWITCHES_T &switches);
void exerciseRelayOutputs(int, unsigned long, unsigned long);
unsigned char getPoleInstance();
void messageHandler(const tN2kMsg&);
void operatePowerLED(unsigned long timeout = 0L);
void PGN128777(const tN2kMsg&);
void processSwitches(DEBOUNCED_SWITCHES_T &switches, WINDLASS_T windlasses[]);
void updateRelayOutput(WINDLASS_T windlasses[]);
void transmitWindlassControl(WINDLASS_T windlass, unsigned char up, unsigned char down);




/**********************************************************************
 * N2K PGNs of messages transmitted by this program.
 */

const unsigned long TransmitMessages[] PROGMEM={ 126208UL, 0UL };

/**********************************************************************
 * Some definitions for incoming message handling.   PGNs which are
 * processed and the functions which process them must be declared here
 * and registered in the NMEA2000Handlers jump vector.
 */
typedef struct { unsigned long PGN; void (*Handler)(const tN2kMsg &N2kMsg); } tNMEA2000Handler;
void PGN128777(const tN2kMsg &N2kMsg);
tNMEA2000Handler NMEA2000Handlers[]={ {128777L, &PGN128777}, {0, 0} };

/**********************************************************************
 * GLOBAL VARIABLES
 */

DEBOUNCED_SWITCHES_T DEBOUNCED_SWITCHES;

WINDLASS_T WINDLASSES[2] = {
  { EEPROM.read(EEPROMADDR_W0_INSTANCE), 0xFF, GPIO_W0_UP_RELAY, GPIO_W0_DN_RELAY, OSOFF, OSOFF },
  { EEPROM.read(EEPROMADDR_W1_INSTANCE), 0xFF, GPIO_W1_UP_RELAY, GPIO_W1_DN_RELAY, OSOFF, OSOFF }
};

/**********************************************************************
 * MAIN PROGRAM - setup()
 */

void setup() {
  #ifdef SERIAL_DEBUG
  Serial.begin(9600);
  #endif

  // Set pin modes...
  int ipins[GPIO_INSTANCE];
  for (unsigned int i = 0 ; i < ELEMENTCOUNT(ipins); i++) { pinMode(ipins[i], INPUT_PULLUP); }
  pinMode(GPIO_W0_PRG_SWITCH, INPUT_PULLUP);
  pinMode(GPIO_W1_PRG_SWITCH, INPUT_PULLUP);
  pinMode(GPIO_W0_UP_SWITCH, INPUT_PULLUP),
  pinMode(GPIO_W0_DN_SWITCH, INPUT_PULLUP),
  pinMode(GPIO_W1_UP_SWITCH, INPUT_PULLUP),
  pinMode(GPIO_W1_DN_SWITCH, INPUT_PULLUP),
  pinMode(GPIO_PWR_RELAY, OUTPUT);
  pinMode(GPIO_W0_UP_RELAY, OUTPUT);
  pinMode(GPIO_W0_DN_RELAY, OUTPUT);
  pinMode(GPIO_W0_UP_RELAY, OUTPUT);
  pinMode(GPIO_W0_DN_RELAY, OUTPUT);


  // Cycle outputs as startup check and leave all LOW
  exerciseRelayOutputs(STARTUP_CHECK_CYCLE_COUNT, STARTUP_CHECK_CYCLE_ON_PERIOD, STARTUP_CHECK_CYCLE_OFF_PERIOD);

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
  debounceSwitches(DEBOUNCED_SWITCHES);
  processSwitches(DEBOUNCED_SWITCHES, WINDLASSES);
  NMEA2000.ParseMessages();
  updateRelayOutput(WINDLASSES);
  operatePowerLED();
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

void processSwitches(DEBOUNCED_SWITCHES_T &switches, WINDLASS_T windlasses[]) {
  static unsigned long deadline = 0L;
  unsigned long now = millis();
  if (now > deadline) {
    if (!switches.state.W0Prog) {
      #ifdef SERIAL_DEBUG
      Serial.write("Processing switch W0 PRG");
      Serial.println(getPoleInstance(), HEX);
      #endif
      EEPROM.update(EEPROMADDR_W0_INSTANCE, (windlasses[0].instance = getPoleInstance()));
    }
    if (!switches.state.W1Prog) {
      EEPROM.update(EEPROMADDR_W1_INSTANCE, (windlasses[1].instance = getPoleInstance()));
    }
    if ((!switches.state.W0Up) || (!switches.state.W0Dn)) {
      digitalWrite(GPIO_W0_LED, HIGH);
      transmitWindlassControl(windlasses[0], switches.state.W0Up, switches.state.W0Dn);
    } else {
      digitalWrite(GPIO_W0_LED, LOW);
    }
    if ((!switches.state.W1Up) || (!switches.state.W1Dn)) {
      digitalWrite(GPIO_W1_LED, HIGH);
      transmitWindlassControl(windlasses[1], switches.state.W1Up, switches.state.W1Dn);
    } else {
      digitalWrite(GPIO_W1_LED, LOW);
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

void transmitWindlassControl(WINDLASS_T windlass, unsigned char up, unsigned char down) {
  // We can't go up and down at the same time...
  if (up ^ down) {
    // And we must have programmed an index and have recovered an address...
    if ((windlass.instance != 0xFF) && (windlass.address != 0xFF)) {
      tN2kMsg N2kMsg;
      N2kMsg.SetPGN(126208UL);
      N2kMsg.Priority = 2;
      N2kMsg.Destination = windlass.address;
      N2kMsg.AddByte(0x01); // Command message
      N2kMsg.Add3ByteInt(128776L); // Windlass Control Status PGN
      N2kMsg.AddByte(0xF8); // Retain existing priority
      N2kMsg.AddByte(0x01); // Just one parameter pair to follow
      N2kMsg.AddByte(0x03); // Parameter 1 - Field 3 is Windlass Direction Control
      N2kMsg.AddByte((up != 0)?0x02:((down != 0)?0x01:0x00));
      NMEA2000.SendMsg(N2kMsg);
    }
  }
}  

/**********************************************************************
 * getPoleInstance() returns the 8-bit instance address set by the
 * hardware DIP switches defined in GPIO_INSTANCE (the pin sequence
 * supplied must be lo-bit to hi-bit). If GPIO_INSTANCE is not defined
 * then returns 0xFF.
 */

unsigned char getPoleInstance() {
  unsigned char instance = 0xFF;
  #ifdef GPIO_INSTANCE
  instance = 0x00;
  int ipins[GPIO_INSTANCE]; 
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

  if (ParseN2kPGN128777(N2kMsg, SID, WindlassIdentifier, RodeCounterValue, WindlassLineSpeed, WindlassMotionStatus, RodeTypeStatus, AnchorDockingStatus, WindlassOperatingEvents)) {
    for (int i = 0; i < 2; i++) {
      if ((WINDLASSES[i].instance != 0xFF) && (WINDLASSES[i].instance == WindlassIdentifier)) {
        // We have a message from a configured device, so occult the LED
        operatePowerLED(POWER_LED_TIMEOUT);
        // Save the CAN address of the sender because we will need this to direct control messages
        WINDLASSES[i].address = N2kMsg.Source;
        // And now set the relay states
        if (AnchorDockingStatus == N2kDD482_FullyDocked) {
          // Set UP relay ON, DN relay off
          WINDLASSES[i].upRelayState = OSON;
          WINDLASSES[i].dnRelayState = OSOFF;
        } else {
          switch (WindlassMotionStatus) {
            case N2kDD480_DeploymentOccurring:
              // Set DN relay FLASHING
              WINDLASSES[i].upRelayState = OSOFF;
              WINDLASSES[i].dnRelayState = OSFLASH;
              break;
            case N2kDD480_RetrievalOccurring:
              WINDLASSES[i].upRelayState = OSFLASH;
              WINDLASSES[i].dnRelayState = OSOFF;
              break;
            default:
              WINDLASSES[i].upRelayState = OSOFF;
              WINDLASSES[i].dnRelayState = OSON;
              break;
          }
        }
      }
    }
  }
}

/**********************************************************************
 * updateRelayOutput() processes the WINDLASSES variable, setting the
 * state of relay outputs dependent on the value of the upRelayState
 * and dnRelayState attributes. The function should be called from the
 * loop() and will operate at the inerval defined in
 * RELAY_UPDATE_INTERVAL.
 */

void updateRelayOutput(WINDLASS_T windlasses[]) {
  #ifdef SERIAL_DEBUG
  Serial.println("updateRelayOutput()...");
  #endif

  static unsigned long deadline = millis();
  unsigned long now = millis();
  if (now > deadline) {
    for (int i = 0; i < 2; i++) {
      switch (windlasses[i].upRelayState) {
        case OSON: digitalWrite(windlasses[i].upRelayGpio, HIGH); break;
        case OSOFF:  digitalWrite(windlasses[i].upRelayGpio, LOW);break;
        case OSFLASH: digitalWrite(windlasses[i].upRelayGpio, (digitalRead(windlasses[i].upRelayGpio) == HIGH)?LOW:HIGH); break;
      }
      switch (windlasses[i].dnRelayState) {
        case OSON: digitalWrite(windlasses[i].dnRelayGpio, HIGH); break;
        case OSOFF:  digitalWrite(windlasses[i].dnRelayGpio, LOW);break;
        case OSFLASH: digitalWrite(windlasses[i].dnRelayGpio, (digitalRead(windlasses[i].dnRelayGpio) == HIGH)?LOW:HIGH); break;
      }
    }
    deadline = (now + RELAY_UPDATE_INTERVAL);
  }
}

/**********************************************************************
 * The transmit LED is illuminated to indicate that the device has
 * power and occults on each NMEA transmit. operatePowerLED()
 * switches the LED off and arranges to switch it on after a short,
 * defined, period.  The function requires the definition of
 * GPIO_POWER_LED and POWER_LED_TIMEOUT.
 * 
 * A call to operatePowerLED() should be made from loop() in order
 * to manage switching the LED.  A call to operatePowerLED(timeout)
 *  will switch the LED off and arrange for it to be turned on after
 * <timeout> milliseconds.
 */

void operatePowerLED(unsigned long timeout) {
  static unsigned long deadline = 0L;
  unsigned long now = millis();
  #ifdef GPIO_POWER_LED
  if (timeout) {
    deadline = (now + timeout);
    digitalWrite(GPIO_POWER_LED, LOW);
  }
  if (now > deadline) {
    digitalWrite(GPIO_POWER_LED, HIGH);
  }
  #endif
}

/**********************************************************************
 * Exercise the relay outputs by turning them on for <onmillis>, off
 * for <offmillis> and repeating the whole thing <cycles> times.  Use
 * to indicate reboot and allow a check of whatever output devices are
 * connected to the relays.
 */

void exerciseRelayOutputs(int cycles, unsigned long onmillis, unsigned long offmillis) {
  for (int i = 0; i < cycles; i++) {
    digitalWrite(GPIO_PWR_RELAY, HIGH);
    digitalWrite(GPIO_W0_UP_RELAY, HIGH);
    digitalWrite(GPIO_W0_DN_RELAY, HIGH);
    digitalWrite(GPIO_W1_UP_RELAY, HIGH);
    digitalWrite(GPIO_W1_DN_RELAY, HIGH);
    delay(onmillis);
    digitalWrite(GPIO_PWR_RELAY, LOW);
    digitalWrite(GPIO_W0_UP_RELAY, LOW);
    digitalWrite(GPIO_W0_DN_RELAY, LOW);
    digitalWrite(GPIO_W1_UP_RELAY, LOW);
    digitalWrite(GPIO_W1_DN_RELAY, LOW);
    delay(offmillis);
  }
}
