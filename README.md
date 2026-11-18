# Power Push

**Give an ordinary physical button a little Wi-Fi superpower.**

Power Push is a small, open-source, Wi-Fi-controlled servo actuator built around an M5Stack ATOM Lite. It lets you press a physical switch remotely from a phone or computer on your local network. Think PC power buttons, accessible push buttons, or other low-force controls that don't need electrical modification.

The device hosts its own responsive retro-inspired dashboard at `http://powerpush.local`. No cloud account or external web assets are required.

## What you'll need

- [M5Stack ATOM Lite ESP32 development kit](https://shop.m5stack.com/products/atom-lite-esp32-development-kit)
- [M5Stack SG90 micro servo](https://shop.m5stack.com/products/sg90-servo)
- [4-pin buckled Grove cable](https://shop.m5stack.com/products/4pin-buckled-grove-cable) — connects the ATOM Lite to the adapter included with the M5Stack servo
- USB-C data cable and power for the ATOM
- Regulated 5 V servo supply capable of handling the servo's stall current, plus suitable wires
- A mount and servo horn/linkage suited to your particular button
- A local 2.4 GHz Wi-Fi network and a computer with [PlatformIO](https://platformio.org/install/ide?install=vscode)

## 1. Wire it up

For the plug-in build used in this project, connect the ATOM Lite Grove port to the included servo adapter using the 4-pin Grove cable, then connect the adapter to the servo. Check the signal (GPIO 26), 5 V and ground routing before applying power. This configuration worked in the original build; if the servo draws more current than the ATOM/USB supply can provide, use a separately regulated servo supply with a shared ground as described below. Do not backfeed the ATOM through the adapter.

Leave the servo horn/linkage disconnected from the target switch during setup and calibration. Turn power off before wiring.

| Servo wire | Connect to |
| --- | --- |
| Yellow (signal) | ATOM GPIO 26 |
| Brown (ground) | Servo supply ground and ATOM GND (common ground) |
| Red (power) | Regulated external 5 V servo supply |

For the separate-supply wiring option, power the ATOM over USB. Do not connect the external servo supply's positive rail to the ATOM USB supply rail. Check your particular servo's wire colors and voltage rating before powering up. Never use this project for safety-critical controls or mains wiring.

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

The default range is 1,000–2,000 µs at 50 Hz, following the [M5Stack SG90 documentation](https://docs.m5stack.com/en/accessory/SG90_Servo). Power Push uses 0–180° for nominal positions; M5Stack describes the same positions as −90° to +90° around the center. A 1,500 µs pulse corresponds to 90° in Power Push. Narrowing the pulse range narrows the available angle inputs and highlights the permitted arc instead of rescaling it to 180°. These are nominal commands, not measured shaft positions.

**WARNING:** M5Stack warns that pulses outside 1,000–2,000 µs may damage its SG90. For other compatible servos, advanced settings allow 500–2,400 µs and display pulse widths directly rather than inventing physical angles. Do not use extended values with the M5Stack SG90. Reset buttons restore the documented defaults. Saving changed pulse limits resets positions to 1,500 µs and disables normal actuation until recalibration. Stop if the servo binds, buzzes or heats up.

When upgrading from v0.1.4, the firmware converts valid legacy angle commands to their previous pulse widths; the new readouts may differ without a corresponding change in commanded pulse. Disconnect the linkage before uploading and verify both positions before use. If you previously widened the range on a M5Stack SG90, consider restoring the documented defaults with the linkage disconnected.

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

## Release v0.1.5

Manufacturer-based nominal angle mapping, dynamic slider limits and gauge arcs, pulse-width mode for compatible extended-range servos, red PWM safety warning, and updated Grove wiring instructions. Existing calibration is converted from legacy commands to pulse widths; the first new calibration save stores a separate record. The old calibration keys remain untouched (downgrading will not import newer calibration changes).

Firmware compilation and physical validation of this release are pending.

## License

MIT — see the repository's LICENSE file.
