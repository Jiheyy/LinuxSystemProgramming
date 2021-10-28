#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <pthread.h>

#define BUFFER_SIZE 5000 
#define FILE_LEN 255
#define MIN_PERIOD 5
#define MAX_PERIOD 10
#define MIN_TIME 60
#define MAX_TIME 1200
#define MAX_BACKUP_FILE (int)(MAX_TIME / MIN_PERIOD)
#define MIN_NUM 1
#define MAX_NUM 100
#define MAX_FILE_DIR 10 //-d 옵션 디렉토리 안 파일 최대 개수

typedef struct backup_list {

    char path_[BUFFER_SIZE];
    int time;
    char time_[BUFFER_SIZE];
    int size;
}backup_list;

typedef struct node {

    char path[BUFFER_SIZE]; //파일의 절대경로
    char path_[BUFFER_SIZE]; //파일의 상대경로
    int period; //백업 주기
    int option[BUFFER_SIZE]; //백업 옵션
    char option_s[BUFFER_SIZE]; 
    pthread_t tid; //thread
    time_t mtime;//mtime
    int time; //time for -t option
    int n_num; //number for -n option
    char d_path[MAX_FILE_DIR][BUFFER_SIZE];
    struct backup_list files[MAX_BACKUP_FILE];
    struct node *next;
}node;

typedef struct list {

    struct node *cur;
    struct node *head;
    struct node *tail;
    int node_num;
}list;



char *dirpath;
char *curpath;
char logpath[BUFFER_SIZE];
int period;
char filename[BUFFER_SIZE];
list *l;

void changename(char* file, char* newfile);
void gettime(char* day, char* time, int* sec);
void gettime_bf(backup_list *bf);
void prompt();
void add_command(void* command);
void remove_command(void* command);
void compare_command(void* command);
void recover_command(void* command);
void list_command(void* command);
void vi_command(void* command);
void ls_command(void* command);
void exit_command();
char* getfilename(char* command);
int getperiod(char* command);
int check_option(char* command);
void do_option(int num, char* option[]);
void logfile();
void createNode(char* path, int period, char* option[], int d_option);
void createNode_d(char* path, int period, char* option[], int d_option);
void deleteLastNode();
void printNode(); 
int searchNode(char* filename); 
void removeNode(int num);
char* optblock_to_string();
void* backup(void* num);
int searchBF(char *filename, backup_list *bf); 
void clean_up(void *arg) {
}

pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lmu = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char* argv[]) {


    DIR* dir;
    if(argc == 1) { 
	//인자가 없으면 current working dir 밑에 백업 디렉토리 생성
	dirpath = getcwd(NULL, BUFFER_SIZE);
	strcat(dirpath, "/backup");
	curpath = getcwd(NULL, BUFFER_SIZE);
	strcat(curpath, "/");
	sprintf(logpath, "%s/log",dirpath);
	mkdir("backup", 0777);
    }
    else if(argc == 2) {
	if(opendir(argv[1]) == NULL) {
	//인자로 입력받은 디렉토리를 찾을 수 없으면 usage출력 후 프로그램 종료
	//인자로 입력받은 디렉토리가 디렉토리 파일이 아니라면 usage 출력 수 프로그램 종료
	//인자로 입력받은 디렉토리의 접근 권한이 없는 경우 usage 출력 후 프로그램 종료

		fprintf(stderr, "Usage\n");
		exit(1);
	}
    	else { 
		dirpath = argv[1];
		curpath = getcwd(NULL, BUFFER_SIZE);
		strcat(curpath, "/");
		sprintf(logpath, "%s/log",argv[1]);
	}
    }
    else if(argc > 2) {
	//인자가 2개 이상이면 usage 출력 후 프로그램 종료
	fprintf(stderr, "Usage\n");
	exit(1);
    }
	//링크드리스트 할당
	l = (list*)malloc(sizeof(list));
	l->cur = NULL;
	l->head = NULL;
	l->tail = NULL;
	l->node_num = -1;


/////////프롬프트 출력////////

    prompt();
}

