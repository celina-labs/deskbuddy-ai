<h1 align="center">DeskBuddy AI</h1>

<p align="center">
  A compact ESP32-S3 desk robot with animated eyes, focus tools, and a Gemini-powered voice assistant.
</p>

<!-- Add the demo GIF here:
<p align="center">
  <img src="media/deskbuddy-demo.gif" alt="DeskBuddy AI demo" width="520">
</p>
-->

## About

DeskBuddy AI combines embedded hardware, a minimal touch interface, and generative AI in a small 3D-printed desk robot.

The firmware was developed with PlatformIO and is structured into separate modules for touch input, timers, audio recording, display rendering, Wi-Fi, and Gemini communication.

## Features

- Animated OLED eyes
- Pomodoro focus and break timer
- Sitting and standing reminders
- Handmade capacitive touch surface
- Automatic touch calibration
- Voice input through an I2S microphone
- Gemini-powered responses
- Text output optimized for a 128 x 64 display

## Interaction

| Input | Action |
| --- | --- |
| Short touch | Start a Pomodoro session |
| Long touch | Record a question and ask Gemini |

Gemini responses are displayed directly on the OLED. The robot does not use a speaker.

## Hardware

| Component | Role |
| --- | --- |
| Seeed Studio XIAO ESP32-S3 | Main controller |
| SSD1306 OLED | Face and text output |
| INMP441 I2S microphone | Voice input |
| Aluminium-foil touch surface | Capacitive input |
| 3D-printed enclosure | Robot body |

## Built with

- C++ and the Arduino framework
- PlatformIO
- I2C and I2S
- SPIFFS
- Gemini API
- ArduinoJson
- Adafruit GFX and Adafruit SSD1306
- FluxGarage RoboEyes

## Credits

The enclosure was printed on my own 3D printer using the [Compagnon 309 model by Leroyd](https://makerworld.com/de/models/2109424-compagnon-309-build-your-expressive-robot?from=search#profileId-2281938). The model files are not redistributed in this repository. The firmware, electronics integration, soldering, touch implementation, and feature set were developed independently.

The animated face uses the [FluxGarage RoboEyes](https://github.com/FluxGarage/RoboEyes) library by Dennis Hoelscher.

## License

This project is licensed under the [GNU General Public License v3.0 or later](LICENSE).
