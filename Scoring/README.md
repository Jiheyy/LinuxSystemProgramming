# Scoring
scoring program ( linux system programming)


# 프로젝트 개요
Ssu_score 은 정답 파일을 기준으로 학생들이 제출한 답안 파일을 채점하는 프로그램이다.  이 프로젝트의 목표는 유닉스/리눅스 컴퓨팅 환경에서 제공하는 파일 입출력, 파일 속성 디렉토리에 관한 시스템 호출 함수와 라이브러리 함수를 이용한 프로그램을 작성하여 시스템 프로그램 설계 및 응용 능력을 향상시키는 것이다. 

# 설계
본 프로그램은 학생의 정보를 담고있는Std 구조체와 답안의 정보를 담고 있는 Q 구조체를 가지고 있다. 
## Std 구조체
Std 구조체 내에는 학생의 학번이 id 에 문자열 형태로 저장이 되고, 학생의 폴더의 경로를  path에 저장하고 있다. 또한 학생이 제출한 답안들이 answer에 문제의 순서대로 저장이 되어있다. 학생의 답안을 채점한 결과 각 문제를 맞췄는 지 여부가 card에 맞춘 경우 1, 틀린 경우 0으로 저장이 된다.
struct Std {
    char id[10];
    char path[100];
    char *answer[100];
    int card[100];
};
struct Std std[22];

## Q 구조체
Q 구조체 내에는 문제의 이름이 name에 문자열 형태로 저장이 된다. 각 문제의 답을 가지고 있는 .txt또는 .stdout 파일의 경로가 path에 저장이 된다. 또한 각 문제의 점수들이 score에 float형태로 저장이 된다. 각 문제의 답안의 개수를 answer_num에 int 형태로 저장을 하고 해당 문제의 정답들을 answer에 저장을 한다.
struct Q {
    char name[30];
    char path[30];
    float score;
    int blank_q; 
    int answer_num;
    char *answer[100];
};
struct Q q[100];

# 테스트 및 결과
## without option
![1](https://user-images.githubusercontent.com/54221681/138823027-a23a2c26-2439-423b-8e9f-8333dec123f9.PNG)
## score_table.csv 가 있을 경우
![2](https://user-images.githubusercontent.com/54221681/138823065-ddbb9e7b-dd3b-4416-8399-1ade0b6f9ded.PNG)
## ANSdir 에 score_table.csv 와 score.csv 자동 생성
![3](https://user-images.githubusercontent.com/54221681/138823291-af4a81d3-052c-4b3b-a5d5-2539590a0006.PNG)
- score.table.csv 
- ![4](https://user-images.githubusercontent.com/54221681/138823297-aa3cb672-5cd5-45f4-b826-f821a9772f33.PNG)
- score.csv 
- ![5](https://user-images.githubusercontent.com/54221681/138823298-9f8bb529-a80f-4a47-972d-3f0e21d1e117.PNG)
## -t
![t](https://user-images.githubusercontent.com/54221681/138823304-3d96a632-141b-4c80-805b-c1e8cc383833.PNG)
## -p
![p](https://user-images.githubusercontent.com/54221681/138823562-d97e89e2-8892-4bcb-8941-7aa987889399.PNG)
## -h
![h](https://user-images.githubusercontent.com/54221681/138823309-9c16c9d0-b74d-4d46-99bb-041540a42e8a.PNG)
## 각 학생 폴더에 실행 파일 자동생성 및 실행결과 .stdout 저장
![10](https://user-images.githubusercontent.com/54221681/138823312-95cbc8ce-0a5f-4d84-92cd-381f4f2050c0.PNG)

자세한 함수별 설명은 report.docx 참고