void prompt() {
    char command[BUFFER_SIZE];
    pthread_t tid1, tid2, tid3, tid4;
    int th_id;
    int result;
    char* line_p;
    
    /*if(system("export PS1=\"20162467>\"") == 0) {
	fprintf(stderr, "system error\n");
	exit(1);
    }*/

    //exit 명령이 입력될 때까지 위에서 지정한 실행 가능 명령어를 입력받아 실행
    while(strncmp(command, "exit",4)) {
    	printf("20162467>");
	fgets(command, BUFFER_SIZE, stdin);
	if((line_p = strchr(command, '\n')) != NULL) * line_p = '\0';

    	if(strncmp(command, "vi", 2)==0) {
	    vi_command((void*)command);
	}
	else if(strncmp(command, "ls", 2)==0) {
	    ls_command((void*)command);
	} 
	else if(strncmp(command, "add", 3)==0) {
	    add_command((void*)command);
	}
    	else if(strstr(command, "exit")!=NULL) {
		exit_command((void*)command);
		exit(1);	
	}
    	else if(strncmp(command, "remove", 6)==0) {
	    remove_command((void*)command);
	}

	else if(strncmp(command, "compare", 7) == 0) {
	    compare_command((void*)command);
	}

	else if(strncmp(command, "recover", 7) == 0) {
	    recover_command((void*)command);
	}
    	else if(!strncmp(command, "list ", 5)==0) {
	    list_command((void*)command);
	}    
	
    	else { 
		//이외의 명령어 입력 시 에러 처리 후 프롬프트로 넘어감.
		fprintf(stderr, "109 error\n");
		exit(0);
    	}

	logfile(); //logfile에 기록
    }
    for(int i=0; i< BUFFER_SIZE; i++)
	    command[i] = '\0';


}

