/**********************************************************************
 * @file MIKROE5981.h
 * @author Paul Reeve (preeve@pdjr.eu)
 * @brief Interface for a MikroE-5981 Digi Isolator 2 Click module.
 * @version 0.1
 * @date 2023-02-06
 * @copyright Copyright (c) 2023
 */

#ifndef MIKROE5981_H
#define MIKROE5981_H

#include <cstddef>

/**
 * @brief Interface for a MikroE-5981 Click module.
 * 
 * This interface handles a single MicroE-5981 module; if you want
 * operate multiple 5981 modules consolidated as a single switch
 * input bank then see the MIKROE5981S library.
 * 
 * A 5981 module is characterised by a MICROE5981::tPins structure
 * which specifies the MCU pins associated with the module's chip
 * select, enable and reset pins.
 * 
 * The interface does not concern itself with configuration of the
 * host SPI bus to which all modules must be connected.
 */
class MIKROE5981 {

  public:

    /******************************************************************
     * @brief Number of switch input channels in a 5981 module.
     */

    static const uint8_t CHANNEL_COUNT = 8;

    /******************************************************************
     * @brief GPIO pins used to interface with a 5981 module.
     */
    typedef struct { 
      uint8_t cs; // chip (module) select
      uint8_t en; // module enable
      uint8_t rst; // module reset
    } tPins;

    /******************************************************************
     * @brief Instantiate a new MICROE5981 instance.
     * 
     * Configures CS, EN and RST pins as outputs. The module is the
     * deselect()-ed, enable()-d and reset().
     * 
     * @param pins - GPIO pins connected to the module.
     * @param speed - SPI bus speed in MHz.
     * @param order - SPI bus byte order (either LSBFIRST or MSBFIRST).
     * @param mode - SPI bus mode (SPI_MODE0/1/2/3).
     */
    MIKROE5981(tPins pins, uint32_t speed = 14000000, uint8_t order = LSBFIRST, uint8_t mode = SPI_MODE0);

    /******************************************************************
     * @brief Reset the 5981 by toggling RST.
     */
     void reset();

    /******************************************************************
     * @brief Enable the 5981 inputs by setting EN high.
     */
    void enable();

    /******************************************************************
     * @brief Disable the 5981 inputs by setting EN low.
     */
    void disable();

    /******************************************************************
     * @brief Select the 5981 for SPI use by setting CS low.
     */
    void select();

    /******************************************************************
     * @brief Deselect the 5981 for SPI use by setting CS high.
     */
    void deselect();
    
    /******************************************************************
     * @brief Read the buffer's current status.
     * 
     * @return byte representing the state of all 5981 input channels.
     */
    uint8_t read();

    /**
     * @brief Configure an automatic callback for handling buffer data.
     * 
     * This method configures a mechanism for invoking a callback
     * function at a regular interval and works in concert with
     * callbackMaybe().
     * 
     * Make a call to configureCallback() in setup() and make a call to
     * callbackMaybe() in loop().
     * 
     * @param callback - this function will be called with the module
     * status (returned by read()).
     * @param callbackInterval - the interval in milliseconds between
     * successive invocations of the callback function (defaults to
     * 1s).
     */
    void configureCallback(void (*callback)(uint8_t status), unsigned long callbackInterval = 1000UL);
    
    /**
     * @brief Maybe invoke the configured callback.
     * 
     * This method should be called from loop() and, if force is false,
     * will invoke the callback function only if the time elapsed since
     * the previous call is greater than callbackIinterval.
     * 
     * @param force - if true then invoke callback immediately.
     */
    void callbackMaybe(bool force = false);

  private:
    tPins pins;
    uint32_t speed;
    uint8_t order;
    uint8_t mode;

    void (*callback)(uint8_t status);
    unsigned long callbackInterval;
    
};

#endif