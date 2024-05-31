#include <stdio.h>
#include "qrrecognition.h" //function qrrecogntion();
#include <arpa/inet.h>
#include "server.h"
#include <string.h>
#include <wiringPi.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <math.h>
#include <errno.h>

#include <sys/socket.h>

#define TRACKING_PIN1 7  
#define TRACKING_PIN2 0  
#define TRACKING_PIN3 3  
#define TRACKING_PIN4 2  

#define I2C_ADDR 0x16 

#define PORT 8080
#define BUF_SIZE 1024

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

void write_array(int reg, unsigned char* data, int length) {
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
    unsigned char data[4] = { l_dir, l_speed, r_dir, r_speed };
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
    int pin = 27;
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
    int sock;
    struct sockaddr_in server_addr;

    // 소켓 생성
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("Could not create socket");
        // return 1;
    }

    // 서버 주소 구조체 설정
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(PORT);

    // 서버에 연결
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connect failed");
        // return 1;
    }

    printf("Connected to server\0");
    while (1) {
        char direct = 'r';
        int index_x; // our
        int index_y; // our
        char* index;
        DGIST info;
        client_info player_info;
        Node node;
        Item now_item;
        ClientAction game_state;

        printf("qr시작\0");
        index = qrrecognition();
        printf("qr끝\0");

        if (strcmp(index, "77") == 0) { // no qr recognition
            printf("no qr\0");
            Car_Run(55, 55);
        }
        else {
            index_x = index[0] - '0'; // ASCII 값을 실제 숫자로 변환
            index_y = index[1] - '0'; // ASCII 값을 실제 숫자로 변환
            printf("%d %d\n", index_x, index_y);
            printf("데이터 송수신 시작\0");

            //서버 통신

            // 서버로부터 데이터 수신
            if (recv(sock, &info, sizeof(DGIST), 0) < 0) {
                perror("Recv failed\0");
                // return 1;
            }

            // 버퍼를 구조체로 복사
            player_info = info.players[0];
            node = info.map[index_x][index_y];
            now_item = node.item;
            int item_state;
            item_state = now_item.status;

            game_state.row = index_x;
            game_state.col = index_y;
            game_state.action = move;

            //너무 늦으면 보내기 전에 가야할 곳 계산 후 방향 정하기

            if (send(sock, &game_state, sizeof(ClientAction), 0) < 0) {
                perror("Send failed\0");
                // return 1;
            }

            if (index == "22") {
                Car_Stop();
                break;
            }

            //계산 후 direct 정해짐
            if (index == "00") {      //direct == f
                Car_Run(60, 60);       //일단 라인벗어나지 않는 거 가정. qr지나갈 때까지 달리기
                delay(20);
                while (qrrecognition() == "77") {
                    int tv1 = digitalRead(TRACKING_PIN1);
                    int tv2 = digitalRead(TRACKING_PIN2);
                    int tv3 = digitalRead(TRACKING_PIN3);
                    int tv4 = digitalRead(TRACKING_PIN4);
                    if (tv2 == 0 && tv3 == 0) {
                        Car_Run(40, 40);
                    }
                    else if ((tv2 == 1) && (tv3 == 0)) {
                        Car_Right(40, 40);
                    }
                    else if ((tv2 == 0) && (tv3 == 1)) {
                        Car_Left(40, 40);
                    }
                    else {
                        Car_Back(20, 20);
                    }
                    delay(10);
                }
                break;
            }
            else if (index == "01") {  //direct == left
                int tv1 = 2;
                int tv2 = 2;
                int tv3 = 2;
                int tv4 = 2;
                while ((tv1 == 0) || (tv4 == 0)) {
                    int tv1 = digitalRead(TRACKING_PIN1);
                    int tv2 = digitalRead(TRACKING_PIN2);
                    int tv3 = digitalRead(TRACKING_PIN3);
                    int tv4 = digitalRead(TRACKING_PIN4);
                    Car_Run(40, 40);       //일단 라인벗어나지 않는 거 가정. 교차점까지 달리기
                    delay(10);
                }
                Car_Left(40, 40); //앞으로 가면서 돌면x
                delay(10);
                while ((tv2 == 0) && (tv3 == 1)) {  //검흰 나올때까지
                    int tv1 = digitalRead(TRACKING_PIN1);
                    int tv2 = digitalRead(TRACKING_PIN2);
                    int tv3 = digitalRead(TRACKING_PIN3);
                    int tv4 = digitalRead(TRACKING_PIN4);
                    Car_Run(40, 40);
                    delay(10);
                }
                while (qrrecognition() == "77") {         //라인트레이싱
                    int tv1 = digitalRead(TRACKING_PIN1);
                    int tv2 = digitalRead(TRACKING_PIN2);
                    int tv3 = digitalRead(TRACKING_PIN3);
                    int tv4 = digitalRead(TRACKING_PIN4);
                    if (tv2 == 0 && tv3 == 0) {
                        Car_Run(40, 40);
                    }
                    else if ((tv2 == 1)&& (tv3 == 0)) {
                        Car_Right(40, 40);
                    }
                    else if ((tv2 == 0) && (tv3 == 1)) {
                        Car_Left(40, 40);
                    }
                    else {
                        Car_Back(20, 20);
                    }
                    delay(10);
                }
                break;
            }
            else if (index == "10") {  //direct == right
                int tv1 = 2;
                int tv2 = 2;
                int tv3 = 2;
                int tv4 = 2;
                while ((tv1 == 0) || (tv4 == 0)) {
                    int tv1 = digitalRead(TRACKING_PIN1);
                    int tv2 = digitalRead(TRACKING_PIN2);
                    int tv3 = digitalRead(TRACKING_PIN3);
                    int tv4 = digitalRead(TRACKING_PIN4);
                    Car_Run(40, 40);       //일단 라인벗어나지 않는 거 가정. 교차점까지 달리기
                    delay(10);
                }
                Car_Right(40, 40); //앞으로 가면서 돌면x
                delay(10);
                while ((tv2 == 1) && (tv3 == 0)) {  //흰검 나올때까지
                    int tv1 = digitalRead(TRACKING_PIN1);
                    int tv2 = digitalRead(TRACKING_PIN2);
                    int tv3 = digitalRead(TRACKING_PIN3);
                    int tv4 = digitalRead(TRACKING_PIN4);
                    Car_Run(40, 40);
                    delay(10);
                }
                while (qrrecognition() == "77") {         //라인트레이싱
                    int tv1 = digitalRead(TRACKING_PIN1);
                    int tv2 = digitalRead(TRACKING_PIN2);
                    int tv3 = digitalRead(TRACKING_PIN3);
                    int tv4 = digitalRead(TRACKING_PIN4);
                    if (tv2 == 0 && tv3 == 0) {
                        Car_Run(40, 40);
                    }
                    else if ((tv2 == 1) && (tv3 == 0)) {
                        Car_Right(40, 40);
                    }
                    else if ((tv2 == 0) && (tv3 == 1)) {
                        Car_Left(40, 40);
                    }
                    else {
                        Car_Back(20, 20);
                    }
                    delay(10);
                }
                break;
            }

            else if (index == "11") { //direct == b
                Car_Back(100, 100);
                delay(100);
                break;
            }

        }
    }
    close(sock);
    return 0;
}