void add_command(void* command){
//add <filename> [period] [option]
    //add test1.txt\n하면 에러뜸...
    char* cmd = command;
    int i = 0;
    char* ptr = strtok(cmd, " ");
    char* add[10];
    char* option[3];
    int j;
    int d_option;
    while(ptr != NULL) {

	add[i] = (char*)malloc(sizeof(ptr));
	strcpy(add[i], ptr);

	ptr = strtok(NULL, " ");
	i++;
    }

    if(i< 2) {
	fprintf(stderr,"usage: add <filename> [period] [option]\n");
	return;
    } //add -d dirname period -n 5 -t 6
    if(strstr(add[1], "-d") != NULL) {
	printf("-d option\n");
	if(opendir(add[2]) == NULL) {
	    fprintf(stderr, "dir error\n");
	    return;
	}
	strcpy(filename, add[2]); //dir 이름 filename 저장
	period = atoi(add[3]);
	d_option = 1; //-d옵션 들어왓음
    }
    else {
	
	d_option = 0;
    if(access(add[1], 00) < 0) {
	fprintf(stderr,"there is no file name %s\n", add[1]);
    	return;
    }

    if(*add[1] == '/') {
	strcpy(filename, add[1]);
    }
    else {
	sprintf(filename, "%s%s", curpath, add[1]);
    }

    if(searchNode(filename) != -1) {
	fprintf(stderr, "there is filename in backup list\n");
    	return;
    }

   period = atoi(add[2]);
}
   if(period < 5 || period > 10) {
       fprintf(stderr, "error : period error\n");
   	return;
   }

   if(i>3) {
   	do_option(i, add); //do option //add->option설정
   }
   else {
       for(j=0; j<3; j++)
	   add[j] = "0";
   }
if(d_option == 1)
    createNode_d(filename, period, add, d_option);
    else
   createNode(filename, period, add, d_option);

   
    return;
}
void remove_command(void* command){

    char* cmd = command;
    int i = 0;
    char* ptr = strtok(cmd, " ");
    char* rmv[2];
    int exist;
    char day[BUFFER_SIZE];
    char time[BUFFER_SIZE];
    char buf[BUFFER_SIZE];
    int sec;

    //node *p = l->head;

    while(ptr != NULL) {

	rmv[i] = (char*)malloc(sizeof(ptr));
	strcpy(rmv[i], ptr);

	ptr = strtok(NULL, " ");
	i++;
    }
//if it's not remove -a || remove filename
    if(i!=2) {
	fprintf(stderr, "error\n");
	return;
    }
    //옵션이 있을 경우
    //remove -a
    if(!strncmp(rmv[1], "-a", 2)) {
	FILE* fp = fopen(logpath, "a+");
	printf("-a option\n");
	node* p = l->head;
	
	while(p!=NULL) {

    	    pthread_cancel(p->tid);
	    pthread_cond_signal(&cond);

	    pthread_join(p->tid, NULL);

	gettime(day, time, &sec);
	sprintf(buf,"[%s %s] %s deleted\n", day, time, p->path_);
	fwrite(buf, strlen(buf), 1, fp);

	    p = p->next;
	}
    	l->head = NULL;
	fclose(fp);
    }
    //파일이 백업리스트에 없을 경우
    else if((exist = searchNode(rmv[1])) < 0) {

	fprintf(stderr," there is no file name %s in backup list\n", rmv[1]);
	return;
    }
    else {
	removeNode(exist); //파일 백업에서 삭제
    }

   
    	
    return;
}
void compare_command(void* command) {

    char* cmd = command;
    int i=0;
    char file[3][BUFFER_SIZE];
    //int f1, f2;
    struct stat f1;
    struct stat f2;
    off_t size1, size2;
    time_t time1, time2;

    char* cmp = strtok(cmd, " ");
    while(cmp != NULL) {

	strcpy(file[i], cmp);

	cmp = strtok(NULL, " ");
	i++;
    }
    if(i!=3) {
	fprintf(stderr, "input files have to be 2\n");
	return;
    }

    if(access(file[1], F_OK) < 0) {
	char buf[BUFFER_SIZE];
	strcpy(buf, file[1]);
	sprintf(file[1], "%s/%s", dirpath, buf);
    }
    if(stat(file[1], &f1) < 0) {
	fprintf(stderr, "%s error\n", file[1]);
	return;
    }
    	
    time1 = f1.st_mtime;
    size1 = f1.st_size;
    
    if(access(file[2], F_OK) < 0) {
	char buf[BUFFER_SIZE];
	strcpy(buf, file[2]);
	sprintf(file[2], "%s/%s", dirpath, buf);
    }
    if(stat(file[2], &f2) < 0) {
	fprintf(stderr, "f2 error\n");
	return;
    }
    
    time2 = f2.st_mtime;
    size2 = f2.st_size;
   
    if(time1 == time2 && size1 == size2)
	printf("두 파일은 같다.\n");
    else {
    	printf("mtime : %ld %ld\n", time1, time2);
    	printf("size : %ld %ld\n", size1, size2);
    }
}
void recover_command(void* command) {
    char* cmd = command;
    int i=0;
    int num;
    char *input[4];
    char filename[BUFFER_SIZE];
    backup_list bf[100]; //backup file list
    char* ptr = strtok(cmd, " ");
    while(ptr != NULL) {

	input[i] = (char*)malloc(sizeof(ptr));
	strcpy(input[i], ptr);
//	printf("%s\n", input[i]);

	ptr = strtok(NULL, " ");
	i++;
    }
    if(i > 2) {
	char *p;
	if(( p = strstr(input[2], "-n")) == NULL) {
	    fprintf(stderr, "there is no option name %s\n", input[2]);
	    return;
	}
    	if(i==3) {
	    fprintf(stderr, "there is no newfile name\n");
	    return;
	}
	if(!access(input[3], F_OK)) {
	    fprintf(stderr, "there is file named %s\n", input[3]);
	    return;
	}
	else {
	    if(*input[3] == '/') {
//		printf("start with filepath\n");
		strcpy(filename, input[3]);
	    }
	    else {
//		printf("start with partial path\n");
		sprintf(filename, "%s%s", curpath, input[3]);
	    }
	}


    }
    else {
	if(*input[1] == '/') {
//	    printf("start with filepath\n");
	    strcpy(filename, input[1]);
	}
	else {
//	    printf("start with partial path\n");
	    sprintf(filename, "%s%s", curpath, input[1]);
	}
    }

    if((num = searchBF(input[1], bf)) < 0) {
	fprintf(stderr, "there is no file for recover\n");
	return;
    }
    else {
	for(int j=0;j<num-1 ; j++) {
	    printf("%d. %8s %dbytes \n",j+1,bf[j].time_, bf[j].size);
	}
  	printf("Choose file to recover : ");
	char buf[4];
	fgets(buf, sizeof(buf), stdin);
	buf[strlen(buf)-1] = '\0';
	int num = atoi(buf);

	//cp bf[num].path_ filename
	char cp[BUFFER_SIZE];
	sprintf(cp,"cp %s/%s %s",dirpath, bf[num-1].path_, filename);
//	printf(":: %s\n", cp);
	system(cp);

	FILE* fp = fopen(filename, "r");

	while(!feof(fp)) {
		fgets(cp, sizeof(cp), fp);
	//	printf("%s\n", cp);
	}
	
    }


}

