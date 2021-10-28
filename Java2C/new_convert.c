#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdbool.h>
#include <sys/time.h>
#include <errno.h>
#include <sys/wait.h>

#include <classes.c>

#define MAX_OBJ 3
#define MAX_CLASS 3
#define P_OPTION_NUM 30
#define BUFFER_SIZE 1024 
#define SECOND_TO_MICRO 1000000




char* read_file(char* file_name) {
    // 1. open file
    FILE *fp = NULL;
    if((fp = fopen(file_name, "r")) == NULL) {
        printf("there is no file name %s\n", argv[1]);
        exit(1);
    }

    // 2. read file by one line
    int len; 
    int line_no = 0;
    char buf[BUFFER_SIZE];

    while(!feof(fp)) //if it is not an end of file
    {
        fgets(buf, BUFFER_SIZE, fp);
        low[line_no] = (char *)malloc(sizeof(buf));
        strcpy(low[line_no], buf);

        // if there is definition of Class
        if(strstr(buf, "class") != NULL) {
            class_setting(buf, line_no);
            if(class_num >= 1) //class시작전 지우기
               for(i=1; i<line_no; i++)
                   if(strcmp(low[line_no-i],"\n")){
                       strcpy(low[line_no-i], " ");
                       break;
                   }class_num++;
        }
        line_no++;
    }
    fclose(fp);
    setHeader(header);

}


typedef struct Class {

    char* name;
    char* constructor;//생성함수 new Stack();
    char* obj[MAX_OBJ]; //한 클래스당 객체 MAX_OBJ개 생성 가능
    int num_obj; //obj 개수
    int linelen; //몇번째 줄에서 class가 시작되는지
}Class;

/******************to define class***********/
//새로운 class가 들어왔을 경우 새로운 class가 시작되는 줄의 위치 linelen에 기록
//class의 이름을 구조체 class 의 name에 저장
//class의 생성자 함수인 new name(); 을 constructor 에 저장
void class_setting(char* buf, int linelen) {
    int i;
    bool class_case;
    bool loop = true;
    // class_case
    // true. public class ql {
    // false. public class q1{ 
    char* ptr = strtok(buf, " ");

    while(loop) {
        if (strstr(ptr, " {") == NULL) {
            loop = false;
            strtok(ptr, " {");
        }
        else if (strstr(ptr, "{") == NULL) {
            loop = false;
            strtok(ptr, "{");

        }
        else
            ptr = strtok(NULL, " ");
    }

    class[class_num].name = (char*)malloc(sizeof(ptr));
    strcpy(class[class_num].name, ptr);

    if(class_case) {
        
    }

    
    strcpy(ptr, "new ");
    strcat(ptr, class[class_num].name);
    strcat(ptr, "();");
    class[class_num].constructor = (char*)malloc(sizeof(ptr));
    strcpy(class[class_num].constructor, ptr); 

    class[class_num].num_obj = 0;
    class[class_num].linelen = linelen;
}


int main(int argc, char* argv[]) {

    struct timeval begin_t, end_t;
    int i = 0;
    int j = 0;
    int linelen = 0;
    FILE *fp = NULL;
    char *low[100];
    clow clow[100]; //there are char* low & int len(which represent the length of the low)
    char buf[BUFFER_SIZE];
    char *filename;
    Header header[3];

    gettimeofday(&begin_t, NULL);
    getfilename(argv[1]);

    read_file(argv[1])

    