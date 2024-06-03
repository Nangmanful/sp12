#include <stdio.h>
#include <wiringPi.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#include <errno.h>

#define TRACKING_PIN1 7
#define TRACKING_PIN2 0
#define TRACKING_PIN3 3
#define TRACKING_PIN4 2

// MCU I2C 주소
#define MCU_ADDRESS 0x16

// 레지스터 정의
#define REG_MOTOR_CONTROL 0x01
#define REG_STOP_ALL 0x02
#define REG_SERVO_CONTROL 0x03

// 모터 방향 정의
#define b 0
#define f 1

int i2c_fd;

// 함수 선언
void Car_Control(int leftDir, int leftSpeed, int rightDir, int rightSpeed) {
    if (leftSpeed < 0) leftSpeed = 0;
    if (leftSpeed > 180) leftSpeed = 180;
    if (rightSpeed < 0) rightSpeed = 0;
    if (rightSpeed > 180) rightSpeed = 180;

    unsigned char buffer[5];
    buffer[0] = REG_MOTOR_CONTROL;
    buffer[1] = leftDir;
    buffer[2] = leftSpeed;
    buffer[3] = rightDir;
    buffer[4] = rightSpeed;

    if (write(i2c_fd, buffer, 5) != 5) {
        perror("Failed to write to the i2c bus");
    }
}

void Car_Stop() {
    unsigned char buffer[2];
    buffer[0] = REG_STOP_ALL;
    buffer[1] = 0x00;

    if (write(i2c_fd, buffer, 2) != 2) {
        perror("Failed to write to the i2c bus");
    }
}

void Car_Run(int leftSpeed, int rightSpeed) {
    Car_Control(f, leftSpeed, f, rightSpeed);
}
void Car_Back(int leftSpeed, int rightSpeed) {
    Car_Control(b, leftSpeed, b, rightSpeed);
}
void Car_Left(int leftSpeed, int rightSpeed) {
    Car_Control(b, leftSpeed, f, rightSpeed);
}
void Car_Right(int leftSpeed, int rightSpeed) {
    Car_Control(f, leftSpeed, b, rightSpeed);
}


void controlServo(int fd, int servoNum, int angle);

