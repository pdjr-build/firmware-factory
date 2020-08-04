# ankreo-spudpole-n2k

Abstract data type modelling an NMEA 2000 enabled spudpole from the
manufacturer Ankreo.

The __N2kSpudpole__ class specialises the __Spudpole__ class defined in
(ankreo-spudpole)[https://github.com/preeve9534/ankreo-spudpole]
with properties and behaviours supporting use in an NMEA 2000 (N2K) context.

These enhancements include:
* addition of an NMEA instance identifier property.
* addition of reporting methods which directly support the N2K Anchor Windlass
  Interface (AWI) defined in. 
* support for a failsafe, timeout based, control interface of the type implied
  by AWI.

## Creating and configuring an N2kSpudpole instance

An __N2kSpudpole__ instance is created in the usual C++ way by simply declaring
a variable of the N2kSpudpole type and supplying some arguments which
characterise the new object.
```
#define instance 3
#define manufacturerName "Ankreo"
#define modelCode "Type 32"
#define serialCode" "0001"

N2kSpudpole mySpudpole(instance, manufacturerName, modelCode, serialCode);
```
_instance_ is a required, unique, identifier which discriminates each anchor
windlass on a vessel's N2K bus.

### Primitive methods

_getInstance()_ returns the instance value that was supplied when __N2kSpudpole__
was instantiated.
```
unsigned char instance = mySpudpole.getInstance();
```

_setControllerVoltage()_ sets the voltage in Volts at which the spudpole
system is operating.  This could be set initially as a nominal value, or
periodically in real time if continuous voltage monitoring is available.
```
#define NOMINAL_CONTROLLER_VOLTAGE 24.0

mySpudpole.setControllerVoltage(NOMINAL_CONTROLLER_VOLTAGE); 
```

_setMotorCurrent()_ sets the current in Amps at which the spudpole motor
is operating.  This only makes sense if continuous current monitoring is
available, so, if not, best set to zero.  N2K makes no provision for
"data not available" on this.
```
#define MOTOR_CURRENT 0.0

mySpudpole.setMotorCurrent(MOTOR_CURRENT);
```

### Failsafe command interface

_configureCommandTimeout()__ configures and enables a timeout mechanism bys
specifying a remote timer function and setting a command timeout interval in
milliseconds. The remote timer function is called from the _deploy()_ and
_retrieve()_ methods each time they are invoked, passing the timeout interval
as argument.

The remote timer function should monitor the timeout interval and if it
expires then immediately call _stop()_ method.  This strategy implements the
anchor winch control mechanism required by N2K.

The following example illustrates how this might be implemented on the Arduino
platform and assumes that a call is made to _controlTimer(0L)_ from within
_loop()_. 
```
unsigned long controlTimeout = 1000L;

void controlTimer(unsigned long interval) {
  static unsigned long threshold, startTime;
  if (interval > 0L) { // Start timing
    threshold = interval;
    startTime = millis();
  } else {
    if ((threshold > 0L) && ((startTime + threshold) < millis())) {
      threshold = 0L;
      mySpudpole->stop();
    }
  }
}

mySpudpole.configureCommandTimeout(controlTimer, controlTimeout);
```

### N2K AWI reporting methods

The following reporting methods return values in the format that is mandated
by the N2K Anchor Windlass Interface definition.

The following six methods return bit-field values describing the state of the
spudpole that conform to an the data definitions in "N2kTypes.h".

_tN2kDD477 getWindlassMonitoringEvents()_
_tN2kDD480 getWindlassMotionStatus()_
_tN2kDD481 getRodeTypeStatus()_
_tN2kDD482 getAnchorDockingStatus()_
_tN2kDD483 getWindlassOperatingEvents()_
_tN2kDD484 getWindlassDirectionControl()_

The following four methods return values which in N2K required units at the
accuracy demanded by the N2K specification.

_double getRodeCounterValue()_
_double getWindlassLineSpeed()_
_double getControllerVoltage()_
_double getMotorCurrent()_
