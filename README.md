# M5Atom-Power-Push
v0.1.0 — initial development firmware. Not hardware tested.

## Build
Install PlatformIO, open this directory, connect the ATOM Lite over USB, run `pio run -t upload`. Serial monitor: `pio device monitor`.

## Setup
Keep servo linkage disconnected during initial setup. Servo PWM GPIO26, regulated external 5V servo supply and common ground with ATOM. Do not connect external 5V to the ATOM USB supply rail. On first boot join PowerPush-Setup and enter Wi-Fi credentials through its captive portal. Open http://powerpush.local or the assigned IP. Set safe rest and press angles and save before enabling the actuator. Both initially default to 90 degrees; press is disabled until calibration. Test moves return to the stored rest after 500 ms. The physical button performs a short press; hold over five seconds and release to reset Wi-Fi. Default press 500 ms, maximum 2000 ms. No long power-off hold in this version.

Target monitoring tests TCP port 445 on an optional IPv4 address; a failed connection is not proof the machine is off. No authentication: use only a trusted isolated local network, never expose to the internet. No battery monitoring. No external web dependencies. Hardware validation and network timeout/release testing are required before attaching to the switch. Servo release is software-controlled, not a hardware failsafe.
