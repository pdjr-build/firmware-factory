#include "Scheduler.h"

/**********************************************************************
 * Create a new Scheduler with the process interval specified by
 * <loopInterval>.  The specified interval is the frequency at which
 * the scheduler will check to see if a callback should be executed, so
 * its best if this is frequent.
 */
 
Scheduler::Scheduler(unsigned long loopInterval) {
    this->size = 0;
    this->loopInterval = loopInterval;
}

/**********************************************************************
 * This function must be called from the main loop(). It will execute
 * any scheduled callback functions and then delete it from the
 * collection of scheduled callback functions (unless the callback was
 * scheduled with a repeat flag in which cast the callback will be
 * re-scheduled).
 */

void Scheduler::loop() {
    static unsigned long deadline = 0UL;
    unsigned long now = millis();

    if ((now > deadline) && (this->size > 0)) {
        for (unsigned int i = 0; i < 10; i++) {
            if (this->callbacks[i].when >= now) {
                this->callbacks[i].func();
                if (!this->callbacks[i].repeat) {
                    this->callbacks[i].when = 0UL;
                    this->size--;
                } else {
                    this->callbacks[i].when = (now + this->callbacks[i].interval);
                }
            }
        }
        deadline = (now + this->loopInterval);
    } 
}

/**********************************************************************
 * Schedule <func> for callback in <interval> milliseconds. If <repeat>
 * is omitted or false, then the <func> will be called once, otherwise
 * it will be called repeatedly every <interval> milliseconds.
 */

bool Scheduler::schedule(void (*func)(), unsigned long interval, bool repeat) {
    bool retval = false;

    if (this->size < 10) {
        for (unsigned int i = 0; i < 10; i++) {
            if (this->callbacks[i].when == 0UL) {
                this->callbacks[i].func = func;
                this->callbacks[i].interval = interval;
                this->callbacks[i].repeat = repeat;
                this->callbacks[i].when = (millis() + interval);
                this->size++;
                retval = true;
                break;
            }
        }
    }
    return(retval);
}

