/* Automatically generated nanopb header */
/* Generated by nanopb-0.3.9-dev at Sun Sep 24 23:38:31 2017. */

#ifndef PB_CONTROL_CHANNEL_PB_H_INCLUDED
#define PB_CONTROL_CHANNEL_PB_H_INCLUDED
#include <pb.h>

/* @@protoc_insertion_point(includes) */
#if PB_PROTO_HEADER_VERSION != 30
#error Regenerate this file with the current version of nanopb generator.
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Enum definitions */
typedef enum _Event_EventType {
    Event_EventType_NOEVENT = 0,
    Event_EventType_POWERON = 1,
    Event_EventType_POWEROFF = 2,
    Event_EventType_DOGTRIGGERED = 3
} Event_EventType;
#define _Event_EventType_MIN Event_EventType_NOEVENT
#define _Event_EventType_MAX Event_EventType_DOGTRIGGERED
#define _Event_EventType_ARRAYSIZE ((Event_EventType)(Event_EventType_DOGTRIGGERED+1))

typedef enum _Command_PowerCommand_PowerOp {
    Command_PowerCommand_PowerOp_NOOP = 0,
    Command_PowerCommand_PowerOp_ON = 1,
    Command_PowerCommand_PowerOp_OFF = 2,
    Command_PowerCommand_PowerOp_RESET = 3
} Command_PowerCommand_PowerOp;
#define _Command_PowerCommand_PowerOp_MIN Command_PowerCommand_PowerOp_NOOP
#define _Command_PowerCommand_PowerOp_MAX Command_PowerCommand_PowerOp_RESET
#define _Command_PowerCommand_PowerOp_ARRAYSIZE ((Command_PowerCommand_PowerOp)(Command_PowerCommand_PowerOp_RESET+1))

typedef enum _Command_FanCommand_FanOp {
    Command_FanCommand_FanOp_NOOP = 0,
    Command_FanCommand_FanOp_AUTO = 1,
    Command_FanCommand_FanOp_MANUAL = 2
} Command_FanCommand_FanOp;
#define _Command_FanCommand_FanOp_MIN Command_FanCommand_FanOp_NOOP
#define _Command_FanCommand_FanOp_MAX Command_FanCommand_FanOp_MANUAL
#define _Command_FanCommand_FanOp_ARRAYSIZE ((Command_FanCommand_FanOp)(Command_FanCommand_FanOp_MANUAL+1))

/* Struct definitions */
typedef struct _Command_FanCommand {
    Command_FanCommand_FanOp op;
    int32_t whichFan;
    int32_t dutyCycle;
/* @@protoc_insertion_point(struct:Command_FanCommand) */
} Command_FanCommand;

typedef struct _Command_IDCommand {
    bool on;
/* @@protoc_insertion_point(struct:Command_IDCommand) */
} Command_IDCommand;

typedef struct _Command_NoCommand {
    int32_t dummy;
/* @@protoc_insertion_point(struct:Command_NoCommand) */
} Command_NoCommand;

typedef struct _Command_PowerCommand {
    Command_PowerCommand_PowerOp op;
/* @@protoc_insertion_point(struct:Command_PowerCommand) */
} Command_PowerCommand;

typedef struct _Command_UpdateCommand {
    uint32_t serverIP;
    uint32_t port;
    uint32_t crc32;
/* @@protoc_insertion_point(struct:Command_UpdateCommand) */
} Command_UpdateCommand;

typedef struct _Event {
    Event_EventType type;
/* @@protoc_insertion_point(struct:Event) */
} Event;

typedef struct _Sol {
    int32_t portNum;
/* @@protoc_insertion_point(struct:Sol) */
} Sol;

typedef struct _Status {
    bool isPowerOn;
    int32_t coreTemp;
    bool isIDOn;
    bool isManualFanControl;
    pb_callback_t fanRPMs;
/* @@protoc_insertion_point(struct:Status) */
} Status;

