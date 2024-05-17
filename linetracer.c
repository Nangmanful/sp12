#include <stdio.h>
#include <wiringPi.h>

// 핀 정의
#define TRACKING_PIN 0  // WiringPi pin 0, corresponds to GPIO 17
#define MOTOR_PIN 1     // WiringPi pin 1, corresponds to GPIO 18

// 자동차 클래스 정의
class Car {
public:
    Car() {
        pinMode(MOTOR_PIN, OUTPUT);
        digitalWrite(MOTOR_PIN, LOW);
    }

    void forward() {
        digitalWrite(MOTOR_PIN, HIGH);
    }

    void stop() {
        digitalWrite(MOTOR_PIN, LOW);
    }

    void cleanup() {
        digitalWrite(MOTOR_PIN, LOW);
    }
};

int main() {
    // WiringPi 초기화
    if (wiringPiSetup() == -1) {
        printf("WiringPi setup failed!\n");
        return 1;
    }

    // 핀 설정
    pinMode(TRACKING_PIN, INPUT);

    Car car;

    while (1) {
        int trackValue = digitalRead(TRACKING_PIN);
        printf("Track Value: %d\n", trackValue);

        if (trackValue == 0) {
            car.forward();
        } else {
            car.stop();
        }
        delay(100); // 0.1초 대기
    }

    car.cleanup();
    return 0;
}
