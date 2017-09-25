syntax = "proto3";
message Status{
  bool isPowerOn = 1;
  int32 coreTemp = 2;
  bool isIDOn = 3;
  bool isManualFanControl = 4;
  repeated int32 fanRPMs = 5;
}

message Sol{
  int32 portNum = 1;
}

message Event{
  enum EventType {
    NOEVENT = 0;
    POWERON = 1;
    POWEROFF = 2;
    DOGTRIGGERED = 3;
  }
  EventType type = 1;
}

message Command{
  message NoCommand {int32 dummy = 1;}
  message IDCommand {bool on = 1;}
  message PowerCommand {
    enum PowerOp {
      NOOP = 0;
      ON = 1;
      OFF = 2;
      RESET = 3;
    }
    PowerOp op = 1;
  }
  message FanCommand {
    enum FanOp {
      NOOP = 0;
      AUTO = 1;
      MANUAL = 2;
    }
    FanOp op = 1;
    int32 whichFan = 2;
    int32 dutyCycle = 3; // 0 to 255
  }
  message UpdateCommand {
    fixed32 serverIP = 1;
    fixed32 port = 2;
    fixed32 crc32 = 3;
  }
  oneof command {
    NoCommand noCommand = 1;
    IDCommand idCommand = 2;
    PowerCommand powerCommand = 3;
    FanCommand fanCommand = 4;
    UpdateCommand updateCommand = 5;
  }
}
