#include <stdio.h>
#include <wiringPi.h>

// IR 센서 GPIO 핀 번호 설정 (예시)
#define IR_SENSOR_PIN 1

int main() {
    // WiringPi 라이브러리 초기화
    if (wiringPiSetup() == -1) {
        printf("WiringPi setup failed.\n");
        return 1;
    }

    // IR 센서 핀을 입력으로 설정
    pinMode(IR_SENSOR_PIN, INPUT);

    // 라인 트레이싱 루프
    while (1) {
        // IR 센서로부터 데이터 읽기
        int sensorValue = digitalRead(IR_SENSOR_PIN);

        // 센서 값에 따라 로봇의 움직임을 결정
        if (sensorValue == HIGH) {
            printf("Black line detected.\n");
            // 검은색 선을 감지한 경우의 처리 로직
        } else {
            printf("No black line detected.\n");
            // 검은색 선이 없는 경우의 처리 로직
        }

        delay(100); // 100ms 지연
    }

    return 0;
}

}
