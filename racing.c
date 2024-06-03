#include <stdio.h>
#include "qrrecognition.h"
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
#include <time.h>
#include <pthread.h>
#include <sys/socket.h>

#define TRACKING_PIN1 7  
#define TRACKING_PIN2 0  
#define TRACKING_PIN3 3  
#define TRACKING_PIN4 2  

#define I2C_ADDR 0x16 


DGIST global_info;
volatile char qrCodeData[256] = "77";
pthread_mutex_t qrCodeMutex = PTHREAD_MUTEX_INITIALIZER;
int i2c_fd;


char* copy_string(const char* src) {
    // 원본 문자열의 길이를 계산
    size_t len = strlen(src);
    
    // 문자열을 저장할 새로운 메모리 할당
    char* dest = (char*)malloc((len + 1) * sizeof(char)); // +1은 null 종료 문자('\0')를 위한 공간
    
    // 메모리 할당이 성공했는지 확인
    if (dest == NULL) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }
    
    // 원본 문자열을 새로 할당된 메모리로 복사
    strcpy(dest, src);
    
    // 새로 복사된 문자열의 포인터 반환
    return dest;
}

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

// 레지스터 정의
#define REG_MOTOR_CONTROL 0x01
#define REG_STOP_ALL 0x02
#define REG_SERVO_CONTROL 0x03

// 모터 방향 정의
#define b 0
#define f 1


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

typedef struct {
    int sock;
} thread_args;

void* handle_info(void* arg) {
	thread_args *args = (thread_args*) arg;
    	int sock = args->sock;
    	int n;

    	while ((n = recv(sock, &global_info, sizeof(DGIST), 0)) > 0) {
		printf("Received data from server\n");
	    	fflush(stdout);
    	}

    	if (n == 0) {
		printf("Connection closed by server\n");
	}
    	else if (n < 0) {
	    	perror("recv failed");
    	}
	fflush(stdout);
    	close(sock);
    	free(arg);
    	return NULL;
}

void* qrRecognitionThread() {
    	while (1) {
		const char* data = qrrecognition();
        	pthread_mutex_lock(&qrCodeMutex);
        	strncpy((char*)qrCodeData, data, sizeof(qrCodeData) - 1);
        	pthread_mutex_unlock(&qrCodeMutex);
	    	printf("qrthread action\n");
	    	fflush(stdout);
    	}
    	return NULL;
}


