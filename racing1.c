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

    int present_x = -1;
    int present_y = -1;

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
        }
        else {
            Car_Stop();
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

            game_state.col = index_x;
            game_state.row = index_y;
            game_state.action = move;

            if (send(sock, &game_state, sizeof(ClientAction), 0) < 0) {
                perror("Send failed\0");
                // return 1;
            }
        }
        /*
            algorithm 구현(linetracer)
        */

        // Finding future_x & future_y

        int past_x = present_x, past_y = present_y;
        present_x = index_x;
        present_y = index_y;

        int left_x = present_x, left_y = present_y-1;
        int up_x = present_x-1, up_y = present_y;
        int right_x = present_x, right_y = present_y+1;
        int down_x = present_x+1, down_y = present_y;

        Node node_r, node_l, node_u, node_d, best_node;
        Item r_item, l_item, u_item, d_item;

        if(left_x>=0 && left_x<=4 && left_y>=0 && left_y<=4){
            node_l = info.map[left_x][left_y];
            l_item = node_l.item;}
        else{l_item.score = -100;}
        if(right_x>=0 && right_x<=4 && right_y>=0 && right_y<=4){
            node_r = info.map[right_x][right_y];
            r_item = node_r.item;}
        else{r_item.score = -100;}
        if(up_x>=0 && up_x<=4 && up_y>=0 && up_y<=4){
            node_u = info.map[up_x][up_y];
            u_item = node_u.item;}
        else{u_item.score = -100;}
        if(down_x>=0 && down_x<=4 && down_y>=0 && down_y<=4){
            node_d = info.map[down_x][down_y];
            d_item = node_d.item;}
        else{d_item.score = -100;}

        if(l_item.score>r_item.score){best_node = node_l;}
        else{best_node = node_r;}
        if(u_item.score>best_item.score){best_node = node_u;}
        if(d_item.score>best_item.score){best_node = node_d;}
        
        int future_x = best_node.col, future_y = best_node.row;


        // Greedy algorithm
        
        char run_direct;

        int fpp_x = future_x-past_x;
        int fpp_y = future_y-past_y;
        int fp_x = future_x-present_x;
        int fp_y = future_y-present_y;
        int pp_x = present_x-past_x;
        int pp_y = present_y-past_y;

        if(past_x==-1 && past_y==-1){
            if(present_x==0 && present_y==0){
                if(fp_x==0 && fp_y==1){run_direct = 'f';}
                else if(fp_x==1 && fp_y==0){run_direct = 'r';}
            else if(present_x==4 && present_y==4){
                if(fp_x==0 && fp_y==-1){run_direct = 'f';}
                else if(fp_x==-1 && fp_y==0){run_direct = 'r';}
            }
            }
        }
        else{
        if(fpp_x==0 && fpp_y==0){run_direct = 'b';}
        else if(abs(fpp_x)==2 && fpp_y==0){run_direct = 'f';}
        else if(fpp_x==0 && abs(fpp_y)==2){run_direct = 'f';}
        else if(pp_x==1 && pp_y==0){
            if(fp_x==0 && fp_y==-1){run_direct = 'r';}
            else if(fp_x=0 && fp_y==1){run_direct = 'l';}
        }
        else if(pp_x==-1 && pp_y==0){
            if(fp_x==0 && fp_y==-1){run_direct = 'l';}
            else if(fp_x==0 && fp_y==1){run_direct = 'r';}
        }
        else if(pp_x==0 && pp_y==1){
            if(fp_x==-1 && fp_y==0){run_direct = 'l';}
            else if(fp_x==1 && fp_y==0){run_direct = 'r';}
        }
        else if(pp_x==0 && pp_y==-1){
            if(fp_x==-1 && fp_y==0){run_direct = 'r';}
            else if(fp_x==1 && fp_y==0){run_direct = 'l';}
        }
        }

        // linetracer
        char* qrvalue = qrrecognition();
        if(index_x ==0 && index_y == 0){
            direct == 'f';
        }
        else if(index_x ==0 && index_y == 1){
            direct == 'l';
        }
        else if(index_x ==1 && index_y == 0){
            direct == 'r';
        }
        else if(index_x == 1 && index_y == 1){
            direct == 'b';
        }
        else if(index_x == 2 && index_y == 2){
            direct == 'n';
        }
        if(direct == 'l'){ // left
            while(strcmp(qrvalue, "77") == 0 || strcmp(qrvalue, index) == 0){
                int trackvalue1 = digitalRead(TRACKING_PIN1);
                int trackvalue2 = digitalRead(TRACKING_PIN2);
                int trackvalue3 = digitalRead(TRACKING_PIN3);
                int trackvalue4 = digitalRead(TRACKING_PIN4);
                    if (trackvalue2 == 0 && trackvalue3 == 0 && trackvalue1 == 1 && trackvalue4 == 1) {
                    Car_Run(40, 40);
                    } 
                    else if (trackvalue2 == 0 && trackvalue3 == 0 && trackvalue1 == 0 && trackvalue4 == 1) {
                        Car_Run(40, 40);
                    }
                    else if (trackvalue2 == 0 && trackvalue3 == 0 && trackvalue1 == 1 && trackvalue4 == 0) {
            
                        Car_Run(40, 40);
                    }
                    else if (trackvalue2 == 0 && trackvalue3 == 1 && trackvalue1 == 1 && trackvalue4 == 0) {
                    
                        Car_Left(100, 100);
                        delay(80);
                        Car_Run(50, 50);
                        delay(30);
                        Car_Left(100, 100);
                        delay(80);
                    }
                else if (trackvalue2 == 0 && trackvalue3 == 1 && trackvalue1 == 1 && trackvalue4 == 1) {
                    Car_Left(100, 100);
                    delay(80);
                    Car_Run(50, 50);
                    delay(30);
                    Car_Left(100, 100);
                    delay(80);
                }
                else if (trackvalue2 == 0 && trackvalue3 == 1 && trackvalue1 == 0 && trackvalue4 == 0) {
                    Car_Left(100, 100);
                    delay(80);
                    Car_Run(50, 50);
                    delay(30);
                    Car_Left(100, 100);
                    delay(80);
                } 
                else if (trackvalue2 == 1 && trackvalue3 == 0 && trackvalue1 == 0 && trackvalue4 == 0) {
                    Car_Back(60, 60);
                } 
                else if (trackvalue2 == 0 && trackvalue3 == 0 && trackvalue1 == 0 && trackvalue4 == 0) {
                    Car_Run(40, 40);
                } 
                else if(trackvalue2 == 0 && trackvalue3 == 1 && trackvalue1 == 0 && trackvalue4 == 1){
                    Car_Left(100, 100);
                    delay(80);
                    Car_Run(50, 50);
                    delay(30);
                    Car_Left(100, 100);
                    delay(80);
                }
                else if(trackvalue2 == 1 && trackvalue3 == 0 && trackvalue1 == 1 && trackvalue4 == 0){
                    Car_Back(60, 60);
                }
                else if(trackvalue2 == 1 && trackvalue3 == 0 && trackvalue1 == 1 && trackvalue4 == 1){
                    Car_Back(60, 60);
                }
                else if(trackvalue2 == 1 && trackvalue3 == 0 && trackvalue1 == 0 && trackvalue4 == 1){
                    Car_Back(60, 60);
                }
                else if(trackvalue2 == 1 && trackvalue3 == 1 && trackvalue1 == 1 && trackvalue4 == 1){
                    Car_Back(60, 60);
                }
                else{
                    Car_Back(30, 30);
                } 
            delay(10);
            qrvalue = qrrecognition();
            }
    }
    else if(direct == 'r'){
        while(strcmp(qrvalue, "77") == 0 || strcmp(qrvalue, index) == 0){
        int trackvalue1 = digitalRead(TRACKING_PIN1);
        int trackvalue2 = digitalRead(TRACKING_PIN2);
        int trackvalue3 = digitalRead(TRACKING_PIN3);
        int trackvalue4 = digitalRead(TRACKING_PIN4);
        if (trackvalue2 == 0 && trackvalue3 == 0 && trackvalue1 == 1 && trackvalue4 == 1) {
                    Car_Run(40, 40);
        } 
        else if (trackvalue2 == 0 && trackvalue3 == 0 && trackvalue1 == 0 && trackvalue4 == 1) {
                        Car_Run(40, 40);
        }
                    else if (trackvalue2 == 0 && trackvalue3 == 0 && trackvalue1 == 1 && trackvalue4 == 0) {
                        Car_Run(40, 40);
                    }
                    else if (trackvalue2 == 0 && trackvalue3 == 1 && trackvalue1 == 1 && trackvalue4 == 0) {
                        Car_Back(60, 60);
                    }
                else if (trackvalue2 == 0 && trackvalue3 == 1 && trackvalue1 == 1 && trackvalue4 == 1) {
                    Car_Back(60, 60);
                }
                else if (trackvalue2 == 0 && trackvalue3 == 1 && trackvalue1 == 0 && trackvalue4 == 0) {
                    Car_Back(60, 60);
                } 
                else if (trackvalue2 == 1 && trackvalue3 == 0 && trackvalue1 == 0 && trackvalue4 == 0) {
                    Car_Right(100, 100);
                    delay(80);
                    Car_Run(50, 50);
                    delay(30);
                    Car_Right(100, 100);
                    delay(80);
                } 
                else if (trackvalue2 == 0 && trackvalue3 == 0 && trackvalue1 == 0 && trackvalue4 == 0) {
                    Car_Run(40, 40);
                } 
                else if(trackvalue2 == 0 && trackvalue3 == 1 && trackvalue1 == 0 && trackvalue4 == 1){
                    Car_Back(60, 60);
                }
                else if(trackvalue2 == 1 && trackvalue3 == 0 && trackvalue1 == 1 && trackvalue4 == 0){
                
                    Car_Right(100, 100);
                    delay(80);
                    Car_Run(50, 50);
                    delay(30);
                    Car_Right(100, 100);
                    delay(80);
                }
                else if(trackvalue2 == 1 && trackvalue3 == 0 && trackvalue1 == 1 && trackvalue4 == 1){
                    
                    Car_Right(100, 100);
                    delay(80);
                    Car_Run(50, 50);
                    delay(30);
                    Car_Right(100, 100);
                    delay(80);
                }
                else if(trackvalue2 == 1 && trackvalue3 == 0 && trackvalue1 == 0 && trackvalue4 == 1){
                    Car_Right(100, 100);
                    delay(80);
                    Car_Run(50, 50);
                    delay(30);
                    Car_Right(100, 100);
                    delay(80);
                }
                else if(trackvalue2 == 1 && trackvalue3 == 1 && trackvalue1 == 1 && trackvalue4 == 1){
                    Car_Back(60, 60);
                }
                else{
                    Car_Back(30, 30);
                } 
            delay(10);
            qrvalue = qrrecognition();
            }
    }
    else if(direct == 'f'){
        while(strcmp(qrvalue, "77") == 0 || strcmp(qrvalue, index) == 0){
        int trackvalue1 = digitalRead(TRACKING_PIN1);
        int trackvalue2 = digitalRead(TRACKING_PIN2);
        int trackvalue3 = digitalRead(TRACKING_PIN3);
        int trackvalue4 = digitalRead(TRACKING_PIN4);
        if (trackvalue2 == 0 && trackvalue3 == 0 && trackvalue1 == 1 && trackvalue4 == 1) {
                    Car_Run(40, 40);
        }
        else{
                    Car_Run(40, 40);
        } 
            delay(10);
            qrvalue = qrrecognition();
        }
    }

    else if(direct == 'b'){
       while(strcmp(qrvalue, "77") == 0 || strcmp(qrvalue, index) == 0){
        int trackvalue1 = digitalRead(TRACKING_PIN1);
        int trackvalue2 = digitalRead(TRACKING_PIN2);
        int trackvalue3 = digitalRead(TRACKING_PIN3);
        int trackvalue4 = digitalRead(TRACKING_PIN4);
        if (trackvalue2 == 0 && trackvalue3 == 0 && trackvalue1 == 1 && trackvalue4 == 1) {
                    Car_Back(40, 40);
        } 
        else{
                    Car_Back(30, 30);
        } 
            delay(10);
            qrvalue = qrrecognition();

        }
    }
    else if(direct == 'n'){
        Car_Stop();
    }
    }
    close(sock);    
    return 0;
}
