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

#define SECOND_TO_MICRO 1000000
#define BUFFER_SIZE 1024 
#define P_OPTION_NUM 30
#define MAX_CLASS 3
#define MAX_OBJ 3

typedef struct clow {

    char* low;
    char* jlow;
    int len;
    int ch;
}clow;

typedef struct Class {

    char* name;
    char* constructor;//생성함수 new Stack();
    char* obj[MAX_OBJ]; //한 클래스당 객체 MAX_OBJ개 생성 가능
    int num_obj; //obj 개수
    int linelen; //몇번째 줄에서 class가 시작되는지
}Class;

typedef struct Header {

    char type[10];
    char file[3][BUFFER_SIZE]; //한 type당 3개의 header file 가질 수 있음
    int num_file; //하나의 type의 header file의 개수
}Header;

typedef struct P { //for P option
     
    int num;
    char java[BUFFER_SIZE]; //java line
    char c[BUFFER_SIZE]; //c line
}P;

P p[P_OPTION_NUM];
Class class[MAX_CLASS];
int p_option[BUFFER_SIZE];
int p_num=0;
int class_num = 0;
int rOption = false;
int lOption = false;
int fOption = false;
int pOption = false;
int cOption = false;
int jOption = false;
char javafile[10];
char filename[10];
char cfile[10];
char exefilename[10];


void ssu_runtime(struct timeval* begin_t ,struct timeval* end_t);
void setHeader(Header* header);
char* convert(char* low, int* clow, int* clow_ch, Header* header);
void makecfile(char* filename, clow* clow, int line_len, Header* header);
void make_exefile();
void getfilename(char* filename);
int check_option(int argc, char * argv[], clow* clow);
int getTotalLine(char *filename);
void create_method(char* low, char* buf);
char* new_malloc(char* low, char* buf);
void get_obj(char* low, int class_num);
void class_setting(char* buf, int linelen);
char* obj_method(char* low);
void newHeader(Header* header, int type, char* file);
void setP();
void setP2();
int search_obj(char* low);
void update_Header(Header* header);
void do_option(clow* clow);
void r_option(clow* clow, Header* header);

/*******to check run time********/
void ssu_runtime(struct timeval* begin_t, struct timeval* end_t) {

    end_t -> tv_sec -= begin_t -> tv_sec;

    if(end_t -> tv_usec < begin_t -> tv_usec) {
        end_t -> tv_sec--;
        end_t -> tv_usec += SECOND_TO_MICRO;
    }
    end_t -> tv_usec -= begin_t -> tv_usec;
    printf("Runtime : %ld:%06ld(sec:usec)\n", end_t -> tv_sec, end_t -> tv_usec);
}

/**********Header***********/
//새로 추가된 헤더를 header에 추가한다.
//기존에 있었던 내용을 지우고
//struct Header에 있는 내용을 순차적으로 적는다.
void update_Header(Header* header) {

    int i, j;
    int fd = open("header", O_WRONLY|O_TRUNC);
    
    for(i=0; i<3; i++) {
        
        write(fd, header[i].type, strlen(header[i].type));
        for(j=0; j< header[i].num_file; j++) {
            //write(fd," ",1);
            write(fd, header[i].file[j], strlen(header[i].file[j]));
        }
        write(fd, "\n", 1);

    }
    close(fd);
}

