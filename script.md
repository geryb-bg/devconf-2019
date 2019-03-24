# Sender part 1

1. `send1` - Import and init
2. `send2` - setup (false = no interrupts)
3. `send3` - detect touch on buttons
4. `send4` - print lines `Light`
5. `send4` - print lines `Temperature`
6. `send5` - readLight
7. `send6` - readTemperature
8. Check USB port and file name
9. Upload

# Receiver part 1

1. `rec1` - Import and init
2. `rec2` - setup (ProximityGain is how dirty the air is allowed to be)
3. `rec3` - detect proximity
4. `int isOn = 0;` - outside loop method
5. `rec4` - turn on and off
6. Check USB port and file name
7. Upload

# Generating Elliptic Curve private/public key pairs

1. Show in demo-config.h

# Sender part 2

1. change `readLight` and `readTemperature` to be variables
2. `send7` - Include more libraries
3. `send8` - initialise some variables
4. `send9` - WiFi
5. `send10` - Time
6. Call both from setup
7. `send11` - init cloud
8. Call from setup
9. `send12` - create jwt method
10. `send13` - connect to MQTT
11. `send14` - call connect from loop
12. `send15` - send temp and light data in loop
13. change delay to 10 secs

# Receiver part 2

1. Delete everything and run `rec5`
2. `rec6` - message handler
3. `rec7` - message received method
4. `rec8` - listen for messages

# Cloud function

1. show config
2. `func1` - Import
3. `func2` - Init
4. `func3` - Google auth
5. `func4` - Device registry name
6. `func5` - interpret data
7. `func6` - Forward data to device