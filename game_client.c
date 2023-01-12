#ifdef RaspberryPi

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

// 소켓
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

// wiringPi
#include <wiringPi.h>
#include <wiringSerial.h>

// 시리얼 usb 연결
char device[] = "/dev/ttyACM0";

//filedescriptor
int fd;
unsigned long baud = 9600;
unsigned long time = 0;

void setup();

int main(int argc, const char* args[])
{
    int sock = 0;
    struct sockaddr_in serv_address;
    memset(&serv_address, 0, sizeof serv_address);
    char message[1024];
    memset(message, '0', sizeof message);

    if(argc != 3)
    {
        fprintf(stdout, "입력 형식이 잘 못 되었음. \r\n");
        exit(1);
    }

    sock = socket(PF_INET, SOCK_STREAM, 0);

    if(sock == -1)
    {
        puts("소켓 오류 FD 문제 발생");
        exit(1);
    }

    serv_address.sin_family = AF_INET;
    serv_address.sin_addr.s_addr = inet_addr(args[1]);
    serv_address.sin_port = htons(atoi(args[2]));

    int connect_state = connect(sock, (struct sockaddr*)&serv_address, sizeof serv_address);
    if(connect_state == -1)
    {
        puts("connect 오류");
        exit(1);
    }

    setup();
    
    int play_game = 1;

    while(play_game)
    {
        char temp[2] = {0};
        char name[11] = {0};
        int len = 0;

        fflush(stdout);
        char data[30] = {'\0',};
        char newChar = '0';

        for(;;)
        {
            if(serialDataAvail(fd))
            {
                for(int i = 0; newChar != '\n'; ++i)
                {
                    newChar = serialGetchar(fd);
                    data[i] = newChar;
                }
            }

            printf("%s", data);

            if(strcmp(data, "NAME_INPUT\r\n") == 0)
            {
                fflush(stdout);
                break;
            }
        }
        
        printf("이름을 입력하세요(10글자 이내) : ");
        fgets(name, sizeof name, stdin);
        name[strlen(name) - 1] = '\0';
        printf("%s \r\n", name);

        write(sock, name, sizeof name);

        serialPuts(fd, "GAME_START");
        fflush(stdout);

        char scores[10] = {'\0',};
        newChar = '0';

        for(;;)
        {
            if(serialDataAvail(fd))
            {
                for(int i = 0; newChar != '\n'; ++i)
                {
                    newChar = serialGetchar(fd);
                    scores[i] = newChar;
                }

                break;
            }
        }

        write(sock, scores, sizeof scores);

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

        printf("\r\n==============================\r\n");
        printf("이름 : %s\r\n점수 : %d \r\n걸린 시간 : %d\r\n남은 목숨 : %d초\r\n", name, score, game_time, life);
        printf("==============================\r\n\r\n");

        char game_state[30] = {'\0',};
        newChar = '0';

        for(;;)
        {
            if(serialDataAvail(fd))
            {
                for(int i = 0; newChar != '\n'; ++i)
                {
                    newChar = serialGetchar(fd);
                    game_state[i] = newChar;
                }

                break;
            }
        }

        if(strcmp(game_state, "CONTINUE\r\n") == 0)
        {
            printf("게임을 계속 진행합니다.\r\n");
            write(sock, game_state, sizeof game_state);
        }
        else if(strcmp(game_state, "STOP\r\n")== 0)
        {
            printf("게임을 종료합니다.\r\n");
            write(sock, game_state, sizeof game_state);
            play_game = !play_game;
        }
    }
    
    close(sock);

    return 0;
}

void setup()
{
    printf("%s \r\n", "Raspberry Startup!");
    fflush(stdout);

    if ((fd = serialOpen (device, baud)) < 0)
    {
        fprintf (stderr, "Unable to open serial device: %s\n", strerror (errno));
        exit(1); //error
  }
 
  if (wiringPiSetup () == -1)
  {
    fprintf (stdout, "Unable to start wiringPi: %s\n", strerror (errno));
    exit(1); //error
  }
}

#endif