typedef struct _Command {
    pb_size_t which_command;
    union {
        Command_NoCommand noCommand;
        Command_IDCommand idCommand;
        Command_PowerCommand powerCommand;
        Command_FanCommand fanCommand;
        Command_UpdateCommand updateCommand;
    } command;
/* @@protoc_insertion_point(struct:Command) */
} Command;

/* Default values for struct fields */

/* Initializer values for message structs */
#define Status_init_default                      {0, 0, 0, 0, {{NULL}, NULL}}
#define Sol_init_default                         {0}
#define Event_init_default                       {(Event_EventType)0}
#define Command_init_default                     {0, {Command_NoCommand_init_default}}
#define Command_NoCommand_init_default           {0}
#define Command_IDCommand_init_default           {0}
#define Command_PowerCommand_init_default        {(Command_PowerCommand_PowerOp)0}
#define Command_FanCommand_init_default          {(Command_FanCommand_FanOp)0, 0, 0}
#define Command_UpdateCommand_init_default       {0, 0, 0}
#define Status_init_zero                         {0, 0, 0, 0, {{NULL}, NULL}}
#define Sol_init_zero                            {0}
#define Event_init_zero                          {(Event_EventType)0}
#define Command_init_zero                        {0, {Command_NoCommand_init_zero}}
#define Command_NoCommand_init_zero              {0}
#define Command_IDCommand_init_zero              {0}
#define Command_PowerCommand_init_zero           {(Command_PowerCommand_PowerOp)0}
#define Command_FanCommand_init_zero             {(Command_FanCommand_FanOp)0, 0, 0}
#define Command_UpdateCommand_init_zero          {0, 0, 0}

/* Field tags (for use in manual encoding/decoding) */
#define Command_FanCommand_op_tag                1
#define Command_FanCommand_whichFan_tag          2
#define Command_FanCommand_dutyCycle_tag         3
#define Command_IDCommand_on_tag                 1
#define Command_NoCommand_dummy_tag              1
#define Command_PowerCommand_op_tag              1
#define Command_UpdateCommand_serverIP_tag       1
#define Command_UpdateCommand_port_tag           2
#define Command_UpdateCommand_crc32_tag          3
#define Event_type_tag                           1
#define Sol_portNum_tag                          1
#define Status_isPowerOn_tag                     1
#define Status_coreTemp_tag                      2
#define Status_isIDOn_tag                        3
#define Status_isManualFanControl_tag            4
#define Status_fanRPMs_tag                       5
#define Command_noCommand_tag                    1
#define Command_idCommand_tag                    2
#define Command_powerCommand_tag                 3
#define Command_fanCommand_tag                   4
#define Command_updateCommand_tag                5

/* Struct field encoding specification for nanopb */
extern const pb_field_t Status_fields[6];
extern const pb_field_t Sol_fields[2];
extern const pb_field_t Event_fields[2];
extern const pb_field_t Command_fields[6];
extern const pb_field_t Command_NoCommand_fields[2];
extern const pb_field_t Command_IDCommand_fields[2];
extern const pb_field_t Command_PowerCommand_fields[2];
extern const pb_field_t Command_FanCommand_fields[4];
extern const pb_field_t Command_UpdateCommand_fields[4];

/* Maximum encoded size of messages (where known) */
/* Status_size depends on runtime parameters */
#define Sol_size                                 11
#define Event_size                               2
#define Command_size                             26
#define Command_NoCommand_size                   11
#define Command_IDCommand_size                   2
#define Command_PowerCommand_size                2
#define Command_FanCommand_size                  24
#define Command_UpdateCommand_size               15

/* Message IDs (where set with "msgid" option) */
#ifdef PB_MSGID

#define CONTROL_CHANNEL_MESSAGES \


#endif

#ifdef __cplusplus
} /* extern "C" */
#endif
/* @@protoc_insertion_point(eof) */

#endif