int searchBF(char *filename, backup_list *bf) {
    DIR* dir = NULL;
    struct dirent* entry = NULL;
    int i=0;
    struct stat sb;
    char path[BUFFER_SIZE];

    if((dir = opendir(dirpath)) == NULL) {
    
	fprintf(stderr, "open dir error\n");
	return -1;
    }
    while((entry = readdir(dir)) != NULL) {
	//printf("507 open %s\n", dirpath);
	if(strstr(entry->d_name, filename)!= NULL) {
	    if(!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) {
		i--;
		printf("skip\n");
		continue;
	    }
//	    printf("%d %s\n", i, entry->d_name);
	    strcpy(bf[i].path_, entry->d_name);
	    sprintf(path, "%s/%s", dirpath, entry->d_name);
	    stat(path, &sb);
	    bf[i].size = sb.st_size;
	    gettime_bf(&bf[i]);
	    i++;
	}
    }
    closedir(dir);
    return i;

}
void gettime_bf(backup_list *bf) {
    char name[BUFFER_SIZE];
    strcpy(name, bf->path_);
    char * ptr;
    ptr =strstr(name,"_");
    ptr = strtok(ptr, "_");

    strcpy(bf->time_, ptr);
}

void list_command(void* command) {
    node *p = l->head;
    while(p!=NULL) {
	printf("%s	,%d %s\n", p->path_, p->period,p->option_s);
	p= p->next;
    }

    return;
}
void vi_command(void* command) {
    system(command);
}
void ls_command(void* command) {
    system(command);
}

void exit_command(){

    node *p = l->head;

    while(p!=NULL) {
	pthread_cancel(p->tid);
	p=p->next;
    }
}

char* getfilename(char* command) {

    char* filename = strtok(command, " ");

    filename = strtok(NULL, " ");
     strtok(filename, " ");
    
   printf("154 filename %s\n", filename);


    return filename;
}

int getperiod(char* command) {

    int period;
    char* buf = strtok(command, " ");

    buf = strtok(NULL, " ");
    buf = strtok(NULL, " ");

   period = atoi(buf); 

    return period;
}

int check_option(char* command) {}

void do_option(int argc, char* option[]) {

    char c;
    char* buf_;
    int num;
    int time;
    int buf[4];
    int num_option = 0;

    for(int i=0; i<4; i++)
	buf[i] = 0;
	
    while((c = getopt(argc, option, "mn:t:d:")) != -1) {
	switch(c) {
	    case 'm' :
		buf[0] = 1;
		num_option++;
	
		
		break;
	    case 'n' :
	//	printf("n option\n");
		buf_ = optarg;
		if(buf_ == NULL) {
		    fprintf(stderr,"there is no num\n");
		    return;
		}
		num = atoi(buf_);
		//printf("583 %d\n", num);
		
		if(num <MIN_NUM ||num > MAX_NUM) {
		    fprintf(stderr, "number error\n");
		    return;
		}
		buf[1] = num;
		num_option++;
		break;
	    case 't' :
		buf_ = optarg;
		time = atoi(buf_);
		//time 이 정수형이 아닌 실수형 일경우 에러처리
		//입력이 없을 시 에러처리 후 프롬르트로 제어가 넘어감
		buf[2] = time;
		num_option++;
		break;
	    case '?' :
		printf("error");
		break;
	 /*   default:
		printf("default\n");
		break;*/
	}
    }
    for(int i=0; i<3; i++) {
	char tmp[BUFFER_SIZE];
	sprintf(tmp, "%d",buf[i]);
	strcpy(option[i],tmp);
    }
    /*for(int i=0; i<4; i++)
	printf("384 %d %s\n",i, option[i]);*/

}


