#include <stdio.h>
#include <wiringPi.h>

// 4개의 트래킹 핀 정의 (WiringPi 핀 번호, 실제 GPIO 핀 번호는 주석으로 표시)
#define TRACKING_PIN1 7  
#define TRACKING_PIN2 0  
#define TRACKING_PIN3 3  
#define TRACKING_PIN4 2  

#define MOTOR_RIGHT 8   
#define MOTOR_LEFT 9

// 자동차 초기화 함수
void car_init() {
    pinMode(MOTOR_LEFT, OUTPUT);
    pinMode(MOTOR_RIGHT, OUTPUT);

    digitalWrite(MOTOR_LEFT, LOW);
    digitalWrite(MOTOR_RIGHT, LOW);
}

// 전진 함수
void forward() {
    digitalWrite(MOTOR_LEFT, HIGH);
    digitalWrite(MOTOR_RIGHT, HIGH);
}

// 후진 함수
void backward() {
    digitalWrite(MOTOR_LEFT, LOW);
    digitalWrite(MOTOR_RIGHT, LOW);
}

// 좌회전 함수
void left() {
    digitalWrite(MOTOR_LEFT, HIGH);
    digitalWrite(MOTOR_RIGHT, LOW);
}

// 우회전 함수
void right() {
    digitalWrite(MOTOR_LEFT, LOW);
    digitalWrite(MOTOR_RIGHT, HIGH);
}

// 정지 함수
void stop() {
    digitalWrite(MOTOR_LEFT, LOW);
    digitalWrite(MOTOR_RIGHT, LOW);
}

// 회전 함수 (각도)
void turn(int angle) {
    int duration = angle * 10;  // 각도에 따라 회전 지속 시간 조절 (단위는 ms)
    if (angle > 0) {
        right();
    } else {
        left();
    }
    delay(duration);
    stop();
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

    // 자동차 초기화
    car_init();

    while (1) {
        int trackValue1 = digitalRead(TRACKING_PIN1);
        int trackValue2 = digitalRead(TRACKING_PIN2);
        int trackValue3 = digitalRead(TRACKING_PIN3);
        int trackValue4 = digitalRead(TRACKING_PIN4);

        // 각 트래킹 핀의 값을 출력
        printf("Track Values: %d %d %d %d\n", trackValue1, trackValue2, trackValue3, trackValue4);

        // 트래킹 핀 값을 바탕으로 자동차 제어 로직
        if (trackValue2 == 0 && trackValue3 == 0) {
            forward();
            printf("Forward: TrackValue1=%d TrackValue2=%d TrackValue3=%d TrackValue4=%d\n", trackValue1, trackValue2, trackValue3, trackValue4);
        } 
        else if(trackValue2 == 0 && trackValue3 == 1){
            right();
            printf("Forward: TrackValue1=%d TrackValue2=%d TrackValue3=%d TrackValue4=%d\n", trackValue1, trackValue2, trackValue3, trackValue4);
        }
        else if(trackValue2 == 1 && trackValue3 == 0){
            left();
            printf("Forward: TrackValue1=%d TrackValue2=%d TrackValue3=%d TrackValue4=%d\n", trackValue1, trackValue2, trackValue3, trackValue4);
        }
        else {
            stop();
            printf("Stop: TrackValue1=%d TrackValue2=%d TrackValue3=%d TrackValue4=%d\n", trackValue1, trackValue2, trackValue3, trackValue4);
        }

        delay(100); // 0.1초 대기
    }

    return 0;
}
