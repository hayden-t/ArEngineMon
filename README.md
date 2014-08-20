ArEngineMon
===========

Engine Sensor Monitor for Arduino

Used on a Landrover Td5 Engine but easily adaptable to any engine, car or other

It connects into the existing coolant temperature and oil pressure switch to provide a visual readout and adjustable alarm, as well as addon sensors, in my case a float switch and oil pressure sensor.

New feature, support for oil pressure sensor (pressure readout), comment out "OIL_PRESSURE_SENDER" if using oil pressure switch instead.

Development is in testing stage, with a prototype built as seen in photos folder.

Sensors are 'tapped' into with custom harness that has Bosch Fuel Injector type plugs at each end, this is the type used on my engine, and it means i dont have to cut and engine wiring.

Engine coolant temperature sensor is monitored via a unity gain buffer (voltage follower) circuit featuring a opa2350pa opamp to prevent any changes to the ECU's recieved sensor reading.

Oil pressure switch is monitored via a voltage divider to bring the engine's 11-17v down to under 5v so the arduino can read it.

Oil pressure sensor/sender is tied to ground and ranges 3-160ohms, there is a 56ohm pullup resistor on its arduino pin.

Coolant level is monitored with custom float switch in the header tank, which also is connected via a voltage divider.

Menu system is provided by MENWIZ library and supports various sized lcd besides my current 16 x 2 character one.
https://github.com/brunialti/MENWIZ

Ive tried to write the code in a way that makes it expandable to more/different sensors.