//struct Header 에 입력하고자하는 헤더파일(file)이 해당type에 존재하는지 검색후
//있을경우 1을 return 없을경우 0을 return한다.
int searchHeader(Header* header, int type, char* file) {

    int num = header[type].num_file;

    for(int i=0; i< num; i++) {
        if((strncmp(header[type].file[i], file, strlen(file))) == 0) {
            return 1;
        }
    }
    return 0;
    
}
//setHeader()는 헤더 테이블이 있는 header파일을 읽기 모드로 읽은 후
//struct header에 테이블의 값을 넣는다.
//header.type에는 open read exit를 넣고
//header.file은 각 type에 들어있는 file들을 순차적으로 넣는다.
void setHeader(Header* header) {
    FILE *fp = fopen("header", "r+");
    int i=0, j=0;
    char buf[BUFFER_SIZE];
    char *line_p;

    while(!feof(fp))
    {
        j=0;
        fgets(buf, BUFFER_SIZE, fp);
        if((line_p = strchr(buf, '\n')) != NULL)*line_p = '\0';
        char* ptr = strtok(buf, "#");
        while(ptr != NULL) {

            if( j== 0) {
                strcpy(header[i].type, ptr);
            }
            else {
                strcpy(header[i].file[j-1], "#");
                strcat(header[i].file[j-1], ptr);
            }
            j++;
            ptr = strtok(NULL, "#");
        }header[i].num_file = j-1;
        i++;
    }
}
//새로운 header를 입력할 경우 어느 type(open, read, exit) 에 넣고 싶은지 
//새로 넣고 싶은 file의 이름을 type의 비어있는 공간에 넣어준다.
void newHeader(Header* header,int type, char* file) {

    int i = header[type].num_file;
    int j;
    if(i >= 3) {
        printf("there is no space to put new header\n");
        exit(1);
    }
    strcpy(header[type].file[i], file);
    strcat(header[type].file[i], " ");
    header[type].num_file+=1;
}
/***********check if there is any obj in this low*******/
//class로 생성된 객체의 이름(st)이 존재하는 지 확인한다.
int search_obj(char* low) {

    char* pos;

    for(int i=0; i< MAX_CLASS; i++) {
        for(int j=0; j< MAX_OBJ; j++) {
            
            if(class[i].obj[j] != NULL && (pos = strstr(low, class[i].obj[j])) != NULL)
                return 1;
        }
    }
    return 0;

}
/*************for -p option**************/
//p option을 위해 java에서 c로 변환되는 함수에 번호를 매겨 구조체p에
//번호 num, java 자바 함수, c c함수를 저장한다.
void setP() {

    p[0].num = 0;
    strcpy(p[0].java, "System.out.printf()");
    strcpy(p[0].c, "printf()");

    p[1].num = 1;
    strcpy(p[1].java, "scn.nextInt()");
    strcpy(p[1].c, "scanf()");

    p[2].num = 2;
    strcpy(p[2].java, "close()");
    strcpy(p[2].c, "fclose()");
    
    p[3].num = 3;
    strcpy(p[3].java, "File file = new File()");
    strcpy(p[3].c, "FILE* file = fopen()");
    
    p[4].num = 4;
    strcpy(p[4].java, "flush()");
    strcpy(p[4].c, "fflush()");

    p[5].num = 5;
    strcpy(p[5].java, "writer.write()");
    strcpy(p[5].c, "fputs()");

    p[6].num = 6;
    strcpy(p[6].java, "new");
    strcpy(p[6].c, "malloc");


}
//convert()중 어떠한 java 함수들이 c로 번역이 되었는지에 대한 정보를 담은 q[]를 분석한다.
//분석후 중복되는 함수는 하나만 기록한다.
//p = {6, 0,0,1,2} -> p ={6,0,1,2}로 바꾼다.
void setP2() {

    int c;
    int i;
    int num0 = 0;
    int num1 = 0;
    int num2 = 0;
    int num3 = 0;
    int num4 = 0;
    int num5 = 0;
    int num6 = 0;

    int q[6];
    int q_num = 0;

    for(i=0; i<p_num; i++) {

        c = p_option[i];
        switch(c) {

            case 0 :
                if(num0 == 0) {
                    q[q_num] = 0;     
                    num0 = 1;
                    q_num++;
                }
                break;

            case 1 :
                if(num1 == 0) {
                    q[q_num] = 1;     
                    num1 = 1;
                    q_num++;
                }
                break;

            case 2 :
                if(num2 == 0) {
                    q[q_num] = 2;     
                    num2 = 1;
                    q_num++;
                }
                break;

            case 3 :
                if(num3 == 0) {
                    q[q_num] = 3;     
                    num3 = 1;
                    q_num++;
                }
                break;

            case 4 :
                if(num4 == 0) {
                    q[q_num] = 4;     
                    num4 = 1;
                    q_num++;
                }
                break;

            case 5 :
                if(num5 == 0) {
                    q[q_num] = 5;     
                    num5 = 1;
                    q_num++;
                }
                break;

            case 6 :
                if(num6 == 0) {
                    q[q_num] = 6;     
                    num6 = 1;
                    q_num++;
                }
                break;
        }
    }
        for(i=0; i< q_num; i++) {
            p_option[i] = q[i];
        }
        p_num = i;
}
/******************to define class***********/
//새로운 class가 들어왔을 경우 새로운 class가 시작되는 줄의 위치 linelen에 기록
//class의 이름을 구조체 class 의 name에 저장
//class의 생성자 함수인 new name(); 을 constructor 에 저장
void class_setting(char* buf, int linelen) {
    int i;
    char* ptr = strtok(buf, " ");
    while(strstr(ptr, "{") == NULL) {
        ptr = strtok(NULL, " ");
    }
    strtok(ptr, "{");
    class[class_num].name = (char*)malloc(sizeof(ptr));
    strcpy(class[class_num].name, ptr);

    strcpy(ptr, "new ");
    strcat(ptr, class[class_num].name);
    strcat(ptr, "();");
    class[class_num].constructor = (char*)malloc(sizeof(ptr));
    strcpy(class[class_num].constructor, ptr); 

    class[class_num].num_obj = 0;
    class[class_num].linelen = linelen;
}
/****************make c file and write **************/
//c file을 생성 후 convert로 번역한 clow구조체의 low의 값을 한 줄씩 cfile에 적는다.
void makecfile(char* filename ,clow* clow, int line_len, Header* header) {

    int fd;
    char buf[BUFFER_SIZE];
    int i, j;
   

    if((fd = creat(cfile, 0777)) < 0) {
        fprintf(stderr, "creat error for %s\n", cfile);
        return;
    }
    for(i=0; i<3; i++) {
        for(j=0; j<header[i].num_file; j++) {
            
            write(fd, header[i].file[j], strlen(header[i].file[j]));
            write(fd, "\n", 1);
        }
    }
    for(i=0; i<line_len-2; i++) {
        write(fd, clow[i].low, clow[i].len);
    } 
    close(fd);

}
/*************make an exe file**********/
//c file을 컴파일 시켜 cfile_Makefile 실행파일을 만든다
//자식 프로세스가 system()을 이용해 컴파일을 하고
//부모 프로세스는 자식프로세스가 할 일을 끝내고 죽기를 기다린다.
void make_exefile() {
    FILE* fp;
    int fd;

    char buf[2016];

    fp = fopen(exefilename, "w+");

    for(int i=0; i< 2016; i++)
        buf[i]='\0';

    strcat(buf, filename);
    strcat(buf, ": ");
    strcat(buf, cfile);
    strcat(buf, "\n\t");

    strcat(buf, "gcc ");
    strcat(buf, cfile);
    strcat(buf, " -o ");
    strcat(buf, filename);

    fprintf(fp, "%s", buf);

    
    fclose(fp);
    
}
/***********convert java method into c*******/
//java 형식의 method 가 들어올경우 public 을 제거하고 return 형이 정의 되어있지
//않을 경우 void로 적는 등
//c의 형태의 method로 바꾼다.
void create_method(char* low, char* buf) {  //public Stack()

    char *ptr = strtok(low, " ");
    int len;
    int i=0;

    while(ptr != NULL) {

        len = strlen(ptr) +1;

        if(strstr(ptr, "public") != NULL) {
            strncpy(ptr, "     ", 6);
            i++;
        }
        if(strstr(ptr, "(") !=NULL && i<3) {
            strcat(buf, "void ");
        }

        strcat(buf, ptr);
        strcat(buf," ");
        i++;
        ptr = strtok(NULL, " ");
    }
}
/***********get file name from argv[1]*******/
//argv[1]로 들어온 q1.java등 의 파일이름을 cfile c형태의 파일이름 q1.c
//filename q1을 전역변수로 저장한다.
void getfilename(char* name) {
    int len = strlen(name);
    char ptr[len];

    strcpy(ptr, name);
    char* pos = strtok(ptr, ".");

    strcpy(filename, pos);
    
    strcpy(cfile, filename);
    strcat(cfile, ".c");

    strcpy(javafile, name);
    
    strcpy(exefilename, filename); //q1
    strcat(exefilename, "_Makefile"); //q1_Makefile
}
/***********convert new in java file into c's malloc*******/
    //low : new int[STACK_SIZE];
    //devide like type:int , size:STACK_SIZE
    //make buf like (type*)malloc(sizeof(type)*size);