int main(void) {
    // I2C 장치 파일 열기
    i2c_fd = open("/dev/i2c-1", O_RDWR);
    if (i2c_fd < 0) {
        perror("Failed to open the i2c bus");
        return 1;
    }

    // I2C 장치 주소 설정
    if (ioctl(i2c_fd, I2C_SLAVE, MCU_ADDRESS) < 0) {
        perror("Failed to acquire bus access and/or talk to slave");
        return 1;
    }

    int remotepin = 27;
    if (wiringPiSetup() == -1) {
        printf("WiringPi setup failed!\n");
        return 1;
    }
    pinMode(remotepin, INPUT);
    pullUpDnControl(remotepin, PUD_UP);

    pinMode(TRACKING_PIN1, INPUT);
    pinMode(TRACKING_PIN2, INPUT);
    pinMode(TRACKING_PIN3, INPUT);
    pinMode(TRACKING_PIN4, INPUT);


    while (1) {                               //어떻게든 라인만 따라가도록(n). ㅏ나 ㅓ에선 회전. 꼭짓점도 회전. 십자가는 일단 우회전. 그리고 직진. 그림자 생각x. 바로 뒤집기
        if(digitalRead(remotepin) == LOW){
            Car_Stop();
            close(i2c_fd);
            break;
        }
        int trackValue1 = digitalRead(TRACKING_PIN1);
        int trackValue2 = digitalRead(TRACKING_PIN2);
        int trackValue3 = digitalRead(TRACKING_PIN3);
        int trackValue4 = digitalRead(TRACKING_PIN4);

        // 트래킹 핀 값을 바탕으로 자동차 제어 로직
        if (trackValue2 == 0 && trackValue3 == 0 && trackValue1 == 1 && trackValue4 == 1) {  //흰검검흰
            Car_Run(40, 40);
        }
        else if (trackValue2 == 0 && trackValue3 == 0 && trackValue1 == 0 && trackValue4 == 1) {  //검검검흰
                Car_Control(f, 100, b, 30);
                delay(650);

                Car_Control(b, 100, f, 100);
            delay(1500);
        }
        else if (trackValue2 == 0 && trackValue3 == 0 && trackValue1 == 1 && trackValue4 == 0) {  //흰검검검
                Car_Control(f, 100, b, 30);
                delay(650);
                Car_Control(b, 100, f, 100);
            delay(1500);
        }
        else if (trackValue2 == 0 && trackValue3 == 0 && trackValue1 == 0 && trackValue4 == 0) { //검검검검
                Car_Control(f, 100, b, 30);
                delay(650);
                Car_Control(b, 100, f, 100);
            delay(1500);

        }

        else if (trackValue2 == 0 && trackValue3 == 1 && trackValue1 == 1 && trackValue4 == 0) { //흰검흰검
            Car_Run(40,80);

        }
        else if (trackValue2 == 0 && trackValue3 == 1 && trackValue1 == 1 && trackValue4 == 1) {  //흰검흰흰
            Car_Run(40,80);

        }
        else if (trackValue2 == 0 && trackValue3 == 1 && trackValue1 == 0 && trackValue4 == 0) { //검검흰검
                Car_Control(f, 100, b, 30);
                delay(650);
                Car_Control(b, 100, f, 100);
            delay(1500);

        }
        else if(trackValue2 == 0 && trackValue3 == 1 && trackValue1 == 0 && trackValue4 == 1){  //검검흰흰
                Car_Control(f, 100, b, 30);
                delay(650);
                Car_Control(b, 100, f, 100);
            delay(1500);

        }
        else if (trackValue2 == 1 && trackValue3 == 0 && trackValue1 == 0 && trackValue4 == 0) {  //검흰검검
                Car_Control(f, 100, b, 30);
                delay(650);
                Car_Control(b, 100, f, 100);
            delay(1500);


        }
        else if(trackValue2 == 1 && trackValue3 == 0 && trackValue1 == 1 && trackValue4 == 0){  //흰흰검검
                Car_Control(f, 100, b, 30);
                delay(650);
                Car_Control(b, 100, f, 100);
            delay(1500);


        }
        else if(trackValue2 == 1 && trackValue3 == 0 && trackValue1 == 1 && trackValue4 == 1){  //흰흰검흰
            Car_Run(80,40);
        }
        else if(trackValue2 == 1 && trackValue3 == 0 && trackValue1 == 0 && trackValue4 == 1){  //검흰검흰
            Car_Run(80,40);
        }

        else if (trackValue2 == 1 && trackValue3 == 1 && trackValue1 == 0 && trackValue4 == 1) { //검흰흰흰
            Car_Run(0, 60);
            delay(400);
        }
        else if (trackValue2 == 1 && trackValue3 == 1 && trackValue1 == 1 && trackValue4 == 0) { //흰흰흰검
            Car_Run(60,0);
            delay(400);
        }

        else if(trackValue2 == 1 && trackValue3 == 1 && trackValue1 == 1 && trackValue4 == 1){  //흰흰흰흰
            Car_Run(40,40);
        }
        else{               //검흰흰검
            Car_Run(40,40);
        }
        delay(10);
    }


    // 모터 제어 예시
    // Car_Control(f, 150, b, 150); // 좌측 모터 전진, 우측 모터 후진

    // 적절한 딜레이 추가
    // sleep(5); // 5초 동안 동작

    // 모든 모터 정지
    // stopAllMotors(i2c_fd);

    // 서보 제어 예시
    // controlServo(i2c_fd, 1, 90); // 서보 1번을 90도 위치로 이동

    // I2C 장치 파일 닫기
    close(i2c_fd);

    return 0;
}
