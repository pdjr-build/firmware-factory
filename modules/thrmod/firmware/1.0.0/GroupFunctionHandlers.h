#ifndef _GroupFunctionHandlers_H_
#define _GroupFunctionHandlers_H_

#include "NMEA2000_CompilerDefns.h"
#include "PGN128006.h"
#include "N2kGroupFunction.h"

#if !defined(N2K_NO_GROUP_FUNCTION_SUPPORT)

class GroupFunctionHandlerForPGN128006 : public tN2kGroupFunctionHandler {
  protected:
    virtual bool HandleCommand(const tN2kMsg &N2kMsg, uint8_t PrioritySetting, uint8_t NumberOfParameterPairs, int iDev);
  public:
    GroupFunctionHandlerForPGN128006(tNMEA2000 *_pNMEA2000, void (*updateFunction)(PGN128006_Field[])) : tN2kGroupFunctionHandler(_pNMEA2000, 128006L), updateFunction(updateFunction) {};
  private:
    void (*updateFunction)(PGN128006_Field[]);
};

#endif
#endif