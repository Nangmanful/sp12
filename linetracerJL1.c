#include <stdio.h>
#include <wiringPi.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <math.h>
#include <errno.h>

#define TRACKING_PIN1 7  
#define TRACKING_PIN2 0  
#define TRACKING_PIN3 3  
#define TRACKING_PIN4 2  

#define I2C_ADDR 0x16 

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


int main() {
    int pin =27;
    if (wiringPiSetup() == -1) {
        printf("WiringPi setup failed!\n");
        return 1;
    }
    pinMode(pin, INPUT);
    pullUpDnControl(pin, PUD_UP);

    pinMode(TRACKING_PIN1, INPUT);
    pinMode(TRACKING_PIN2, INPUT);
    pinMode(TRACKING_PIN3, INPUT);
    pinMode(TRACKING_PIN4, INPUT);

    i2c_init();
    int n = 0;
    int f = 0;
    int k = 0;
    int l = 0;
    while (1) {
        if(digitalRead(pin) == LOW){
            Car_Stop();    
            close(i2c_fd);   
        }
        int trackValue1 = digitalRead(TRACKING_PIN1);
        int trackValue2 = digitalRead(TRACKING_PIN2);
        int trackValue3 = digitalRead(TRACKING_PIN3);
        int trackValue4 = digitalRead(TRACKING_PIN4);

        // 각 트래킹 핀의 값을 출력
        printf("Track Values: %d %d %d %d\n", trackValue1, trackValue2, trackValue3, trackValue4);

        // 트래킹 핀 값을 바탕으로 자동차 제어 로직
        if (f == 3){
            Car_Stop();    
            delay(100);
            f = 0;
            l += 1;
        }
        if (trackValue2 == 0 && trackValue3 == 0 && trackValue1 == 1 && trackValue4 == 1) {
            n = 0;
            Car_Run(60, 60);
        } 
        else if (trackValue2 == 0 && trackValue3 == 0 && trackValue1 == 0 && trackValue4 == 1) {
            n = 0;
            Car_Stop();
            delay(50);
            Car_Left(200, 200);
            delay(280);
            Car_Run(70, 70);
            delay(120);
        }
        else if (trackValue2 == 1 && trackValue3 == 1 && trackValue1 == 0 && trackValue4 == 1) {
            n = 0;
            Car_Left(80, 80);
            delay(70);
            Car_Run(50, 50);
            delay(30);
        }
        else if (trackValue2 == 1 && trackValue3 == 1 && trackValue1 == 1 && trackValue4 == 0) {
            n = 0;
            Car_Right(80, 80);
            delay(70);
            Car_Run(50, 50);
            delay(30);
        }
        else if (trackValue2 == 0 && trackValue3 == 0 && trackValue1 == 1 && trackValue4 == 0) {
            n = 0;
            Car_Run(60, 60);
        }
        else if (trackValue2 == 0 && trackValue3 == 1 && trackValue1 == 1 && trackValue4 == 0) {
            n = 0;
            Car_Back(40, 40);
        }
        else if (trackValue2 == 0 && trackValue3 == 1 && trackValue1 == 1 && trackValue4 == 1) {
            n = 0;
            Car_Run(60, 60);
        }
        else if (trackValue2 == 0 && trackValue3 == 1 && trackValue1 == 0 && trackValue4 == 0) {
            n = 0;
            Car_Back(40, 40);
        } 
        else if (trackValue2 == 1 && trackValue3 == 0 && trackValue1 == 0 && trackValue4 == 0) {
            n = 0;
            Car_Back(40, 40);
        } 
        else if (trackValue2 == 0 && trackValue3 == 0 && trackValue1 == 0 && trackValue4 == 0) {
            n = 0;
            Car_Stop();
            delay(50);
            Car_Left(200, 200);
            delay(280);
            Car_Run(70, 70);
            delay(120);
        } 
        else if(trackValue2 == 0 && trackValue3 == 1 && trackValue1 == 0 && trackValue4 == 1){
            n = 0;
            Car_Stop();
            delay(50);
            Car_Left(200, 200);
            delay(280);
            Car_Run(70, 70);
            delay(120);
        }
        else if(trackValue2 == 1 && trackValue3 == 0 && trackValue1 == 1 && trackValue4 == 0){
            n = 0;
            Car_Run(60, 60);
        }
        else if(trackValue2 == 1 && trackValue3 == 0 && trackValue1 == 1 && trackValue4 == 1){
            n = 0;
            Car_Right(80, 80);
            delay(70);
            Car_Run(50, 50);
            delay(30);
        }
        else if(trackValue2 == 1 && trackValue3 == 0 && trackValue1 == 0 && trackValue4 == 1){
            n = 0;
            Car_Back(40,40);
        }
        else if (trackValue2 == 1 && trackValue3 == 1 && trackValue1 == 0 && trackValue4 == 0) {
            n = 0;
            Car_Left(80, 80);
            delay(70);
            Car_Run(50, 50);
            delay(30);
        }
        else if(trackValue2 == 1 && trackValue3 == 1 && trackValue1 == 1 && trackValue4 == 1){
            if (n == 10){
                Car_Stop();
                delay(100);
            }
            else if(n == 7){
                Car_Back(50, 50);   
                delay(30);
                f += 1;
            }
            else{
            Car_Run(40,40);
            n += 1;
            }
        }
        else{
            f = 0;
            n = 0;
            k += 1;
            Car_Back(40, 40);
        }
    }

    close(i2c_fd);
    return 0;
}
