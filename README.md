# Power Push

**Give an ordinary physical button a little Wi-Fi superpower.**

Power Push is a small, open-source, Wi-Fi-controlled servo actuator built around an M5Stack ATOM Lite. It lets you press a physical switch remotely from a phone or computer on your local network. Think PC power buttons, accessible push buttons, or other low-force controls that don't need electrical modification.

The device hosts its own responsive retro-inspired dashboard at `http://powerpush.local`. No cloud account or external web assets are required.

## What you'll need

- [M5Stack ATOM Lite ESP32 development kit](https://shop.m5stack.com/products/atom-lite-esp32-development-kit)
- [M5Stack SG90 micro servo](https://shop.m5stack.com/products/sg90-servo)
- USB-C data cable and power for the ATOM
- Regulated 5 V servo supply capable of handling the servo's stall current, plus suitable wires
- A mount and servo horn/linkage suited to your particular button
- A local 2.4 GHz Wi-Fi network and a computer with [PlatformIO](https://platformio.org/install/ide?install=vscode)

## 1. Wire it up

Leave the servo horn/linkage disconnected from the target switch during setup and calibration. Turn power off before wiring.

| Servo wire | Connect to |
| --- | --- |
| Yellow (signal) | ATOM GPIO 26 |
| Brown (ground) | Servo supply ground and ATOM GND (common ground) |
| Red (power) | Regulated external 5 V servo supply |

Power the ATOM separately over USB. Do not connect the external servo supply's positive rail to the ATOM USB supply rail. Check your particular servo's wire colors and voltage rating before powering up. Never use this project for safety-critical controls or mains wiring.

## 2. Install the firmware

Clone or download this repository, open the project folder in VS Code with PlatformIO, and connect your ATOM Lite. Select Upload in PlatformIO, or run:

```sh
pio run -t upload
```

For diagnostic output, open the serial monitor at 115200 baud (`pio device monitor`). When updating an existing installation, don't erase flash if you want to retain settings.

## 3. Join your Wi-Fi network

1. On first boot, connect your phone or computer to `PowerPush-Setup`.
2. If the setup portal doesn't open automatically, visit `http://192.168.4.1`.
3. Choose your 2.4 GHz Wi-Fi network and enter its credentials.
4. Rejoin your normal Wi-Fi network and visit [powerpush.local](http://powerpush.local).
5. If the hostname doesn't resolve, use the IP address printed in the serial monitor.

The setup hotspot is only requested when saved Wi-Fi credentials are absent. To reset credentials, hold the physical ATOM button for more than five seconds and release while the actuator is idle. Other settings are retained.

## 4. Calibrate the servo

In **Settings → Servo Calibration**, choose a rest angle and press angle using the sliders or number fields. Use Test Rest and Test Press with the linkage disconnected. Test movements last 500 ms before returning to the stored rest position. Both initial angles are 90°; normal actuation is disabled until distinct valid angles are saved.

Choose a press duration (100–2,000 ms; default 500 ms) and save calibration. Start with small movements. Verify the servo returns to rest reliably before attaching the mechanism, then check clearance and force with the mount installed. A short press on the physical ATOM button also triggers the configured cycle.

The gauges show commanded positions, not measured shaft angles. The conservative PWM defaults are 1,000–2,000 µs at 50 Hz. Advanced settings allow an extended preset of 500–2,400 µs for compatible servos. M5Stack's [SG90 documentation](https://docs.m5stack.com/en/accessory/SG90_Servo) warns that exceeding 1,000–2,000 µs may damage its servo; do not use wider values unless your specific servo is rated for them. Stop if the servo binds, buzzes or heats up. Reset buttons restore conservative defaults. Saving changed pulse limits resets the calibration to 90° and requires recalibration. Neither a 180° command nor a wider pulse range guarantees 180° mechanical travel.

## 5. Make it yours

- **Control:** press or release your switch, see live status and Wi-Fi signal.
- **Settings:** customize the hostname, switch label, optional target IPv4 address, servo calibration and optional uptime restart interval (disabled by default).
- **System:** find uptime, memory, network diagnostics, restart controls and Wi-Fi recovery instructions.

The optional target monitor checks TCP port 445; a failed connection does not prove a computer is off, and reachability does not indicate a physical switch position.

## Troubleshooting and safety

- If the hostname fails, try the device IP from the serial monitor. Wi-Fi sleep is disabled and mDNS is retried after reconnecting.
- If the servo jitters or the ATOM resets, check the external power supply and common ground.
- If the servo moves less than requested, check its rated pulse range and mechanical travel rather than forcing its endpoints.
- This interface has no authentication. Use only on a trusted local network; never expose it to the internet. The provisioning hotspot is open.
- Servo release is software-controlled, not a hardware failsafe. Verify timing, network interruption behavior and safe rest before use. No long power-off hold or battery monitoring is provided.

## Release v0.1.4

Compact header signal bars and transient feedback, PWM reset controls and extended-range preset, commanded pulse readouts, and calibration range-saving improvements. Existing stored PWM limits and angles are preserved. Previous dashboard, independent settings saves, Wi-Fi recovery and optional restart features remain in place.

Firmware compilation and physical validation of this release are pending.

## License

MIT — see the repository's LICENSE file.
