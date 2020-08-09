/**
 * N2kSpudpole.cpp 20 July 2020 <preeve@pdjr.eu>
 *
 * Abstract data type modelling NMEA 2000 enabled spudpoles from the manufacturer Ankreo.
 */
 
#include <cstddef>
#include <string.h>
#include <N2kMessages.h>
#include "N2kSpudpole.h"


N2kSpudpole::N2kSpudpole(N2kSpudpole::Settings settings):
  Spudpole(settings.spudpoleSettings) {  
  this->settings = settings;
  this->commandTimeout = this->settings.defaultCommandTimeout;
}

N2kSpudpole::Settings N2kSpudpole::getN2kSpudpoleSettings() {
  return(this->settings);
}

void N2kSpudpole::setCommandTimeout(double seconds) {
  this->commandTimeout = seconds;
}

double N2kSpudpole::getCommandTimeout() {
  return(this->commandTimeout);
}