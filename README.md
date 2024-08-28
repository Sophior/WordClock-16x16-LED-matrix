# WordClock-16x16-LED-matrix

Based on the great work of <a href="https://github.com/AWSW-de/WordClock-16x16-LED-matrix">AWSW</a>

While trying to run it on a simple ESP32-CAM I came across some issues

* Settings didn't work with that ESP32
* The libraries are quite big
* The pixel library didn't work for me
* Base files for PlatformIO and WokWi emulator (you can skip this and just copy the code from the main.cpp in an ino file and be done)

So this version is reduced to FastLED with a time server.


