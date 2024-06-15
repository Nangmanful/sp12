# Sytem Programing Group 12
202011061 김창규<br>
202011169 이재훈<br>
202211153 이지은<br>

# Overview
Our team completed two tasks. First, we implemented a program for an IR tracking sensor to trace a black line. Second, we implemented QR code recognition. The line tracing code was implemented in C and used wiringPi to control the GPIO. The QR code recognition was implemented in C++ using the OpenCV module to recognize QR codes.
<br><br>
# BEFORE START

## ~~ENVIRONMENT~~

Build wiringpi  from  https://github.com/WiringPi/WiringPi.git<br>
And execute the following command:

```bash
sudo apt-get install i2c-tools libi2c-dev
sudo apt-get install libopencv-dev
```
## Fetch the code from GitHub
```bash
git clone https://github.com/Nangmanful/sp12
cd sp12
make
```

<br>

# ~~LINETRACER~~ (FOR DEMO IT WAS REPLACED, SO DO NOT WORK NOW)

```bash
sudo ./linetracer
```
Then Raspbot will move, tracing the black line. <br>
To stop the program, press Ctrl + C <br>
Or use the linked remote control to shut it down.
## TEST RESULT-LINETRACER
We tested the track in both clockwise and counterclockwise directions.

Clockwise<br>

https://github.com/Nangmanful/sp12/assets/154849613/08da5865-4399-42b9-84d0-dd6f86393573


Counterclockwise<br>

https://github.com/Nangmanful/sp12/assets/154849613/909b5be8-a4cf-4695-bdd4-833216020d9b

<br>

# ~~QRCODERECOGNITION~~ (FOR DEMO IT WAS REPLACED, SO DO NOT WORK NOW)

```bash
./qrrecognition
```
When you run the executable file, a window displaying the camera feed will open. You can then use this to scan the QR code.

## TEST RESULT-QRCODERECOGNITION


https://github.com/Nangmanful/sp12/assets/154849613/08028528-1354-4cc7-a31a-071c209830f2

Source of the QR code used:http://www.yahboom.net/study/Raspbot
