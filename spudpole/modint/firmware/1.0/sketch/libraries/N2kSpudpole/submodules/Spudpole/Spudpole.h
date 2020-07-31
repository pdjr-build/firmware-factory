/**
 * Spudpole.h 20 July 2020 <preeve@pdjr.eu>
 *
 * Abstract data type modelling spudpoles from the manufacturer Ankreo.
 */
 
enum SpudpoleControl { SpudpoleControl_STOP=0, SpudpoleControl_DEPLOY=1, SpudpoleControl_RETRIEVE=2 };
enum SpudpoleTimer { SpudpoleTimer_STOP=0, SpudpoleTimer_START=1 };
enum SpudpoleState { SpudpoleState_UNKNOWN=0, SpudpoleState_DOCKED=1, SpudpoleState_DEPLOYING=2, SpudpoleState_RETRIEVING=3, SpudpoleState_STOPPED=4 };

class Spudpole {
  public:
    Spudpole();
    // Configuration
    void setControlCallback(void (*controlCallback)(SpudpoleControl));
    void configureLineMeasurement(double spoolDiameter, double lineDiameter, unsigned int spoolWidth, unsigned int workingCapacity);
    void configureRunTimeAccounting(unsigned long motorRunTime, unsigned long (*timerCallback)(SpudpoleTimer, unsigned long));
    // Primitives
    void setControllerVoltage(double controllerVoltage);
    void setMotorCurrent(double motorCurrent);
    double getControllerVoltage();
    double getMotorCurrent();
    void setDocked();
    void deploy();
    void retrieve();
    void stop();
    void setStopped();
    SpudpoleState getState();
    bool isWorking();
    bool isDocked();
    bool isDeployed();
    unsigned int getCounter();
    unsigned int incrCounter();
    unsigned int decrCounter();
    unsigned int bumpCounter();
    // If line measurement is configured...
    double getDeployedLineLength();
    // If run time accounting is configured...
    unsigned long getMotorRunTime();
  private:
    SpudpoleState state;
    double controllerVoltage;
    double motorCurrent;
    unsigned int counter;
    void (*controlCallback)(SpudpoleControl);
    double spoolDiameter;
    double lineDiameter;
    unsigned int spoolWidth;
    unsigned int lineTurnsWhenDocked;
    unsigned long (*timerCallback)(SpudpoleTimer, unsigned long);
    unsigned long motorRunTime;
    double lineLengthFromCounter(int counter);
    double lineLengthOnLayer(int layer, int turnsOnLayer);
};
