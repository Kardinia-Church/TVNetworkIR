# TVNetworkIR
 A ESP8266 project for controlling a TV using IR

# Packet structure
The device expects a broadcast message (255.255.255.255) of size 16 sent on the configured port (default 2390) as follows:
```
0x49 0x52 0x54 0x56 [P P P P P P P P] [I] [C C C]
IRTV <PASSWORD length 8> <TV ID length 1> <CMD length 3>
```
Example using a PON command with password "pass" to TV with id 1
```
[0x49 0x52 0x54 0x56] [0x70 0x61 0x73 0x73 0x00 0x00 0x00 0x00] [0x01] [0x50 0x4F 0x4E]

[] is each part of the packet
```
Note the password is not encrypted!

Each command will reply to the host at the configured answer port (default 2391) with:
```
IRTVOK if successful
IRTVFAIL if not successful
```
or if the command had a speicifc reply it'll reply with that

# Supported commands
- Power On/Off
```
PON will turn on the tv
POF will turn off the tv
```
- Get power status
```
CPW will return the current power state of the tv as IRTV<state> (1 = on 0 = off)
```

# Custom commands
If you want to add custom commands this can be done in the commands.h file.