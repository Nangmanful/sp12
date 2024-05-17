#include <stdio.h>
#include <wiringPi.h>

// 4개의 트래킹 핀 정의 (WiringPi 핀 번호, 실제 GPIO 핀 번호는 주석으로 표시)
#define TRACKING_PIN1 0  // WiringPi pin 0, corresponds to GPIO 17
#define TRACKING_PIN2 1  // WiringPi pin 1, corresponds to GPIO 18
#define TRACKING_PIN3 2  // WiringPi pin 2, corresponds to GPIO 27
#define TRACKING_PIN4 3  // WiringPi pin 3, corresponds to GPIO 22
#define MOTOR_PIN 1      // Motor control pin, WiringPi pin 1, corresponds to GPIO 18

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

    // 4개의 트래킹 핀을 입력으로 설정
    pinMode(TRACKING_PIN1, INPUT);
    pinMode(TRACKING_PIN2, INPUT);
    pinMode(TRACKING_PIN3, INPUT);
    pinMode(TRACKING_PIN4, INPUT);

    // 자동차 구조체 초기화
    Car car;
    car_init(&car, MOTOR_PIN);

    while (1) {
        int trackValue1 = digitalRead(TRACKING_PIN1);
        int trackValue2 = digitalRead(TRACKING_PIN2);
        int trackValue3 = digitalRead(TRACKING_PIN3);
        int trackValue4 = digitalRead(TRACKING_PIN4);

        // 각 트래킹 핀의 값을 출력
        printf("Track Values: %d %d %d %d\n", trackValue1, trackValue2, trackValue3, trackValue4);

        // 트래킹 핀 값을 바탕으로 자동차 제어 로직
        if (trackValue1 == 0 || trackValue2 == 0 || trackValue3 == 0 || trackValue4 == 0) {
            car_forward(&car);
            printf("Forward: TrackValue1=%d TrackValue2=%d TrackValue3=%d TrackValue4=%d\n", trackValue1, trackValue2, trackValue3, trackValue4);
        } else {
            car_stop(&car);
            printf("Stop: TrackValue1=%d TrackValue2=%d TrackValue3=%d TrackValue4=%d\n", trackValue1, trackValue2, trackValue3, trackValue4);
        }

        delay(100); // 0.1초 대기
    }

    car_cleanup(&car);
    return 0;
}
