/**********************************************************************
 * GroupFunctionHandlers.cpp
 * Copyright (c) 2021 Paul Reeve <preeve@pdjr.eu>
 * 
 * This file implements a number of classes providing group function
 * support for PGNs 128006, 128007 and 128008.
 * 
 * GroupFunctionHandlerForPGN128006 implements a Command function
 * handler which allows a remote to update any field.
 * 
 * GroupFunctionHandlerForPGN128007 implements a Request function
 * handler which allows a remote to request transmission of the
 * eponymous PGN. Note that requests for individual field values are
 * not honoured - any request results in transmission of a complete
 * PGN
 */ 

#include <string.h>
#include <NMEA2000StdTypes.h>
#include "GroupFunctionHandlers.h"
#include "NMEA2000.h"

#if !defined(N2K_NO_GROUP_FUNCTION_SUPPORT)

///////////////////////////////////////////////////////////////////////
// START OF HANDLERS FOR PGN128006
///////////////////////////////////////////////////////////////////////

bool GroupFunctionHandlerForPGN128006::HandleCommand(const tN2kMsg &N2kMsg, uint8_t PrioritySetting, uint8_t NumberOfParameterPairs, int iDev) {
  int i;
  int Index;
  uint8_t field;
  tN2kGroupFunctionTransmissionOrPriorityErrorCode pec = N2kgfTPec_Acknowledge;
  tN2kGroupFunctionParameterErrorCode PARec;
  tN2kMsg N2kRMsg;
  int canUpdate = true;
  PGN128006_UpdateField fields[] = { {false,0},{false,0},{false,0},{false,0},{false,0},{false,0},{false,0},{false,0},{false,0},{false,0} };

 	if (PrioritySetting != 0x08 || PrioritySetting != 0x0f || PrioritySetting != 0x09) pec = N2kgfTPec_TransmitIntervalOrPriorityNotSupported;

  SetStartAcknowledge(
    N2kRMsg,
    N2kMsg.Source,
    128006L,
    N2kgfPGNec_Acknowledge,  // What we actually should response as PGN error, if we have invalid field?
    pec,
    NumberOfParameterPairs
  );

  StartParseCommandPairParameters(N2kMsg, Index);
  for (i = 0; i < NumberOfParameterPairs; i++) {
    field = N2kMsg.GetByte(Index);
    PARec = N2kgfpec_Acknowledge;
    switch (field) {
      case PGN128006_ThrusterIdentifier_FieldIndex:
        fields[field].modified = true;
        fields[field].value.F02 = N2kMsg.GetByte(Index);
        break;
      case PGN128006_ThrusterDirectionControl_FieldIndex:
        switch(N2kMsg.GetByte(Index) & 0x0f) {
          case 0: fields[field].modified = true; fields[field].value.F03 = N2kDD473_OFF; break;
          case 1: fields[field].modified = true; fields[field].value.F03 = N2kDD473_ThrusterReady; break;
          case 2: fields[field].modified = true; fields[field].value.F03 = N2kDD473_ThrusterToPORT; break;
          case 3: fields[field].modified = true; fields[field].value.F03 = N2kDD473_ThrusterToSTARBOARD; break;
          default: PARec = N2kgfpec_RequestOrCommandParameterOutOfRange; canUpdate = false; break;
        }
        break;
      case PGN128006_PowerEnable_FieldIndex:
        switch(N2kMsg.GetByte(Index) & 0x03) {
          case 0: fields[field].modified = true; fields[field].value.F04 = N2kDD002_Off; break;
          case 1: fields[field].modified = true; fields[field].value.F04 = N2kDD002_On; break;
          case 2: fields[field].modified = true; fields[field].value.F04 = N2kDD002_Error; break;
          case 3: fields[field].modified = true; fields[field].value.F04 = N2kDD002_Unavailable; break;
          default: PARec = N2kgfpec_RequestOrCommandParameterOutOfRange; canUpdate = false; break;
        }
        break;
      case PGN128006_ThrusterRetractControl_FieldIndex:
        switch(N2kMsg.GetByte(Index) & 0x03) {
          case 0: fields[field].modified = true; fields[field].value.F05 = N2kDD474_OFF; break;
          case 1: fields[field].modified = true; fields[field].value.F05 = N2kDD474_Extend; break;
          case 2: fields[field].modified = true; fields[field].value.F05 = N2kDD474_Retract; break;
          default: PARec = N2kgfpec_RequestOrCommandParameterOutOfRange; canUpdate = false; break;
        }
        break;
      case PGN128006_SpeedControl_FieldIndex:
        fields[field].modified = true; fields[field].value.F06 = N2kMsg.GetByte(Index);
        break;
      case PGN128006_ThrusterControlEvents_FieldIndex:
        fields[field].modified = true; fields[field].value.F07.SetEvents(N2kMsg.GetByte(Index));
        break;
      case PGN128006_CommandTimeout_FieldIndex:
        fields[field].modified = true; fields[field].value.F08 = N2kMsg.Get1ByteUDouble(0.005, Index);
        break;      
      case PGN128006_AzimuthControl_FieldIndex:
        fields[field].modified = true; fields[field].value.F09 = N2kMsg.Get2ByteDouble(0.0001, Index);
        break;
      default:
        PARec = N2kgfpec_InvalidRequestOrCommandParameterField;
    }
    AddAcknowledgeParameter(N2kRMsg, i, PARec);
  }
  pNMEA2000->SendMsg(N2kRMsg, iDev);

  if (canUpdate) this->updateFunction(fields);

  return true;
}

///////////////////////////////////////////////////////////////////////
// END OF HANDLERS FOR PGN128006
///////////////////////////////////////////////////////////////////////

#endif