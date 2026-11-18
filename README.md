# M5Atom-Power-Push
v0.1.3 — Enhanced Dashboard, Calibration & System Maintenance. Firmware build and hardware validation pending.

## Build
Install PlatformIO, open this directory, connect the ATOM Lite over USB, run `pio run -t upload`. Serial monitor: `pio device monitor`.

## Setup
Keep servo linkage disconnected during initial setup. Servo PWM GPIO26, regulated external 5V servo supply and common ground with ATOM. Do not connect external 5V to the ATOM USB supply rail. On first boot join PowerPush-Setup and enter Wi-Fi credentials through its captive portal. Open http://powerpush.local or the assigned IP. Set safe rest and press angles and save before enabling the actuator. Both initially default to 90 degrees; press is disabled until calibration. Test moves return to the stored rest after 500 ms. The physical button performs a short press; hold over five seconds and release to reset Wi-Fi. Default press 500 ms, maximum 2000 ms. No long power-off hold in this version.

Target monitoring tests TCP port 445 on an optional IPv4 address; a failed connection is not proof the machine is off. No authentication: use only a trusted isolated local network, never expose to the internet. No battery monitoring. No external web dependencies. Hardware validation and network timeout/release testing are required before attaching to the switch. Servo release is software-controlled, not a hardware failsafe.

## Network diagnostics
Wi-Fi sleep is disabled; mDNS is restarted after reconnection or IP changes and retried if initialization fails. Serial output reports boot reset reason, local IP, RSSI, mDNS and HTTP startup. Browser polling is slower when idle. Existing saved settings are retained when uploading without erasing flash. If .local fails, try the printed IP address.

## v0.1.2
Angle sliders select values without moving the servo. Numerical entries are checked inline and clamped on change; test/save actions require valid whole numbers. The backend independently validates the command range. The 0–180 degree command range is not a guarantee of mechanical travel. Existing networking and settings are preserved.

## v0.1.3
Responsive blue navigation, lime actuator, live status and 5-second success notifications, compact Wi-Fi panel and header RSSI, animated commanded-angle SVG gauges, independent device and calibration saves/discard. Setup AP is requested only when no stored Wi-Fi credentials exist; normal reconnect uses station mode. Manual restart and optional uptime restart (disabled by default; selectable intervals from 5 minutes to 30 days) defer until idle. Uptime uses the ESP 64-bit timer. System displays minimum free heap and recovery guidance.

PWM range defaults to the previous 1000–2000 microseconds. Changing endpoints requires confirmation with linkage disconnected and resets calibration to 90 degrees, disabling normal actuation until new distinct angles are saved. Expand in small increments and never force mechanical travel; angle is a command, not measured position. Existing settings are retained on normal upload. Test first with linkage disconnected. Setup AP is open when provisioning; use only a trusted network. Target checks remain TCP port 445. Firmware compilation and hardware tests pending; Chromium UI tested with simulated API responses, not an actual device.
