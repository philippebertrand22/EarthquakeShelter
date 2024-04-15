# EarthquakeShelter

The Microcontroller firmware can be found in firmware_final.ino written in C. This Arduino code initializes several pins for various components such as pumps, PWM channels, a buzzer, buttons, switches, and pressure sensors. It also sets up a WiFi connection and a UDP listener on a specified port. The main loop of the program continuously checks for changes in switch state and button press, as well as incoming UDP packets. When triggered, it activates the buzzer and controls the PWM channels to inflate or deflate certain components based on pressure sensor readings. Depending on the received UDP message, it either triggers an alarm sequence or resets the system. Additionally, it periodically checks pressure levels and adjusts the PWM signals accordingly to maintain desired pressure leve
A flowchart detailing the code logic is appended below for reference.

![flow final](https://github.com/philippebertrand22/EarthquakeShelter/assets/76165234/9d942373-a2a3-4f6a-9aca-87082d5056bc)

The schematic for the circuit designed for realizing seismic detection and protection are shown below

![PCB schematic](https://github.com/philippebertrand22/EarthquakeShelter/assets/76165234/88513902-fa1a-4b3d-8e53-f62d0e889891)

The schematic design was implemented and tested with permanent soldered connections on a perfboard. Images of the prototype circuit are appended below for reference.
![1](https://github.com/philippebertrand22/EarthquakeShelter/assets/76165234/093272b9-4899-4499-9c21-e5dc3418b19d)
![PXL_20240315_002428831](https://github.com/philippebertrand22/EarthquakeShelter/assets/76165234/4cba549c-cfe4-4fb9-a928-e811e01abc12)

Pinout of Microcontroller (FREENOVE ESP32-S3 WROOM) 
![ESP32S3_Pinout](https://github.com/philippebertrand22/EarthquakeShelter/assets/76165234/31b758d3-8733-4406-9b2d-eed88cecc12b)


The seismic validation script written in python was the last software component in development for validating rsudp alarms using data from nearby sensors. The script accepts stationIDs from the RaspberryShake stationView web app. the program must receive at least three arguments, corresponding to primary_station_id and a set of secondary_station_ids. This Python script sets up a UDP server to receive messages, particularly those starting with "ALARM." Upon receiving such messages, it queries seismic data from a specified endpoint for a primary station and a list of secondary stations provided as command-line arguments. It then compares the seismic data from the primary station with the averages of the corresponding data from the secondary stations. If the differences in acceleration, velocity, and displacement are within 5% of the primary station's values, the script forwards the original message to another port specified as FORWARD_PORT. Otherwise, it logs that the secondary station data is not sufficiently similar to the primary station data. The script continuously listens for incoming packets while running.

https://github.com/raspishake/rsudp
rsudp: a tool for receiving and interacting with data casts from Raspberry Shake personal seismographs and Raspberry Boom pressure transducer instruments.
Continuous sudden motion and visual monitoring of Raspberry Shake data
Written by Ian Nesbitt (@iannesbitt), Richard Boaz, and Justin Long (@crockpotveggies)
rsudp has full documentation at https://raspishake.github.io/rsudp/. We also have tutorial instructions to install, set up, and run rsudp there. Additionally, our documentation features YouTube walkthroughs, notes for contributors, a brief Developer's guide, and module documentation.
