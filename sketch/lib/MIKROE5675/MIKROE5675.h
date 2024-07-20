/**********************************************************************
 * @file MIKROE5675.h
 * @author Paul Reeve (preeve@pdjr.eu)
 * @brief Interface for a MikroE-5675 Relay 5 Click module.
 * @version 0.1
 * @date 2024-07-12
 * @copyright Copyright (c) 2024
 */

#ifndef MIKROE5675_H
#define MIKROE5675_H

#include <cstddef>
#include <Wire.h>

/**
 * @brief Interface for MikroE-5675 Click module.
 * 
 * The MicroE-5675 is a MicroBus Click module containing three relays
 * operated over I2C via a PCA9538A parallel interface buffer.
 * 
 * Before using any methods in this class the client application must
 * call Wire.begin() to enable microcontroller access to the I2C bus as
 * a master device.
 */
class MIKROE5675 {

  public:

    /******************************************************************
     * @brief Number of relays in a MikroE 5675 module.
     */
    static const uint8_t CHANNEL_COUNT = 3;

    /******************************************************************
     * @brief PCA9538A control registers.
     */
    static const uint8_t PCA9538A_INPUT_REGISTER = 0x00;
    static const uint8_t PCA9538A_OUTPUT_REGISTER = 0x01;
    static const uint8_t PCA9538A_POLARITY_REGISTER = 0x02;
    static const uint8_t PCA9538A_CONFIG_REGISTER = 0x03;

    /******************************************************************
     * @brief PCA9538A default configuration (all pins as outputs).
     */
    static const uint8_t PCA9538A_CONFIG_DEFAULT = 0xF8;

    /******************************************************************
     * Return error codes (negated values returned by the 5675 module).
     */

    /******************************************************************
     * @brief MicroE 5675 configuration.
     * 
     * Specifies the associated module's i2c address and the
     * microcontroller pin associated with the module's RST input.
     */
    typedef struct {
      uint8_t address;   // i2c device address (one of 70/71/72/73)
      uint8_t gpioReset;
    } tConfig;

    /******************************************************************
     * @brief Create a new MICROE5675 instance.
     * 
     * Create and initialise a new MICROE5675 instance by specifying
     * the GPIO pins of the host bus and enumerating the MikroE-5675
     * devices that are connected to it.
     * 
     * @param config - tConfig structures specifying MikroE 5675.
     */
    MIKROE5675(MIKROE5675::tConfig config);

    /******************************************************************
     * @brief Reset the module.
     * 
     * Reset the 5675 module by toggling its hardware RST line,
     * configuring all parallel pins as outputs and setting them to 0.
     * 
     * @return 0 says success; a negative value is an error code.
     */
    int reset();

    /******************************************************************
     * @brief Get the state of the module's relays.
     * 
     * Returns an integer representing the state of all relays where
     * bit 0 reports relay 0, bit 1 reports relay 1 and bit 2 reports
     * relay 2. A bit value of 0 says OPEN; 1 says CLOSED.
     * 
     * @return a positive value indicates success and reports the
     * relay states; a negative value is an error code.
     */
    int getStatus();

    /******************************************************************
     * @brief Set the state of the module's relays.
     * 
     * Set the state of all relays where status bit 0 sets the state of
     * relay 0, bit 1 sets the state of relay 1 and bit 2 sets the
     * state of relay 3. A bit value of 0 says OPEN; 1 says CLOSED.
     * 
     * @return 0 says success; a negative value is an error code.
     */
    int setStatus(uint8_t status);

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
    tConfig config;

    void (*callback)(int status);
    unsigned long callbackInterval;
    
};

#endif