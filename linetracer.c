#include <stdio.h>
#include <wiringPi.h>

// 핀 정의
#define TRACKING_PIN 0  // WiringPi pin 0, corresponds to GPIO 17
#define MOTOR_PIN 1     // WiringPi pin 1, corresponds to GPIO 18

// 자동차 구조체 정의
typedef struct {
    int motorPin;
} Car;

// 자동차 초기화 함수
void car_init(Car *car, int motorPin) {
    car->motorPin = motorPin;
    pinMode(motorPin, OUTPUT);
    digitalWrite(motorPin, LOW);
}

// 전진 함수
void car_forward(Car *car) {
    digitalWrite(car->motorPin, HIGH);
}

// 정지 함수
void car_stop(Car *car) {
    digitalWrite(car->motorPin, LOW);
}

// 정리 함수
void car_cleanup(Car *car) {
    digitalWrite(car->motorPin, LOW);
}

int main() {
    // WiringPi 초기화
    if (wiringPiSetup() == -1) {
        printf("WiringPi setup failed!\n");
        return 1;
    }

    // 핀 설정
    pinMode(TRACKING_PIN, INPUT);

    // 자동차 구조체 초기화
    Car car;
    car_init(&car, MOTOR_PIN);

    while (1) {
        int trackValue = digitalRead(TRACKING_PIN);
        printf("Track Value: %d\n", trackValue);

        if (trackValue == 0) {
            car_forward(&car);
        } else {
            car_stop(&car);
        }
        delay(100); // 0.1초 대기
    }

    car_cleanup(&car);
    return 0;
}
