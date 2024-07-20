/**********************************************************************
 * @file MIKROE5981S.h
 * @author Paul Reeve (preeve@pdjr.eu)
 * @brief Interface for an input buffer made of multiple MikroE-5981
 * Digi Isolator 2 Click modules.
 * @version 0.1
 * @date 2023-02-06
 * @copyright Copyright (c) 2023
 */

#ifndef MIKROE5981S_H
#define MIKROE5981S_H

#include <cstddef>
#include "MIKROE5981.h"

/**
 * @brief Interface for an input buffer made of multiple MikroE-5981
 * Digi Isolator 2 Click modules.
 * 
 * This interface handles up to a maximum of four MicroE-5981 modules
 * allowing the recovery of external input states for all modules
 * through a single 32-bit unsigned integer.
 * 
 * Each module is characterised by a MICROE5981::tPins structure which
 * specifies the MCU pins associated with the module's chip select,
 * enable and reset pins. Configuration information is supplied
 * as an array of tPins structures with the first element in the
 * array describing Module 0, the second element Module 1 and so-on.
 * The configuration array must be terminated by a tPins structure with
 * the value { 0, 0, 0 }. The configuration array for an interface to
 * two Click 5981 modules might have the form.
 * 
 * MICROE5981::tPins config[3] = { {3,4,5}, {12,13,14}, {0,0,0} };
 * 
 * The order of tPins structures in the array is significant and
 * determines the order of bytes in the 32-bit integer which is
 * returned to report the state of each modules external inputs.
 * Elements in the array are ordered hi to low in terms of bits
 * returned by the read() operation.
 */
class MIKROE5981S {

  public:

    /******************************************************************
     * Create and initialise a new MICROE5981S instance by specifying
     * the configuration for a maximum of four Click 5981 modules and
     * optionally the SPI bus speed, byte order and mode.
     * 
     * @param modules - array of tPins objects specifying 5981 modules
     * connected to the host SPI bus.
     * @param speed - SPI bus speed in MHz.
     * @param order - SPI bus byte order (either LSBFIRST or MSBFIRST).
     * @param mode - SPI bus mode (SPI_MODE0/1/2/3).
     */
    MIKROE5981S(MIKROE5981::tPins *pins, uint32_t speed = 14000000, uint8_t order = LSBFIRST, uint8_t mode = SPI_MODE0);

    void reset();

    /******************************************************************
     * @brief Get the number of configured modules.
     */
    uint8_t getModuleCount();

    MIKROE5981 *getModule(uint8_t index);

    /******************************************************************
     * @brief Read the buffer's current state.
     * 
     * @return integer representing the status of each configured
     * Click 5981 module.
     */
    uint32_t read();

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
     * @param callback - this function will be called with the current an array
     * containing the status bytes of each IC in the buffer
     * daisy-chain.
     * @param callbackInterval - the interval in milliseconds between
     * successive invocations of the callback function (defaults to
     * 1s).
     */
    void configureCallback(void (*callback)(uint32_t status), unsigned long callbackInterval = 1000UL);
    
    /**
     * @brief Maybe invoke the configured callback.
     * 
     * This method should be called from loop() and, if force is false,
     * will determine when to invoke the callback function based upon
     * the specified update interval.
     * 
     * @param force - if true then invoke callback immediately.
     */
    void callbackMaybe(bool force = false);

  private:
    std::vector<MIKROE5981*> modules;
    uint32_t speed;
    uint8_t order;
    uint8_t mode;

    void (*callback)(uint32_t status);
    unsigned long callbackInterval;
    
};

#endif