# LinuxSystemProgramming
19년도 1학기 숭실대 리눅스 시스템 프로그래밍 과제

# 설계 1번 : SSU_SCORE
- 학생 답안과 정답을 비교하여 자동으로 채점하는 설계 프로그램
- 빈칸 문제와 프로그램 문제가 있음
-  빈칸 문제 : 연산자의 방향과 띄어쓰기등을 고려하여 답안과 답이 같을 때 정답처리
-- a == b 와 b==a 정답
-- a >= b 와 b <= a 정답
-- a + b * c 와 (a + b) * c는 오답
# 프로그램 문제 : 프로그램 실행 결과가 답안 프로그램의 실행 결과와 일치 할때 정답 처리
- 띄어쓰기를 모두 제거한 후 한 글자 씩 정답 체크
- 프로그램 실행 시 답을 저장하는 파일을 출력을 쉘 리다이렉션으로 사용하는 것을 금지하여 dup2()를 이용한 표준 출력 리다이렉션을 이용
- 프로그램 실행이 너무 오래 지속되거나 무한루프로 끝나지 않는 것을 방지하기 위한 5초 제한이 존재
- 위 문제를 fork()와 쓰레드 사용을 배우기 전 이여서 프로그램 실행전 timeover.c 코드 생성, 컴파일(killer 생성) 실행하여 프로그램을 제어

# 설계 2번 : SSU_CONVERT
- java코드로 작성된 코드를 C코드로 변환하는 프로그램
- java 메소드에 대응되는 C의 함수들을 매칭 시켜 같은 코드가 되도록 변환
- 과제로 주어진 함수들만 처리하면 되는 과제였어서 약 9개정도의 라이브러리 메소드를 매칭하도록 설계
- 중괄호{} 및 if, for, while 사용에 따른 indent가 제대로 적용되도록 설계
- java의 클래스 정의부는 C에서는 구조체와 그에 관한 함수들이라고 생각하여 해당 부분에 대한 헤더파일을 따로 만들도록 설계

# 설계 3번 : SSU_BACKUP
- 실시간으로 백업 파일을 주기적으로 생성하는 프로그램
- 프로그램을 실행하면 미니 쉘이 실행되어 몇가지 명령을 수행할 수 있도록 설계
- 파일 하나를 백업시키면 설정된 주기로 계속 같은 파일을 파일명에 시간을 붙여서 생성
- 파일 하나당 하나의 쓰레드를 생성하여 해당 파일 백업을 담당
- 백업으로 생기는 파일의 갯수를 정하는 옵션이 있음
- 폴더를 백업시키는 옵션이 있음
- 폴더 옵션을 추가하면 하위 파일들과 폴더 까지도 전부 백업하도록 설계
- 현재 백업중인 파일들을 볼 수 있도록 설계