void createNode_d(char* path, int period, char* option[], int d_option) {
    
    int i;
    void* num;
    node* newNode;

    DIR *d = opendir(path);
    struct dirent* file = NULL;
    while((d = opendir(path)) == NULL) {
	fprintf(stderr, "unavailble to opendir %s\n", path);
	return;
    }
    i=0;
    while((file = readdir(d)) != NULL) {
	if(!strcmp(file->d_name, ".") || !strcmp(file->d_name, "..")) {
	    continue;
	}
	else {
	    newNode = (node*)malloc(sizeof(node));
	    strcpy(newNode->path, file->d_name);
	    strcpy(newNode->path_, path);
	    strcat(newNode->path_, "/");
	    strcat(newNode->path_, newNode->path);
	    newNode->period = period;
	    strcpy(newNode->option_s, "-d ");
	    
	    for(int i=0; option[i]!= '\0'&& i<4; i++) {
		newNode->option[i] =  atoi(option[i]);
	    }
	    //m option
	    if(newNode->option[0] == 1) {
		strcat(newNode->option_s, "-m ");
		struct stat statbuf;
		if((stat(newNode->path_, &statbuf)) <0) {
		    fprintf(stderr, "stat error\n");
		    return;
		}
		newNode->mtime = statbuf.st_mtime;
	    }else
		newNode->mtime = 0;
	    //t option
	    if(newNode->option[2] !=0) {
		strcat(newNode->option_s, "-t ");
		newNode->time = newNode->option[2];
	    }else
		newNode->time = 0;
	    // n option
	    if(newNode->option[1] !=0) {
		strcat(newNode->option_s, "-n ");
		newNode->n_num = newNode->option[1];
	    }else
		newNode->n_num =0;
	    //첫노드
	    if(l->head == NULL && l->tail == NULL && l->node_num == -1)
		l->head = l->tail = newNode;
	    else {//추가
		l->tail->next = newNode;
		l->tail = newNode;
	    }
	    l->node_num++;

	    if(searchNode(newNode->path_) < 0) {
		fprintf(stderr,"there is a same name of file in the backup list\n");
		return;
	    }
	    num = &l->node_num;
	    pthread_create(&newNode->tid, NULL, backup, num);
	    i++;
	}
    }
	    
	
}
void createNode(char* path, int period, char* option[], int d_option) {
    
    int i;
    void* num;
    node* newNode;
    newNode = (node*)malloc(sizeof(node));
    
    //strcpy(newNode->path , path);
    
    strcpy(newNode->path_, path);

    
    newNode->period = period;
    for(int i=0; i<3; i++) {
    	newNode->option[i] =  atoi(option[i]);
    }


    strcpy(newNode->option_s, " ");

    if(newNode->option[0] == 1) {
	struct stat statbuf;

	strcat(newNode->option_s, "-m ");
	if((stat(newNode->path_, &statbuf)) <0) {
	    fprintf(stderr, "stat error\n");
	    exit(1);
	}
	newNode->mtime = statbuf.st_mtime;
    }else
	newNode->mtime = 0;
    
    if(newNode->option[2] !=0) {
	strcat(newNode->option_s, "-t ");
	newNode->time = newNode->option[2];
    }else
	newNode->time = 0;

    if(newNode->option[1] !=0) {
	strcat(newNode->option_s, "-n ");
	newNode->n_num = newNode->option[1];
    }else
	newNode->n_num =0;
	    
    if(l->head == NULL && l->tail == NULL && l->node_num == -1) {
	l->head = l->tail = newNode;
    }
    else {
	l->tail->next = newNode;
	l->tail = newNode;
    }
    l->node_num++;
    //printf("772 %d\n", l->node_num);
    num = &l->node_num;
    if(pthread_create(&newNode->tid, NULL, backup, num) != 0) {
	fprintf(stderr,"why??\n");
    }
     
   // return;
}
void deleteLastNode() {
    node *p = l->head;
    while(p->next->next != NULL) 
	p = p->next;
    p->next = p->next->next;
    l->tail = p;
}
void printNode() {
    node*p = l->head;
    while(p!= NULL) {
	p= p->next;
    }

}
int searchNode(char* filename) {
    int i = 0;
    int len;
    node *p = l->head;
    while(p!= NULL) {
	len = strlen(filename);
	  //  printf("compare %s with %s %d\n", filename, p->path_, len);
	//printf("444 %d\n",strncmp(filename, p->path_, len));
	if(strstr(p->path_, filename)!=NULL) {
	  //  printf("%d\n", i);
	    return i;
	}
	i++;
	p= p->next;
    }
//printf("%s return -1\n", filename);
    return -1;
}
void removeNode(int num) {

    FILE* fp = fopen(logpath, "a+");
    int i=0;
    char time[BUFFER_SIZE];
    char day[BUFFER_SIZE];
    char buf[BUFFER_SIZE];
    int sec;
    node *p = l->head;
    //첫번째 노드 지움
    if(num == 0) {
	char* oldhead = l->head->path_;
	pthread_cancel(l->head->tid);
	pthread_cond_signal(&cond);
//printf("510\n");
	pthread_join(l->head->tid, NULL);
	l->head = p->next;
	gettime(day, time, &sec);
	sprintf(buf,"[%s %s] %s deleted\n", day, time, oldhead);
	fwrite(buf, strlen(buf), 1, fp);
	fclose(fp);
	//히니빆에 안남은 첫번째 노드를 지움
	if(l->node_num == 0) {
	    //printf("everynode is deleted\n");
	    l->head = NULL;
	    l->tail = NULL;
	}
	l->node_num --;
	return;
    }
    while(p!= NULL && i<=num) {
	if(i+1 == l->node_num){
	    pthread_cancel(l->tail->tid);
	    pthread_cond_signal(&cond);

	    pthread_join(l->tail->tid, NULL);

	    free(l->tail);
	    l->tail = p;
	    p->next = NULL;
	    gettime(day, time, &sec);
	sprintf(buf,"[%s %s] %s deleted\n", day, time, p->path_);
	fwrite(buf, strlen(buf), 1, fp);
	fclose(fp);
	    l->node_num--;
//	printf("860\n%d\n", l->node_num);
	    return;
	}
	if(i+1 == num) {
    	    pthread_cancel(p->next->tid);
	    pthread_cond_signal(&cond);

	    pthread_join(p->next->tid, NULL);

	    p->next = p->next->next;
	    l->node_num--;
	gettime(day, time, &sec);
	sprintf(buf,"[%s %s] %s deleted\n", day, time, p->path_);
	fwrite(buf, strlen(buf), 1, fp);
	fclose(fp);
	    return;
	}

	p = p->next;
	i++;
    }
    return;
}
//backup 할 파일을 백업 디렉토리에 추가한다.
void* backup(void* num) {/////////////////자원정리 함수 추가//////////
    FILE *fp = fopen(logpath, "a+");

    char buf[BUFFER_SIZE];
    char day[BUFFER_SIZE];
    char time[BUFFER_SIZE];
    char bufile[BUFFER_SIZE];
    int sec;
    int i = 0;
    int j;
    int no = *(int*)num;

    node* file = l->head;
    for(i; i<no; i++) {
	file = file->next;
    }
    char *path = file->path;
    char* path_ = file->path_;
    int period = file->period;
    time_t mtime = file->mtime;
    int t_time = file->time;
    int n_num = file->n_num;
    char command[BUFFER_SIZE];
    pthread_cleanup_push(clean_up, (void*)file);

    sprintf(command, "cp %s %s/", path_, dirpath);
    //strcat(command, dirpath);
    //strcat(command, " ./backup/");
    //strcat(command, path);

    gettime(day, time, &sec);
    sprintf(buf, "[%s %s] %s added\n",day, time, file->path_);

    fwrite(buf, strlen(buf), 1,fp);
    fclose(fp);
    int bfn = 0;
    int m = 0;
    while(1) {
	if(fopen(logpath, "a+") < 0) {
	    fprintf(stderr, "log file error\n");
	    
	}
	char cmd_buf[BUFFER_SIZE];
	strcpy(cmd_buf, command);
    	sleep(period);
	//moption 실행
    	if(mtime != 0) {
		struct stat statbuf;
		stat(path_, &statbuf);
		if(m == 0) {
		    mtime =statbuf.st_mtime;
		    m++;
		}else if(mtime == statbuf.st_mtime) {
	//    		printf("there was nothing happend\n");
		}
		else mtime = statbuf.st_mtime;
    	}
	    
	gettime(day, time, &sec);
	sprintf(bufile, "%s_%s%s",file->path_,day, time); //절대경로_날짜
    	sprintf(buf, "[%s %s] %s generated\n", day, time, bufile);

	char ch_bufile[BUFFER_SIZE];
	changename(bufile, ch_bufile); //절대경로_날짜를 파일의 이름으로 저장하기 위해서 /을 =로 바꾼 ch_bufile을 이름으로  저장

	sprintf(file->files[bfn].path_, "%s/%s", dirpath, ch_bufile);
	file->files[bfn].time = sec;

	//cp file->path_ ./backup/file->path_daytime
    	strcat(cmd_buf, ch_bufile);
	if(strlen(bufile) > 255) {
	    fprintf(stderr,"this file name is so long\n");
	    pthread_exit(NULL);
	}
	system(cmd_buf);
	fwrite(buf, strlen(buf), 1,fp);
	fclose(fp);
	bfn++;
	//toption 실행
	if(t_time != 0) {
	    for(i=0; i<bfn; i++) {
		gettime(day, time, &sec);
	    	if(sec - t_time >= file->files[i].time) 
		    if(strstr(file->files[i].path_,"#")== NULL) {
	//	    printf("%d - %d >= %d\n",sec, t_time, file->files[i].time);
			char rm[BUFFER_SIZE];
			sprintf(rm,"rm %s", file->files[i].path_);
	//		printf("%s\n", rm);
			system(rm);
			strcpy(file->files[i].path_, "#");//tombstone
			file->files[i].time = MAX_TIME;
		    }
	    }

	}
	//noption 실행
	if(n_num != 0) {
	    if(bfn > n_num) {
		for(i=0,j=0; bfn-n_num > i; i++) {//bfn 5 ..--0
		    if(strstr(file->files[i].path_, "#")== NULL) {
			char rm[BUFFER_SIZE];
			sprintf(rm, "rm %s", file->files[i].path_);
			strcpy(file->files[i].path_ ,"#"); //tombstone;
	//		printf("723 %s\n", rm);
			system(rm);
			file->files[i].time = MAX_TIME;
			j++;
			//bfn--;
		    }
	    	}
	    }
	}

    }
    pthread_cleanup_pop(0);

}
void changename(char* file, char* newfile) {

    strcpy(newfile, file);

    char* pch = strstr(newfile, "/");
    while(pch!=NULL) {

    	strncpy(pch, "=", 1);
    	pch = strstr(newfile, "/");
    }

}
void gettime(char* dy, char* tm, int *s) {
    time_t timer;
    struct tm *t;

    timer = time(NULL);
    t = localtime(&timer);

    char time[BUFFER_SIZE];
    char year[10]; 
	sprintf(year, "%02d",t->tm_year-100);
    char mon[10]; 
	sprintf(mon, "%02d",t->tm_mon+1);
    char day[10];
    sprintf(day, "%02d",t->tm_mday);
    char hour[10];
    sprintf(hour, "%02d",t->tm_hour);
    char min[10];
    sprintf(min, "%02d",t->tm_min);
    char sec[10];
    sprintf(sec, "%02d", t->tm_sec);

    sprintf(dy, "%s%s%s",year, mon, day);
    sprintf(tm,"%s%s%s", hour, min, sec);
    *s = (int)((t->tm_min*60) + t->tm_sec);
    
}
char* optblock_to_string() {}

void logfile(){}
