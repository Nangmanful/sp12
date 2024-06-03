#include <stdio.h>
#include "qrrecognition.h"
#include <arpa/inet.h>
#include "server.h"
#include <string.h>
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



DGIST global_info;


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


int main(int argc, char *argv[]) {	
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
	printf("real 시작\n");

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

	char* index ="77";
        DGIST info;
	info = global_info;
	while (1) {
		char* qrCodeData;
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
    char input[100];
    char value;

    printf("Program is running. Type 'q' to stop.\n");
        printf("Enter a character: ");
        if (fgets(input, sizeof(input), stdin) != NULL) {
            // 사용자가 "q"를 입력하면 프로그램 종료
            if (input[0] == 'q' && input[1] == '\n') {
                break;
            }

            // 입력된 값을 문자로 변환
            qrCodeData = input[0];
            printf("You entered: %c\n", value);
        } else {
            // 입력 오류 처리
            printf("Error reading input.\n");
        }
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
		printf("qr끝 \n");
		fflush(stdout);
        	if (count) { // no qr recognition
                	printf("no qr\n");
			fflush(stdout);
        	}
        	else {
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
			if(ex == -1 && ey == -1){
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
 		}
	} //while(1)
    close(sock);    
    return 0;
}
