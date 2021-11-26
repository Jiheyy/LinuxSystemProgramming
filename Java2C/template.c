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
