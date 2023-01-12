#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <wiringPi.h>
#include <wiringSerial.h>
#include <stdlib.h>
#include "mysql.h"
#include <time.h>

// MySQL 초기화 및 연결
int initDB(MYSQL * mysql, const char * host, const char * id, const char * pw, const char * db)
{
    printf("(i) initDB called, host = %s, id = %s, pw = %s, db = %s \r\n", host, id, pw, db);
    mysql_init(mysql);

    if(mysql_real_connect(mysql, host, id, pw, db, 0, NULL, 0))  // DB 접속
    {
        printf("(i) mysql_real_connect success \r\n");
        return 0;  // 성공
    }

    printf("(!) mysql_real_connect failed \r\n");
    return -1;  // 실패
}

// DB에 쓰는 함수
int writeDB(MYSQL * mysql, char * user_name, int score, int time, int life)
{
    char strQuery[255] = "";  // 쿼리 작성에 사용할 버퍼
    // 삽입 쿼리 작성
    sprintf(strQuery, "INSERT INTO game_rank(user_name, score, time, life) VALUES('%s', %d, %d, %d)", user_name, score, time, life);
    int res = mysql_query(mysql, strQuery);  // 삽입 쿼리 실행

    if(!res)
    {
        printf("(i) inserted %lu rows. \r\n", (unsigned long)mysql_affected_rows(mysql));
    }
    else
    {
        fprintf(stderr, "(i) insert error %d : %s \r\n", mysql_errno(mysql), mysql_error(mysql));
        return -1;
    }

    return 0;
}

// DB에서 읽어오기
// int play_num
int readDB(MYSQL * mysql, char * buf, int size, int id)
{
    char strQuery[256] = "";  // select query를 작성할 버퍼
    buf[0] = 0;  // 반환할 값은 strcat() 함수를 이용할 수 있도록 첫 바이트에 vull을 넣는다.

    // select query 작성
    sprintf(strQuery, "SELECT user_name, score, time, life FROM Where id = '%d';", id);
    int res = mysql_query(mysql, strQuery);  // query의 실행

    if(res != 0)  // 실패
    {
        return -1;
    }
    else  // 성공
    {
        MYSQL_RES * res_ptr = mysql_use_result(mysql);
        MYSQL_ROW sqlrow = mysql_fetch_row(res_ptr);
        
        unsigned int field_count = 0;

        while(field_count < mysql_field_count(mysql))
        {
            char buf_field[256] = "";  // 각 필드에 보관된 개별 값을 저장

            if(sqlrow[field_count])
            {
                sprintf(buf_field, "%s", sqlrow[field_count]);
            }
            else
            {
                sprintf(buf_field, "0,");
            }

            strcat(buf, buf_field);  // string append
            ++field_count;
        }

        if(mysql_errno(mysql))
        {
            fprintf(stderr, "(!) error : %s \r\n", mysql_error(mysql));
            return -1;
        }

        mysql_free_result(res_ptr);  // 시스템에 맡겨놓은 결과셋 버리기
    }

    return 0;
}

int readallDB(MYSQL * mysql)
{
    // int rank_id[10] = {0,};
    
    if(mysql_query(mysql, "SELECT * FROM game_rank ORDER BY score DESC, time ASC, life DESC;"))
    {
        return -1;
    }

    MYSQL_RES * result = mysql_store_result(mysql);
    
    if(result == NULL)
    {
        return -1;
    }

    int num_fields = mysql_num_fields(result);

    MYSQL_ROW row;

    // int j = 0;

    printf("==============================\r\n");
    while(row = mysql_fetch_row(result))
    {
        for(int i = 0; i < num_fields; ++i)
        {
            printf("%s ", row[i]?row[i]:"NULL");
        }

        /*
        rank_id[j] = row[0];
        ++j;
        */

        printf("\r\n");
    }
    printf("=============================\r\n");

    mysql_free_result(result);
    return 0;
}

// DB 접속 종료
int closeDB(MYSQL * mysql)
{
    mysql_close(mysql);  // DB 연결 해제
    return 1;
}
