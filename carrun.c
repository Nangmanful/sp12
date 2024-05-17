#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <math.h>
#include <errno.h>

#define I2C_ADDR 0x16  // STM8의 I2C 주소 (예시)

// I2C 파일 디스크립터
int i2c_fd;

void i2c_init() {
    if ((i2c_fd = open("/dev/i2c-1", O_RDWR)) < 0) {
        perror("Failed to open the i2c bus");
        exit(1);
    }
    if (ioctl(i2c_fd, I2C_SLAVE, I2C_ADDR) < 0) {
        perror("Failed to acquire bus access and/or talk to slave");
        exit(1);
    }
}

void write_u8(int reg, int data) {
    unsigned char buffer[2];
    buffer[0] = reg;
    buffer[1] = data;
    if (write(i2c_fd, buffer, 2) != 2) {
        perror("Failed to write to the i2c bus");
    }
}

void write_array(int reg, unsigned char *data, int length) {
    unsigned char buffer[length + 1];
    buffer[0] = reg;
    for (int i = 0; i < length; i++) {
        buffer[i + 1] = data[i];
    }
    if (write(i2c_fd, buffer, length + 1) != length + 1) {
        perror("Failed to write to the i2c bus");
    }
}

void Ctrl_Car(int l_dir, int l_speed, int r_dir, int r_speed) {
    int reg = 0x01;
    unsigned char data[4] = {l_dir, l_speed, r_dir, r_speed};
    write_array(reg, data, 4);
}

void Control_Car(int speed1, int speed2) {
    int dir1 = (speed1 < 0) ? 0 : 1;
    int dir2 = (speed2 < 0) ? 0 : 1;
    Ctrl_Car(dir1, abs(speed1), dir2, abs(speed2));
}

void Car_Run(int speed1, int speed2) {
    Ctrl_Car(1, speed1, 1, speed2);
}

void Car_Stop() {
    int reg = 0x02;
    write_u8(reg, 0x00);
}

void Car_Back(int speed1, int speed2) {
    Ctrl_Car(0, speed1, 0, speed2);
}

void Car_Left(int speed1, int speed2) {
    Ctrl_Car(0, speed1, 1, speed2);
}

void Car_Right(int speed1, int speed2) {
    Ctrl_Car(1, speed1, 0, speed2);
}

void Car_Spin_Left(int speed1, int speed2) {
    Ctrl_Car(0, speed1, 1, speed2);
}

void Car_Spin_Right(int speed1, int speed2) {
    Ctrl_Car(1, speed1, 0, speed2);
}

void Ctrl_Servo(int id, int angle) {
    if (angle < 0) angle = 0;
    if (angle > 180) angle = 180;
    int reg = 0x03;
    unsigned char data[2] = {id, angle};
    write_array(reg, data, 2);
}

int main() {
    // I2C 초기화
    i2c_init();

    // 기본 동작 테스트
    Car_Run(255, 255);  // 전진
    sleep(1);
    Car_Stop();
    sleep(1);

    Car_Back(255, 255);  // 후진
    sleep(1);
    Car_Stop();
    sleep(1);

    Car_Left(255, 255);  // 좌회전
    sleep(1);
    Car_Stop();
    sleep(1);

    Car_Right(255, 255);  // 우회전
    sleep(1);
    Car_Stop();

    // I2C 종료
    close(i2c_fd);

    return 0;
}
