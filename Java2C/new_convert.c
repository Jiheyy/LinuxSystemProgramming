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

#include <template.c>

#define MAX_OBJ 3
#define MAX_CLASS 3
#define P_OPTION_NUM 30
#define BUFFER_SIZE 1024 
#define SECOND_TO_MICRO 1000000


char* create_c(char * filename) {
	int len = strlen(filename);
	
	strcpy(filename, ".c");

	return filename;
}

char* create_exe(char * filename) {
	int len = strlen(filename);
	
	strcpy(filename, "_Makefile");

	return filename;
}

FILE * openfile(char * file) {

	FILE *fp = NULL;

	// check whether it exits
	if ((fp = fopen(file, "r") == NULL)) {
		printf("no file exist : %s\n", filename);
		exit(1);
	}
	return fp;
}


// q1.java -> q1
char* getfilename(char * file) {
	printf("getfilename");
	int len = strlen(file);
	char* filename;

	char * pos = strtok(file, ".java");
	filename = pos;	
	while(pos != NULL) {
		pos = strtok(NULL, ".java");
		if(pos != NULL && strcmp(pos,".java") != 0)
			filename = pos;
	}
	return filename;

}

// public class q1{ +2
// BUT!! what if class definition looks like
// public class q1 { 
void class_setting(int class_no, char *buf, int linelen) {
	char * ptr = strtok(buf, " ");
	
	while (strcmp(ptr, "{") >= 0) 
		ptr = strtok(NULL,  " ");
	

	strtok(ptr, "{");
	class[class_no].name = (char*)malloc(sizeof(ptr));
	strcpy(class[class_no].name, ptr);

	// q1 new();
	// constructor > 생성자도... 저장..? 굳이/..?

	//	init
	class[class_no].num_obj = 0;
	class[class_no].linelen = linelen;

}



void readfile(FILE * fp) {

	int row = 0;
	int class_no = 0;
	char *line[100] // what if longer than 100 lines?
	char buf[BUFFER_SIZE];

	while(!feof(fp)) 
     {
         fgets(buf, BUFFER_SIZE, fp);
         line[row] = (char *)malloc(sizeof(buf));
         strcpy(line[row], buf);

		// class definintion
         if(strstr(buf, "class") != NULL) {
			
             class_setting(class_no ,buf, linelen);
             if(class_no >= 1) //class시작전 지우기
                for(i=1; i<linelen; i++)
                    if(strcmp(low[linelen-i],"\n")){
                        strcpy(low[linelen-i], " ");
                        break;
                    }class_no++;
         }row++;
     }
     fclose(fp);
}

	

int main(int argc, char* argv[]) {

    struct timeval begin_t, end_t;
    FILE *fp = NULL;
    char *filename, cfile, exefile ;

    //gettimeofday(&begin_t, NULL);
	fp = openfile(argv[1]);
    filename = getfilename(argv[1]);
	cfile = create_c(filename);
	exefile = create_exe(filename);


	// read file one by one
	readfile(fp)
	printf("%s\n", filename);

} 