int main(int argc, char *argv[]) {
	controlServo(i2c_fd, 1, 90);    //카메라 컨트롤 상하(잘못건드리면찢어짐)
    	controlServo(i2c_fd, 2, 90);    //좌우(90이 가운데)
	
	printf("main 시작\n");
	fflush(stdout);
	int present_x;
	int present_y;
	char run_direct;
	int port = atoi(argv[1]);
	int char_num = atoi(argv[2]);
	const char* ip_address = argv[3];
	int enemy_num;
    	if(char_num == 0){
        	enemy_num = 1;
		present_x = -1;
		present_y = 0;
    	}
    	else{
        	enemy_num = 0;
		present_x = 5;
		present_y = 4;
    	}
    	int pin = 27;
    
    	if (wiringPiSetup() == -1) {
        	printf("WiringPi setup failed!\n");
        	return 1;
    	}
	printf("real 시작\n");
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
    	server_addr.sin_addr.s_addr = inet_addr(ip_address);
    	server_addr.sin_port = htons(port);

            // 서버에 연결
    	if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
                perror("Connect failed");
                // return 1;
    	}	

    	printf("Connected to server\0");
	fflush(stdout);
    	pthread_t recv_thread;
    	thread_args *args = malloc(sizeof(thread_args));
    	args->sock = sock;

    	if (pthread_create(&recv_thread, NULL, handle_info, args) != 0) {
        	perror("Could not create thread");
        	close(sock);
        	exit(EXIT_FAILURE);
    	}//server thread
	pthread_t threadId;
    	if (pthread_create(&threadId, NULL, qrRecognitionThread, NULL) != 0) {
        	perror("Failed to create QR recognition thread");
        	return 1;
    	} // qr thread
	char* index ="77";
        DGIST info;
	info = global_info;
	while (1) {
        	char direct = 'r';
        	int index_x; // our
        	int index_y; // our
        	client_info player_info;
        	client_info enemy_info;
        	Node node;
        	Item now_item;
		int count = 1;
		int check = 0;
        	ClientAction game_state;
        	printf("qr시작\n");
		fflush(stdout);
        	pthread_mutex_lock(&qrCodeMutex);
		printf("index : %s, qrcode : %s\n", index, qrCodeData);
		fflush(stdout);
        	if (strcmp(qrCodeData, "77") != 0 && strcmp(index, qrCodeData) != 0) {
            		printf("QR 코드 데이터: %s\n", qrCodeData);
			fflush(stdout);
        		index = copy_string(qrCodeData);
			count = 0;
			check = 1;
		}
		printf("unlock, count: %d\n", count);
		fflush(stdout);
        	pthread_mutex_unlock(&qrCodeMutex);
		printf("qr끝 \n");
		fflush(stdout);
        	if (count) { // no qr recognition
                	printf("no qr\n");
			fflush(stdout);
        	}
        	else {
	        	Car_Stop();
			printf("else start\n");
			fflush(stdout);
            		index_x = index[0] - '0'; // ASCII 값을 실제 숫자로 변환
            		index_y = index[1] - '0'; // ASCII 값을 실제 숫자로 변환
            		printf("%d %d\0", index_x, index_y);
			fflush(stdout);
            		printf("데이터 송수신 시작\n");

        	//서버 통신

            	// 서버로부터 데이터 수신
			info = global_info;
			printf("수신\n");
			fflush(stdout);
            	// 버퍼를 구조체로 복사
	        	player_info = info.players[char_num];
            		enemy_info = info.players[enemy_num];
            		int ex;
            		int ey;
           		ex = enemy_info.col;
            		ey = enemy_info.row;
            		node = info.map[index_x][index_y];
		        now_item = node.item;
		        int item_state;
		        item_state = now_item.status;
			fflush(stdout);
		
		        //bomb
		  	int eleft_x = ex;
		        int eleft_y = ey-1;
		        int eup_x = ex-1;
			int eup_y = ey;
		        int eright_x = ex;
		        int eright_y = ey+1;
		        int edown_x = ex+1;
			int edown_y = ey;
		    
		        Node enode_r, enode_l, enode_u, enode_d, ebest_node;
		        Item er_item, el_item, eu_item, ed_item;
		    
		        if(eleft_x>=0 && eleft_x<=4 && eleft_y>=0 && eleft_y<=4){
		        	enode_l = info.map[eleft_x][eleft_y];
		                el_item = enode_l.item;
		        	if(el_item.status == 0){
					el_item.score =0;
			        }
			        else if(el_item.status ==2){
				        el_item.score = -8;
		    	    	}
		        }
		        else{el_item.score = -100;}
		        if(eright_x>=0 && eright_x<=4 && eright_y>=0 && eright_y<=4){
		                enode_r = info.map[eright_x][eright_y];
		                er_item = enode_r.item;
		            	if(er_item.status == 0){
				        er_item.score =0;
			        }
			        else if(er_item.status ==2){
				        er_item.score = -8;
			        }
		        }
		        else{er_item.score = -100;}
		        if(eup_x>=0 && eup_x<=4 && eup_y>=0 && eup_y<=4){
		        	enode_u = info.map[eup_x][eup_y];
		                eu_item = enode_u.item;
		            	if(eu_item.status == 0){
				        eu_item.score =0;
			        }
			        else if(eu_item.status ==2){
					eu_item.score = -8;
			        }    
		        }
		        else{eu_item.score = -100;}
		        if(edown_x>=0 && edown_x<=4 && edown_y>=0 && edown_y<=4){
		                enode_d = info.map[edown_x][edown_y];
		                ed_item = enode_d.item;
		            	if(ed_item.status == 0){
			        	ed_item.score =0;
			        }
			        else if(ed_item.status ==2){
				        ed_item.score = -8;
			        }
		        }
		        else{ed_item.score = -100;}
    			fflush(stdout);

		        // if(el_item.score>er_item.score){ebest_node = enode_l;}
		        // else{ebest_node = enode_r;}
			
		        // Item ebest_item = ebest_node.item;
			
		        // if(eu_item.score>ebest_item.score){ebest_node = enode_u;}
		        // if(ed_item.score>ebest_item.score){ebest_node = enode_d;}
		    
		        // int efuture_x = ebest_node.row;
		        // int efuture_y = ebest_node.col;
		        // //bomb check


			int efuture_x = -1;
		        int efuture_y = -1;
			Item ebest_item = ebest_node.item;
			
			if(el_item.score>er_item.score){efuture_x = eleft_x; efuture_y = eleft_y; ebest_item.score = el_item.score;}
		        else{efuture_x = eright_x; efuture_y = eright_y; ebest_item.score = er_item.score;}
			
		        if(eu_item.score>ebest_item.score){
				efuture_x = eup_x; efuture_y = eup_y; ebest_item.score = eu_item.score;
			}
		        if(ed_item.score>ebest_item.score){
				efuture_x = edown_x; efuture_y = edown_y; ebest_item.score = ed_item.score;
			}
		
		        game_state.row = index_x;
		        game_state.col = index_y;
			if ((index_x == 0 && index_y ==0) || (index_x == 0 && index_y ==4) || (index_x == 4 && index_y ==0) || (index_x == 4 && index_y ==4)){
		                game_state.action = setBomb;
			}
			else if(ex == -1 && ey == -1){
				game_state.action = move;
			}
		        else if(index_x == efuture_x && index_y == efuture_y){
		                game_state.action = setBomb;
		        }
		        else{
		                game_state.action = move;
		        }
			printf("send time\n");
			fflush(stdout);
		        if (send(sock, &game_state, sizeof(ClientAction), 0) < 0) {
		                perror("Send failed\0");
		                // return 1;
		        }
			printf("send complete");
			fflush(stdout);
        
        		/*
            		algorithm 구현(linetracer)
        		*/
			delay(300);
			info = global_info;

        		// Finding future_x & future_y
		        int past_x = present_x;
		        int past_y = present_y;
		        present_x = index_x;
		        present_y = index_y;
			printf("past x,y : %d, %d\n", past_x, past_y);
			printf("present x,y : %d, %d\n", present_x, present_y);
			fflush(stdout);
 		        int left_x = present_x;
		        int left_y = present_y-1;
		        int up_x = present_x-1;
		        int up_y = present_y;
		        int right_x = present_x;
		        int right_y = present_y+1;
		        int down_x = present_x+1;
		        int down_y = present_y;
		
		        Node node_r, node_l, node_u, node_d, best_node;
		        Item r_item, l_item, u_item, d_item;
		
		        if(left_x>=0 && left_x<=4 && left_y>=0 && left_y<=4){
		        	node_l = info.map[left_x][left_y];
		        	l_item = node_l.item;
		       		if(l_item.status == 0){
					l_item.score =0;
				}	
				else if(l_item.status ==2){
					l_item.score = -8;
				}
			}
		        else{l_item.score = -100;}
			
		        if(right_x>=0 && right_x<=4 && right_y>=0 && right_y<=4){
		        	node_r = info.map[right_x][right_y];
		            	r_item = node_r.item;
			        if(r_item.status == 0){
			        	r_item.score =0;
		        	}
			        else if(r_item.status ==2){
				        r_item.score = -8;
			        }
			}
		        else{r_item.score = -100;}
			
		        if(up_x>=0 && up_x<=4 && up_y>=0 && up_y<=4){
		            	node_u = info.map[up_x][up_y];
		            	u_item = node_u.item;
			        if(u_item.status == 0){
				        u_item.score =0;
			        }
			        else if(er_item.status ==2){
				        u_item.score = -8;
			        }
		        }
		        else{u_item.score = -100;}
			
		        if(down_x>=0 && down_x<=4 && down_y>=0 && down_y<=4){
		        	node_d = info.map[down_x][down_y];
		            	d_item = node_d.item;
			        if(d_item.status == 0){
				        d_item.score =0;
			        }
			        else if(d_item.status ==2){
				        d_item.score = -8;
			        }
		        }
		        else{d_item.score = -100;}

			printf("up, down, left, right : %d, %d, %d, %d\n", u_item.score, d_item.score, l_item.score, r_item.score); 

			int future_x;
		        int future_y;
			Item best_item = best_node.item;
			
			if(l_item.score>r_item.score){future_x = left_x; future_y = left_y; best_item.score = l_item.score;}
		        else{future_x = right_x; future_y = right_y; best_item.score = r_item.score;}
			
		        if(u_item.score>best_item.score){
				future_x = up_x; future_y = up_y; best_item.score = u_item.score;
			}
		        if(d_item.score>best_item.score){
				future_x = down_x; future_y = down_y; best_item.score = d_item.score;
			}
		
			printf("x:%d, y:%d \n", future_x, future_y); 
        		// Greedy algorithm
        
			fflush(stdout);

        		int fpp_x = future_x-past_x;
        		int fpp_y = future_y-past_y;
		        int fp_x = future_x-present_x;
		        int fp_y = future_y-present_y;
        		int pp_x = present_x-past_x;
        		int pp_y = present_y-past_y;

        		if(past_x==-1 && past_y==0){
            			if(present_x==0 && present_y==0){
                			if(fp_x==0 && fp_y==1){run_direct = 'l';}
                			else if(fp_x==1 && fp_y==0){run_direct = 'f';}
				}
			}
			else if(past_x == 5 && past_y == 4){
            			if(present_x==4 && present_y==4){
                			if(fp_x==0 && fp_y==-1){run_direct = 'l';}
			                else if(fp_x==-1 && fp_y==0){run_direct = 'f';}
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
				else {                     //뺐더니 2,1 3,1 2,2 등등 지나갔을 때
					run_direct = 'n';
				}
			}
   		} //else 끝
        	// linetracer
		struct timespec start_time, current_time;
		
		    // 현재 시간을 가져옵니다.
		    clock_gettime(CLOCK_REALTIME, &start_time);
		    double elapsed;
		    do {
		        clock_gettime(CLOCK_REALTIME, &current_time);
		        elapsed = (current_time.tv_sec - start_time.tv_sec) + 
		                  (current_time.tv_nsec - start_time.tv_nsec) / 1000000000.0;

    			if(run_direct == 'l'){ // left
				int trackValue1 = digitalRead(TRACKING_PIN1);
        			int trackValue2 = digitalRead(TRACKING_PIN2);
        			int trackValue3 = digitalRead(TRACKING_PIN3);
        			int trackValue4 = digitalRead(TRACKING_PIN4);

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
			
			else if(run_direct == 'r'){
				int trackValue1 = digitalRead(TRACKING_PIN1);
			        int trackValue2 = digitalRead(TRACKING_PIN2);
			        int trackValue3 = digitalRead(TRACKING_PIN3);
			        int trackValue4 = digitalRead(TRACKING_PIN4);
			
			        // 각 트래킹 핀의 값을 출력
			
			if (trackValue2 == 0 && trackValue3 == 0 && trackValue1 == 1 && trackValue4 == 1) {  //흰검검흰
			            Car_Run(60, 60);
			            delay(10);
			        } 
			        else if (trackValue2 == 0 && trackValue3 == 0 && trackValue1 == 0 && trackValue4 == 1) {  //검검검흰
			            Car_Run(60, 60);
			            delay(10);
			        }
			        else if (trackValue2 == 0 && trackValue3 == 0 && trackValue1 == 1 && trackValue4 == 0) {  //흰검검검
			            Car_Control(f, 110, f, 5);
			            delay(600);
			            trackValue2 = digitalRead(TRACKING_PIN2);
			            trackValue3 = digitalRead(TRACKING_PIN3);
			            while ((trackValue2 == 1) || (trackValue3 == 1)) {
			                Car_Control(f, 80, f, 5);
			                delay(10);
			                trackValue2 = digitalRead(TRACKING_PIN2);
			                trackValue3 = digitalRead(TRACKING_PIN3);
			            }
			            Car_Control(f,40,f,90);
			            delay(10);
			        }
			        else if (trackValue2 == 0 && trackValue3 == 0 && trackValue1 == 0 && trackValue4 == 0) { //검검검검
			            Car_Control(f, 110, f, 5);
			            delay(600);
			            trackValue2 = digitalRead(TRACKING_PIN2);
			            trackValue3 = digitalRead(TRACKING_PIN3);
			            while ((trackValue2 == 1) || (trackValue3 == 1)) {           //오작동의 가능성, 이래도 인식 안되면 stop해주면서 천천히 가보자. 아니다 지금 속도를 낮추자. 둘다 검은색 안뜨는 경우 종종 특정 구간 있고 특정 구간 너무 잘 돌아감. 이니셜 속도를 검정만 지날 정도로 가자
			                Car_Control(f, 100, f, 5);
			                delay(10);
			                trackValue2 = digitalRead(TRACKING_PIN2);
			                trackValue3 = digitalRead(TRACKING_PIN3);
			            }
			            Car_Control(f,40,f,90);
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
			
			        else if (trackValue2 == 1 && trackValue3 == 1 && trackValue1 == 0 && trackValue4 == 1) { //검흰흰흰
			            Car_Left(40,90);
			            delay(100);
			        }
			        else if (trackValue2 == 1 && trackValue3 == 1 && trackValue1 == 1 && trackValue4 == 0) { //흰흰흰검
			            Car_Right(90, 40);
			            delay(100);
			        }
			        
			        else if(trackValue2 == 1 && trackValue3 == 1 && trackValue1 == 1 && trackValue4 == 1){  //흰흰흰흰
			            Car_Run(60,60);
			            delay(10);
			        }
			        else{               //검흰흰검
			            Car_Run(60,60);
			            delay(10);
			        }
			}
				
    			else if(run_direct == 'f'){
				int trackValue1 = digitalRead(TRACKING_PIN1);
        			int trackValue2 = digitalRead(TRACKING_PIN2);
        			int trackValue3 = digitalRead(TRACKING_PIN3);
        			int trackValue4 = digitalRead(TRACKING_PIN4);
                		        if (trackValue2 == 0 && trackValue3 == 0 && trackValue1 == 1 && trackValue4 == 1) {  //흰검검흰
				            Car_Run(60, 60);
				            delay(10);
				        } 
				        else if (trackValue2 == 0 && trackValue3 == 0 && trackValue1 == 0 && trackValue4 == 1) {  //검검검흰
				            Car_Run(60, 60);
				            delay(10);
				        }
				        else if (trackValue2 == 0 && trackValue3 == 0 && trackValue1 == 1 && trackValue4 == 0) {  //흰검검검
				            Car_Run(60, 60);
				            delay(10);
				        }
				        else if (trackValue2 == 0 && trackValue3 == 0 && trackValue1 == 0 && trackValue4 == 0) { //검검검검
				            Car_Run(60, 60);
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
				
				        else if (trackValue2 == 1 && trackValue3 == 1 && trackValue1 == 0 && trackValue4 == 1) { //검흰흰흰
				            Car_Left(40,90);
				            delay(100);
				        }
				        else if (trackValue2 == 1 && trackValue3 == 1 && trackValue1 == 1 && trackValue4 == 0) { //흰흰흰검
				            Car_Right(90, 40);
				            delay(100);
				        }
				        
				        else if(trackValue2 == 1 && trackValue3 == 1 && trackValue1 == 1 && trackValue4 == 1){  //흰흰흰흰
				            Car_Run(60,60);
				            delay(10);
				        }
				        else{               //검흰흰검
				            Car_Run(60,60);
				            delay(10);
				        }
				    }

    			else if(run_direct == 'b'){
	    			//case cross T
	    			int trackValue1 = digitalRead(TRACKING_PIN1);
			        int trackValue2 = digitalRead(TRACKING_PIN2);
			        int trackValue3 = digitalRead(TRACKING_PIN3);
			        int trackValue4 = digitalRead(TRACKING_PIN4);
			
			        // 각 트래킹 핀의 값을 출력
			
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
    			else if(run_direct == 'n'){
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
    		
		    } while (elapsed < 1);   //while 1.5초 끝
		Car_Stop();
	} //while(1)
    close(sock);    
    return 0;
}
