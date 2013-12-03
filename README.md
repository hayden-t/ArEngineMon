ArEngineMon
===========

Engine Sensor Monitor for Arduino

Used on Landrovers Td5 Engine but easily adaptable


It connects into the existing coolant temperature and oil pressure sensors to provide a visual readout and adjustable alarm.

Development is in testing stage, with a prototype built as seen in photos folder.

Sensors are 'tapped' into with custom harness that has Bosch Fuel Injector type plugs at each end, this is the type used on my engine, and it means i dont have to cut and engine wiring.

Engine coolant sensor is monitored via a unity gain buffer (voltage follower) circuit featuring a opa2350pa opamp to prevent any changes to the ECU's recieved sensor reading.

Oil pressure sensor is monitored via a voltage divider to bring the engine's 11-17v down to under 5v so the arduino can read it.

Menu system is provided by MENWIZ library and supports various sized lcd besides my current 16 x 2 character one.
https://github.com/brunialti/MENWIZ

Ive tried to write the code in a way that makes it expandable to more/different sensors.
