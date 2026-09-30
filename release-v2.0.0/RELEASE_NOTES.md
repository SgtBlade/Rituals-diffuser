# v2.0.0 - New Look, Cartridge Usage and a More Robust Diffuser

A major update: a web interface in the style of the Rituals app that you can install on your phone, cartridge usage in the original Rituals format, and a diffuser that recovers on its own from interrupted updates, weak WiFi and a stuck fan.

## New look, installable as an app

- **Restyled after the Rituals app:** warm blurred background that slowly breathes while the diffuser runs, translucent glass cards, serif typography, cream buttons and sand-coloured switches.
- **Two tabs:** Control and Settings. The firmware page is now under Settings.
- **Control:** large on/off button, soft/intense speed slider, cartridge card with usage, timer and interval mode.
- **Smaller type on phones**, desktop unchanged in size.
- **Install on your home screen:** the diffuser now has an app icon and manifest. On iPhone it opens full-screen like an app. No MQTT or cloud needed. See *Install as an App* in the README (Android: Chrome only offers a full install over HTTPS; the README explains the workaround).
- **No extra RAM:** everything is served from flash, 27 KB gzipped including the icon.

## Cartridge

- **Usage per cartridge** in the original Rituals tag format: minutes at full speed (NFC page 6) and first-use date (page 7). Shown on the cartridge card, in `/api/status` and as a new Home Assistant sensor **Cartridge Usage**.
- **Optional: save usage on the cartridge** (Settings → Cartridge, off by default). Like the original firmware, the usage is written to the tag so it moves with the cartridge. Only pages 6 and 7 are written, every write is verified, and only on tags where those pages are not locked or password protected. Writes happen when a blow period ends and at most every 10 minutes while the fan runs continuously, to keep tag wear low.
- **Scent table corrected:** ten scents that were not recognized before (Happy Buddha, Oriental Vetiver, Black Oudh, Cotton Blossom, Precious Amber, Sweet Jasmine, Imperial Rose, Green Cardamom, Royal Tea, Jing Night) are now detected.
- Home Assistant **Cartridge Present** shows *plugged in / unplugged* instead of *home / away*.

## Fan

- **Auto-off failsafe:** without a timer the diffuser turns off after 4 hours without any interaction (web, Home Assistant or button), so a forgotten diffuser doesn't empty a cartridge. Adjustable from 1 to 4 hours. Every interaction restarts the period. Home Assistant's *Time Left* shows the countdown.
- **Stall detection:** when the fan gets power but does not turn, the web interface shows *Fan not turning*, the LED shows the error state, the event is logged and Home Assistant gets a **Fan Problem** sensor.
- More accurate RPM reading when the device is busy.

## WiFi

- **Mesh and repeaters:** the ESP32 now joins the strongest access point instead of the first one it finds.
- **Roaming** (ESP32 and ESP8266): when the signal gets weak, the diffuser looks for a clearly stronger access point of the same network and switches to it.
- The router lists the diffuser as `rituals-diffuser` instead of a chip name.
- `/api/status` shows the connected access point and channel (ESP32).

## Reliability

- **Interrupted updates no longer break the diffuser.** A web interface upload that fails halfway (weak WiFi, power loss) is cleaned up automatically, and the diffuser always starts.
- **Recovery page:** when the web interface files are missing, the diffuser shows a page to upload them again over WiFi.
- A failed ArduinoOTA upload no longer leaves the diffuser in update mode.
- Logs survive a power loss during saving.

## Updating

Update **both** files: the firmware and the web interface (filesystem).

1. Open the web interface, go to **Firmware** (v1.11: Firmware tab, v2.0: Settings → Firmware) and upload the firmware file, then the filesystem file.
2. ESP32 / ESP32-C3: *Install update* downloads and installs both files from GitHub automatically.

Your settings (WiFi, MQTT, fan, night mode) are kept; they are migrated to the new settings format automatically.

## Binaries

| File | Platform | Flash Address |
|------|----------|---------------|
| `firmware_esp8266.bin` | ESP8266 | `0x0` |
| `littlefs_esp8266.bin` | ESP8266 | `0x1E0000` |
| `firmware_esp32.bin` | ESP32 | `0x10000` |
| `spiffs_esp32.bin` | ESP32 | `0x3D0000` |
| `firmware_esp32c3.bin` | ESP32-C3 SuperMini | `0x10000` |
| `spiffs_esp32c3.bin` | ESP32-C3 SuperMini | `0x3D0000` |
