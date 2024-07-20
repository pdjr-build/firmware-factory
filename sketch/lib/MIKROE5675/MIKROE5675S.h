/**********************************************************************
 * @file MIKROE5675S.h
 * @author Paul Reeve (preeve@pdjr.eu)
 * @brief Interface for a bank of MikroE-5675 Relay 5 Click modules.
 * @version 0.1
 * @date 2024-07-12
 * @copyright Copyright (c) 2024
 */

#ifndef MIKROE5675S_H
#define MIKROE5675S_H

#include "MIKROE5675.h"

/**
 * @brief Interface for a bank of MikroE-5675 Relay 5 Click modules.
 *  
 * This library module is designed to interface an arbitrary number
 * of MikroE-5675 devices all of which are connected to the same host
 * I2C bus.
 * 
 * Before using any methods in this class the client application must
 * call Wire.begin() to enable microcontroller access to the I2C bus as
 * a master device.
 */
class MIKROE5675S {

  public:

    /******************************************************************
     * @brief Create a new MICROE5675S instance.
     * 
     * Create and initialise a new MICROE5675S instance by enumerating
     * the MikroE-5675 devices that are connected to it.
     * 
     * @param configs - array of tConfig structures specifying the
     * MikroE 5675 modules connected to the host I2C bus. Other
     * operations in the library use the array index as a module
     * identifier.
     */
    MIKROE5675S(MIKROE5675::tConfig *configs);

    int getModuleCount();

    /******************************************************************
     * @brief Reset all module.
     * 
     * Resets a specified MikroE 5675 module by toggling its hardware
     * RST line, configuring all parallel pins as outputs setting them
     * to 0.
     */
    int reset();

    /******************************************************************
     * @brief Get the state of a module's relays.
     * 
     * Returns an integer representing the state of all relays where
     * bit 0 reports relay 0, bit 1 reports relay 1 and so on. A bit
     * value of 0 says OPEN; 1 says CLOSED.
     * 
     * @return integer representing the state of module relays.
     */
    int getStatus();

    /******************************************************************
     * @brief Set the state of a module's relays.
     * 
     * Set the state of all relays where status bit 0 sets the state of
     * relay 0. bit 1 sets the state of relay 1 and so on. A bit value
     * of 0 says OPEN; 1 says CLOSED.
     */
    int setStatus(uint32_t status);

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
    void configureCallback(void (*callback)(int status), unsigned long callbackInterval = 1000UL);
    
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
    std::vector<MIKROE5675 *> modules;

    void (*callback)(int status);
    unsigned long callbackInterval;
    
};

#endif