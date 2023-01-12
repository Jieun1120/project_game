#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

// 소켓
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

// WiringPi
#include <wiringPi.h>
#include <wiringSerial.h>

// DB
#include "mysql.h"

int main(int argc, const char* args[])
{
    // 서버 연결
    if(argc != 2)
    {
        fprintf(stdout, "입력이 잘못되었어요. \r\n");
        fprintf(stdout, "%s \t %s \r\n", args[0], args[1]);
        exit(1);
    }
    else
    {
        fprintf(stdout, "정상적으로 입력되었습니다. \r\n");
        fprintf(stdout, "%s \t %s \r\n", args[0], args[1]);
    }

    int server_sock = 0;
    int client_sock = 0;
    struct sockaddr_in serv_address;
    struct sockaddr_in client_address;

    server_sock = socket(PF_INET, SOCK_STREAM, 0);

    if(server_sock == -1)
    {
        puts("소켓 오류");
        exit(1);
    }

    memset(&serv_address, 0, sizeof serv_address);
    serv_address.sin_family = AF_INET;
    serv_address.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_address.sin_port = htons(atoi(args[1]));

    memset(&client_address, 0, sizeof client_address);

    int bind_state = bind(server_sock, (struct sockaddr*)&serv_address, sizeof serv_address);

    if(bind_state == -1)
    {
        puts("bind 오류 발생");
        exit(1);
    }

    int listen_state = listen(server_sock, 5);

    if(listen_state == -1)
    {
        puts("listen 오류 발생");
        exit(1);
    }

    int client_addr_size = sizeof client_address;
    int accept_state = accept(server_sock, (struct sockaddr*)&client_address, &client_addr_size);

    if(accept_state == -1)
    {
        puts("accept 오류 발생");
        exit(1);
    }

    // DB 연결
    MYSQL mysql;

    if(initDB(&mysql, "localhost", "root", "1234", "mydb") < 0)
    {
        printf("(!) initDB failed... \r\n");
        return -1;
    }
    else
    {
        printf("(i) initDB successed! \r\n");
    }

    int play_game = 1;
    
    while(play_game)
    {
        char name[11] = {'\0',};
        int read_state = read(accept_state, name, sizeof(name));

        if(read_state == -1)
        {
            puts("이름을 못 받았어요");
        }

        char scores[10] = {'\0',};
        read_state = read(accept_state, scores, sizeof scores);

        char * score_ptr;
        int score_arr[3] = {0,};
        score_ptr = strtok(scores, ",");
    
        for(int i = 0; score_ptr; ++i)
        {
            score_arr[i] = atoi(score_ptr);
            score_ptr = strtok(NULL, ",");
        } 

        int score = score_arr[0];
        int life = 4 - score_arr[1];
        int game_time = score_arr[2];

        printf("%s, %d, %d, %d \r\n", name, score, game_time, life);
        int res = writeDB(&mysql, name, score, game_time, life);

        if(res < 0)
        {
            printf("(!) writeDB failed \r\n");
            return -1;
        }
        else
        {
            printf("(i) writeDB success! \r\n");
        }

        res = readallDB(&mysql);
        // int rank_id[10] = {0,};

        if(res < 0)
        {
            printf("(!) readallDB failed \r\n");
            return -1;
        }
        else
        {
            printf("(i) readallDB success! \r\n");
        }

        char game_state[30] = {'\0',};
        read_state = read(accept_state, game_state, sizeof game_state);

        if(strcmp(game_state, "CONTINUE\r\n") == 0)
        {
            printf("게임을 계속 진행합니다. \r\n");
        }
        else if(strcmp(game_state, "STOP\r\n") == 0)
        {
            printf("게임을 종료합니다. \r\n");
            if(closeDB(&mysql) < 0)
            {
                printf("(!) closeDB failed \r\n");
                return -1;
            }
            else
            {
                printf("(i) closeDB success \r\n");
            }
            play_game = !play_game;
        }
    }
    
    close(client_sock);
    close(server_sock);

    return 0;
}
