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
    
    controlServo(i2c_fd, 1, 90);                  
    controlServo(i2c_fd, 2, 90);            

    while (1) {                               //perfect에서 십자가 우회전만 수정.   난관 봉착. 센서가 검검검흰을 인식한 순간 t라도 우로 꺾어버림... 아마 지금은 센서차이로 괜찮지만 저 turnright도 한 번 정도는 문제가 생길 수도 있을 듯. 같이 수정해야함.
        if(digitalRead(remotepin) == LOW){    //걍 회전에서의 오차도 줄일 겸 ㅏㅓ는 다 직진으로 하자. 그대신 Tㄱㄴ은 걸러주기로. turnright에서도 이런 문제 생기면 ㅏㅓ에서 직진하든가 센서 감도 조정하셈.
            Car_Stop();                        //갑자기 인식 안되는 부분 있음. 하지만 속도는 마분지 못 넘어서 줄일 수 x 일단 delay
            close(i2c_fd);
            break;
        }
        int trackValue1 = digitalRead(TRACKING_PIN1);
        int trackValue2 = digitalRead(TRACKING_PIN2);
        int trackValue3 = digitalRead(TRACKING_PIN3);
        int trackValue4 = digitalRead(TRACKING_PIN4);

        // 트래킹 핀 값을 바탕으로 자동차 제어 로직
        if (trackValue2 == 0 && trackValue3 == 0 && trackValue1 == 1 && trackValue4 == 1) {  //흰검검흰
            Car_Run(60, 60);
            delay(10);
        } 
        else if (trackValue2 == 0 && trackValue3 == 0 && trackValue1 == 0 && trackValue4 == 1) {  //검검검흰
            Car_Control(f, 5, f, 110);
            delay(600);
            trackValue2 = digitalRead(TRACKING_PIN2);
            trackValue3 = digitalRead(TRACKING_PIN3);
            while ((trackValue2 == 1) || (trackValue3 == 1)) {                   //멈추면서 하면 관성이 없어서 잘 못 돌 것 같기도 하지만 일단 ㅇㅋ. 못넘으면 나중에 속도를 올리든가
                Car_Control(f, 5, f, 80);
                delay(10);       //이거 지금은 잘 인식하는데 이 사이에 지나갈 수도 있음. 이것만 10으로 바꿔보자 우선. 바꿔서 나쁠 건x
                trackValue2 = digitalRead(TRACKING_PIN2);
                trackValue3 = digitalRead(TRACKING_PIN3);
            }
            Car_Control(f,90,f,40);
            delay(10);
        }
        else if (trackValue2 == 0 && trackValue3 == 0 && trackValue1 == 1 && trackValue4 == 0) {  //흰검검검
            Car_Run(60, 60);
            delay(10);
        }
        else if (trackValue2 == 0 && trackValue3 == 0 && trackValue1 == 0 && trackValue4 == 0) { //검검검검   여기만 수정
            Car_Control(f, 5, f, 110);
            delay(600);
            trackValue2 = digitalRead(TRACKING_PIN2);
            trackValue3 = digitalRead(TRACKING_PIN3);
            while ((trackValue2 == 1) || (trackValue3 == 1)) {
                Car_Control(f, 5, f, 80);
                delay(10);
                trackValue2 = digitalRead(TRACKING_PIN2);
                trackValue3 = digitalRead(TRACKING_PIN3);
            }
            Car_Control(f,90,f,40);
            delay(10);
        }

        else if (trackValue2 == 0 && trackValue3 == 1 && trackValue1 == 1 && trackValue4 == 0) { //흰검흰검
            Car_Run(40,90);
            delay(10);
        }
        else if (trackValue2 == 0 && trackValue3 == 1 && trackValue1 == 1 && trackValue4 == 1) {  //흰검흰흰
            Car_Run(40,90);
            delay(10);
        }
        else if (trackValue2 == 0 && trackValue3 == 1 && trackValue1 == 0 && trackValue4 == 0) { //검검흰검
            Car_Run(40,90);
            delay(10);
        } 
        else if(trackValue2 == 0 && trackValue3 == 1 && trackValue1 == 0 && trackValue4 == 1){  //검검흰흰
            Car_Run(40,90);
            delay(10);
        }

        else if (trackValue2 == 1 && trackValue3 == 0 && trackValue1 == 0 && trackValue4 == 0) {  //검흰검검
            Car_Run(90,40);
            delay(10);
        } 
        else if(trackValue2 == 1 && trackValue3 == 0 && trackValue1 == 1 && trackValue4 == 0){  //흰흰검검
            Car_Run(90,40);
            delay(10);
        }
        else if(trackValue2 == 1 && trackValue3 == 0 && trackValue1 == 1 && trackValue4 == 1){  //흰흰검흰
            Car_Run(90,40);
            delay(10);
        }
        else if(trackValue2 == 1 && trackValue3 == 0 && trackValue1 == 0 && trackValue4 == 1){  //검흰검흰
            Car_Run(90,40);
            delay(10);
        }

        else if (trackValue2 == 1 && trackValue3 == 1 && trackValue1 == 0 && trackValue4 == 1) { //검흰흰흰   이것들도
            Car_Left(40,90);
            delay(100);
        }
        else if (trackValue2 == 1 && trackValue3 == 1 && trackValue1 == 1 && trackValue4 == 0) { //흰흰흰검   마찬가지
            Car_Right(90, 40);
            delay(100);
        }
        
        else if(trackValue2 == 1 && trackValue3 == 1 && trackValue1 == 1 && trackValue4 == 1){  //흰흰흰흰    고칠수x 직진에 문제생김
            Car_Run(60,60);
            delay(10);
        }
        else{               //검흰흰검     나온적x
            Car_Run(60,60);
            delay(10);
        }
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



void controlServo(int fd, int servoNum, int angle) {
    if (servoNum < 1) servoNum = 1;
    if (servoNum > 4) servoNum = 4;
    if (angle < 0) angle = 0;
    if (angle > 180) angle = 180;

    unsigned char buffer[3];
    buffer[0] = REG_SERVO_CONTROL;
    buffer[1] = servoNum;
    buffer[2] = angle;

    if (write(fd, buffer, 3) != 3) {
        perror("Failed to write to the i2c bus");
    }
}
