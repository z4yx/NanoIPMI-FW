#ifndef ATX_H__
#define ATX_H__ 

#include "control-channel.pb.h"

void ATX_PowerCommand(Command_PowerCommand_PowerOp op);
void ATX_Task(void);
bool ATX_GetPowerOnState(void);


#endif