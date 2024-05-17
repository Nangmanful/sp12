#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <stdio.h>

int main(void) {
    int fd;
    int devId = 0x16; // STM8의 실제 I2C 주소로 변경하세요

    if (wiringPiSetup() == -1) {
        fprintf(stderr, "Unable to start wiringPi: %s\n", strerror(errno));
        return 1;
    }

    fd = wiringPiI2CSetup(devId);
    if (fd < 0) {
        fprintf(stderr, "Unable to open I2C device: %s\n", strerror(errno));
        return 1;
    }

    // 모터를 구동하는 명령어 전송 (예제 명령어)
    if (wiringPiI2CWrite(fd, 0x01) < 0) {
        fprintf(stderr, "Failed to write to I2C device: %s\n", strerror(errno));
        return 1;
    }
    // 방향 설정 (예: 0x01은 앞으로, 0x02는 뒤로)
    wiringPiI2CWriteReg8(fd, 0x10, 0x01);  // 0x10은 방향 제어 레지스터 주소
    // 속도 설정 (예: 0-255 사이 값)
    wiringPiI2CWriteReg8(fd, 0x11, 120);  // 0x11은 속도 제어 레지스터 주소


    return 0;
}