char* new_malloc(char* low, char* buf) {

    char *ptr = strtok(low, " "); //new int[STACK_SIZE];
    char *size;
    char *type;
    char buf_[BUFFER_SIZE];
    char* tmp;
    char *opt[2];
    int i=0;

        ptr = strtok(NULL, " "); //int[STACK_SIZE];
        ptr = strtok(ptr, "[");
        type = ptr;

        ptr = strtok(NULL, "[");
        tmp = strstr(ptr, "];");
        strncpy(tmp, ");", 2);

        size = ptr; //STACK_SIZE

    strcpy(buf_, "(");
    strcat(buf_, type);
    strcat(buf_, "*)malloc(sizeof(");
    strcat(buf_, type);
    strcat(buf_, ")*");
    strcat(buf_, size);
    
    buf = buf_;

    return buf;

}
/********get the num of line of file*******/
//file의 총 줄의 개수를 return 한다.
int getTotalLine(char * filename) {

    FILE *fp;
    int line = 0;
    char c;

    fp = fopen(filename, "r");
    while((c = fgetc(fp)) != EOF)
        if(c == '\n')
            line++;

    fclose(fp);
    return line;
}
/**********get obj from obj.method()******/
//st.push(); 같은 객체.함수형식이 들어왔을 경우
//객체의 이름을 추출해서 구조체 class[몇번째 class?].obj에 순차적으로 저장한다.
//몇번째 obj에 저장할지는 현재 class[class_num]에 몇개의 obj가 있는지에 대한 정보가 있는
//num_obj를 이용해 다음의 빈 공간에 obj의 이름을 저장할 수 있다.
void get_obj(char* low, int class_num) {

    char* ptr = strtok(low, " ");
    ptr = strtok(NULL, " ");

    int num = class[class_num].num_obj;
    strcat(ptr, ".");
    class[class_num].obj[num] = (char*)malloc(sizeof(ptr));
    strcpy(class[class_num].obj[num], ptr);
    class[class_num].num_obj +=1;
}
/*********change obj.method() into method()****/
//low 에 st.push(); 같은 객체.함수 형식일 경우
//객체.를 제거해서 함수만 c파일에 입력할 수 있도록 변환한다.
//st.push(5) -> push(5)
char* obj_method(char* low) {

    char* ptr;
        int i, j;
        for(i=0; class[i].name!= NULL; i++) {
            for(j=0; class[i].obj[j]!= NULL; j++) {
               
                if((ptr = strstr(low, class[i].obj[j]))!= NULL) {
                    //st.push(5); -> push(5);
                    strncpy(ptr, "   ", strlen(class[i].obj[j]));
                    return low;
                }
            }
        }
        return low;
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

    if((fp = fopen(argv[1], "r")) == NULL) {
            printf("there is no file name %s\n", argv[1]);
            exit(1);
    }
    int len; 
    while(!feof(fp)) //if it is not an end of file
    {
        fgets(buf, BUFFER_SIZE, fp);
        low[linelen] = (char *)malloc(sizeof(buf));
        strcpy(low[linelen], buf);
        if(strstr(buf, "class") != NULL) {
            class_setting(buf, linelen);
            if(class_num >= 1) //class시작전 지우기
               for(i=1; i<linelen; i++)
                   if(strcmp(low[linelen-i],"\n")){
                       strcpy(low[linelen-i], " ");
                       break;
                   }class_num++;
        }linelen++;
    }
    fclose(fp);
    setHeader(header);

    if(!check_option(argc, argv, clow)) {
        exit(1);
    }

    for(i=0; i<linelen; i++) { 
	    clow[i].ch = 0;
        clow[i].low = convert(low[i], &clow[i].len, &clow[i].ch, header); //int len길이 포인터 전달
    }
   
   if(rOption) {
	   r_option(clow, header);
	   exit(1);
   }
    
    makecfile(filename, clow, linelen, header); //번역한 clow값을 .cfile write
    
    printf("%s convert Success!\n", cfile);
    do_option(clow);
    
    make_exefile(); //makefile 생성 fork()

    fclose(fp);

    update_Header(header);
    
    for(i=0; i<linelen; i++)
        free(low[i]);

    gettimeofday(&end_t, NULL);
    ssu_runtime(&begin_t, &end_t);
    exit(0);
}
/*********check option and do option's func*****/
int check_option(int argc, char *argv[], clow* clow) {

    int i, j;
    int c;
    char *dir[2];
    char buf[BUFFER_SIZE];


    while((c = getopt(argc, argv, "rjcpfl")) != -1) {
        
    //char *filename = argv[1];
        switch(c) {

            case 'r' :
                rOption = true;
		        //자식 프로세스를 생성하여 java 언어 프로그램이 변환되는 과정을 한 줄 한 줄 출력.
                //단 java 언어 프로그램의 매 statement 가 c언어 프로그램으로 변환되어 출력되고 터미널 화면이 clear되고 다시 다음 statement 변환 및 출력
                //단 clear 는 systme 함수 사용 가능
                //변환이 완료되면 '파일명. c' converting is finished!출력
            
                
                break;

            case 'j' :
                jOption = true;
		
                break;

            case 'c' :
                cOption = true;
                
                break;

            case 'p' : 
                pOption = true;
                
                break;

            case 'f' :
                fOption = true;
		                break;

            case 'l' :
                lOption = true;
		    break;

            case '?' :
                return false;
        }
    }
    i=0;
    while(optind < argc) {
        dir[i] = argv[optind];
        i++;
        optind++;
    }
    return true;
}
void r_option(clow* clow, Header* header) {

        int line = getTotalLine(cfile);
	pid_t pid;
        char* low[100];
        int clinelen = 0;
	int jlinelen = 0;
	FILE* fp1, *fp2;
	int i=0;
	int j=0;
	int status;
	char buf[BUFFER_SIZE];
	char cbuf[BUFFER_SIZE];
	char jbuf[BUFFER_SIZE];
	char hbuf[BUFFER_SIZE];

	fp1 = fopen(javafile, "r");
	clinelen = getTotalLine(cfile);
	jlinelen = getTotalLine(javafile);


  	for(i=0; i< BUFFER_SIZE; i++)
		buf[i] = '\0';

	fflush(fp1);
	fflush(fp2);
	if((pid =fork()) > 0) {
		pid_t waitPid;
		waitPid = wait(&status);
		if(waitPid == -1) {
			printf("error num :%d\n", errno);
			perror("return wait() error");
		}
		else { i=0;
		int num = 1;
			if(WIFEXITED(status)) {
				printf("\n-------------\n%s\n----------\n", cfile);

				for(j=0; j<3; j++){ 
					for(i=0; i<header[j].num_file; i++, num++) {
						printf("%d %s\n",num, header[j].file[i]);
						sleep(2);	
					}
				}
						
				for(i=0; i<clinelen; i++, num++) {
					strcpy(cbuf, clow[i].low);
					printf("%d %s\n",num, cbuf);
					sleep(2);
				}
			}
			else if(WIFSIGNALED(status)) {
				printf("wait : 자식 프로세스 비정상 종료\n");
			}
    		}
	}
   	 else if(pid == 0) {
		printf("%s Converting...\n", javafile);
		printf("-------------\n%s\n--------\n", javafile);
		while(!feof(fp1))
		{
			fgets(buf, BUFFER_SIZE, fp1);
			strcat(jbuf, buf);
			system("clear");
			printf("%s\n", jbuf);
			
		}
		exit(0);
	}
	else {
		perror("fork Fail\n");
		exit(1);
	}
		
        fclose(fp1);
	fclose(fp2);
	exit(0);
	printf("%s converting is finished!\n",filename);
   
}
void do_option(clow* clow) {
    
    char buf[BUFFER_SIZE];

    if(cOption) {

        int fd = open(cfile, O_RDONLY);

        int fsize = lseek(fd, 0, SEEK_END);
        lseek(fd, 0, SEEK_SET);

        read(fd, buf, fsize);
        printf("%s\n", buf);
    }
    if(jOption) {
        int fd = open(javafile, O_RDONLY);

        int fsize = lseek(fd, 0, SEEK_END);
        lseek(fd, 0, SEEK_SET);

        read(fd, buf, fsize);
        printf("%s\n", buf);
        close(fd);
    }   
    if(pOption) {
         setP();
         setP2();

         for(int i=0; i<p_num; i++) {
                    
              int num = p_option[i];

               printf("%d %s -> %s\n", i+1, p[num].java, p[num].c);
        }
    }
    if(fOption) {
        int fd = open(javafile, O_RDONLY);
        int fsize = lseek(fd, 0, SEEK_END);

        printf("1. %s file size is %d bytes\n",javafile, fsize);

        fd = open(cfile, O_RDONLY);
        fsize = lseek(fd, 0, SEEK_END);

        printf("2. %s file size is %d bytes\n", cfile, fsize);

    }
    if(lOption) {
        int line = getTotalLine(javafile);
            
        printf("%s line number is %d lines\n", javafile, line);


        line = getTotalLine(cfile);
        printf("%s line number is %d lines\n", cfile, line);
            
    }
        
       
}

