#!/usr/bin/env sh
adb install -r bin/mandelbrot-debug.apk
adb shell am start -n org.oxygine.mandelbrot/org.oxygine.mandelbrot.MainActivity