/**********convert**************/
char* convert(char* low, int* clow_len, int* clow_ch, Header* header) {

    char buf[BUFFER_SIZE];
    char buf_[BUFFER_SIZE];
    int i;
    char* pos;
    char* clow;

    for(i=0; i<BUFFER_SIZE; i++) 
        buf[i] = '\0';

    if((pos = strstr(low, "System.out.printf(")) != NULL) {
        
        char file[BUFFER_SIZE] = "#include <stdio.h>";
        if(!searchHeader(header, 1, file))  
            newHeader(header, 1, file);
        
    
        if(search_obj(low)) {
            char* ptr = obj_method(low);
            strncpy(ptr, "   ", 3);
        }
        strcpy(buf, "\t\tprintf(");

        strncpy(buf_, pos+18, 50);
        strcat(buf, buf_);
        
        clow = (char*)malloc(sizeof(buf));
        strcpy(clow, buf);
        *clow_len = strlen(clow);

        p_option[p_num] = 0;
        p_num++;

        return clow;
    }
    else if((pos = strstr(low, " = scn.nextInt()")) != NULL) {
       
        char* tmp = strtok(low, "="); //"num
        char* num = strstr(tmp, "\""); 
        strncpy(tmp, " ", 1); //num
        strcpy(buf, "\t\tscanf(\"%d\", &"); //scanf("%d", &
        strcat(buf, tmp); //scanf("%d", &num
        strcat(buf, ");");

        clow = (char*)malloc(sizeof(buf));
        strcpy(clow, buf);
        *clow_len = strlen(clow);

        p_option[p_num] = 1;
        p_num++;
        return clow;
    }
    else if((pos = strstr(low, "class ")) != NULL) {
        clow = (char*)malloc(sizeof(char));
        strcpy(clow, buf);
        *clow_len = strlen(clow);
        return clow;
    }
    else if((pos = strstr(low, "public static void main")) != NULL) {
        strcpy(buf, "int main(){");
        clow = (char*)malloc(sizeof(buf));
        strcpy(clow, buf);
        *clow_len = strlen(clow);
        return clow;
    }
    else if((pos = strstr(low, "public static final")) != NULL) {
        strncpy(buf, pos+20, 50);
        clow = (char*)malloc(sizeof(buf));
        strcpy(clow, buf);
        *clow_len = strlen(clow);
        return clow;
    }
    else if((pos = strstr(low, "public ")) != NULL) {
        create_method(pos, buf);

        clow = (char*)malloc(sizeof(buf));
        strcpy(clow, buf);
        *clow_len = strlen(clow);
        return clow;
    }
    else if((pos = strstr(low, "Scanner scn = new Scanner(System.in);")) != NULL) {
        
        clow = (char*)malloc(sizeof(char));
        strcpy(clow, buf);
        *clow_len = strlen(clow);
        return clow;
    }
    else if((pos = strstr(low, "import java.util.Scanner;")) != NULL) {
        char file[BUFFER_SIZE] = "#include <stdio.h>";
        if(!searchHeader(header, 1, file))
            newHeader(header, 1, file);

        clow = (char*)malloc(sizeof(char));
        strcpy(clow, buf);
        *clow_len = strlen(clow);
        return clow;
    }
    else if((pos = strstr(low, "import java.io.IOException;")) != NULL) {
        
        clow = (char*)malloc(sizeof(char));
        strcpy(clow, buf);
        *clow_len = strlen(clow);
        return clow;
    }
    else if((pos = strstr(low, "import java.io.File;")) != NULL) {

        char file[BUFFER_SIZE] = "#include <stdio.h>";
        if(!searchHeader(header, 1, file))
            newHeader(header, 1, file);

        clow = (char*)malloc(sizeof(char));
        strcpy(clow, buf);
        *clow_len = strlen(clow);
        return clow;
    }
    else if((pos = strstr(low, "import java.io.FileWriter;")) != NULL) {

        clow = (char*)malloc(sizeof(char));
        strcpy(clow, buf);
        *clow_len = strlen(clow);
        return clow;
    }
    else if((pos = strstr(low, "close()")) != NULL) {
        
        strcpy(buf, "\tfclose(file);");

        clow = (char*)malloc(sizeof(buf));
        strcpy(clow, buf);
        *clow_len = strlen(clow);

        p_option[p_num] = 2;
        p_num++;
        return clow;
    }
    else if((pos = strstr(low, "[]")) != NULL) {
        strncpy(pos, "* ", 2);
        clow = (char*)malloc(sizeof(buf));
        strcpy(clow, low);
        *clow_len = strlen(clow);
        return clow;
    }
    else if((pos = strstr(low, "return ;")) != NULL) {

        clow = (char*) malloc(sizeof(char));
        strcpy(clow, buf);
        *clow_len = strlen(clow);
        return clow;
    } 
    else if((pos = strstr(low, "File file = new File(")) != NULL) {
        char* ptr;

        strcpy(buf, "\tFILE* file = fopen(");
        //if((FILE* file = fopen("q3java.txt", "w")) != NULL) {
        //      printf("cant open this file\n");
        //}
         
        strncpy(buf_, pos+21, 22);
        ptr=strtok(buf_, ");");
        strcpy(buf_, ptr);

        strcat(buf_, ", \"w\");\n");

        char * file =strtok(low, " ");
        file = strtok(NULL, " ");

        char buf_1[BUFFER_SIZE];

        strcpy(buf_1, "\tif (");
        strcat(buf_1, file);
        strcat(buf_1, "!= NULL) {\n\t\tprintf(\"cant open this file\");\n\t\texit(1);\n\t}");
        
        strcat(buf, buf_);
        strcat(buf, buf_1);

        clow = (char*)malloc(sizeof(buf));
        strcpy(clow, buf);

        *clow_len = strlen(clow);

        p_option[p_num] = 3;
        p_num++;

        return clow;
    }
    else if((pos = strstr(low, "flush()")) != NULL) {
        strcpy(buf, "\tfflush(file);\n");

        clow = (char*)malloc(sizeof(buf));
        strcpy(clow, buf);
        *clow_len = strlen(clow);

        p_option[p_num] = 4;
        p_num++;

        return clow;
    }
    else if((pos = strstr(low, ".write(")) != NULL) {
        char* ptr;
        strcpy(buf, "\tfputs(\""); //buf = fputs(

        strncpy(buf_, pos+13, 50); //buf_ = "2019 OSLAB\n");
        ptr = strtok(buf_, ");"); //ptr = "2019 OSLAB\n"
        strcpy(buf_, ptr); //buf_ = "2019 OSLAB\n"

        strcat(buf, buf_); //buf = fputs("2019 OSLAB\n"
        strcat(buf, ","); //buf = fputs("2019 OSLAB\n",
        strcat(buf, "file);\n"); //buf = fputs("2019 OSLAB\n", file);

        int len = strlen(buf); //to make len(int) to len_ch(char*)

        clow = (char*)malloc(sizeof(buf));
        strcpy(clow, buf);
        
        *clow_len = strlen(clow);

        p_option[p_num] = 5;
        p_num++;

        return clow;
    }
    else if((pos = strstr(low, "FileWriter ")) != NULL) {
        clow = (char*)malloc(sizeof(char));
        strcpy(clow, buf);
        *clow_len = strlen(clow);

        get_obj(low, 0);
        return clow;
    }
    else if((pos = strstr(low, class[0].constructor)) != NULL) {
        //new Stack();
        char* ptr = strtok(pos," ");
        ptr = strtok(NULL, " ");

                clow = (char*)malloc(sizeof(buf));
                strcpy(clow, "\t\t");
                strcat(clow, ptr);

                get_obj(low, 0); //Stack st = new Stack();
                *clow_len = strlen(clow); 
                return clow;
    }
    else if((pos = strstr(low, "null")) != NULL) {
        strncpy(pos, "NULL", 4);
        clow = (char*)malloc(sizeof(char));
        strcpy(clow, buf);
        *clow_len = strlen(clow);
        return clow;
    }
    else if((pos = strstr(low, "new")) != NULL) {
        char* ptr; 
        ptr = new_malloc(pos, buf); //ptr = (int*)malloc(sizeof(int)*STACK_SIZE);
        //stack = 
        char* new_name = strtok(low, "new");
        strcat(new_name, ptr);
        clow = (char*)malloc(sizeof(buf));
        strcpy(clow, new_name);
        *clow_len = strlen(clow);
        p_option[p_num] = 6;
        p_num++;
        return clow;
    }
    else if(search_obj(low)) {
        char* ptr = obj_method(low);
        clow = (char*)malloc(sizeof(buf));
        strcpy(clow, ptr);
        *clow_len = strlen(clow);
	
        return clow;
    }
    else{
        clow = (char*)malloc(sizeof(buf));
        strcpy(clow, low);
        *clow_len = strlen(clow);
	*clow_ch  = 1;//변화없음.
        return clow;
    }
}


