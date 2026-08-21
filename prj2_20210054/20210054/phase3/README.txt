[System Programming Project 2 - Phase 3]

- csapp.{c,h}
    Provided by CS:APP3e textbook
    Includes wrapper functions for Unix system calls

- myshell.{c,h}
    MyShell 구현 파일
    Phase 3 기능이 포함된 최종 쉘

[구현한 주요 기능]

1. 기본 명령 실행
    - 외부 명령어 실행 (ls, cat 등)
    - 내부 명령어: cd, exit

2. 파이프 처리
    - 명령어 연결 (ex. ls | grep abc)
    - 다중 파이프 지원 (ex. cat file | grep abc | sort -r)

3. 백그라운드 실행 (&)
    - 명령어 끝에 '&'가 있으면 백그라운드로 실행됨
    - 'command&', 'command &', 'command &' 모두 인식 가능

4. Job Control 기능
    - jobs: 실행 중이거나 중지된 백그라운드 작업 목록 출력
    - fg %n: n번 job을 foreground로 전환
    - bg %n: 중지된 job을 background로 재개
    - kill %n: n번 job 종료
    - job 번호는 '%' 없이도 인식 가능하도록 구현 가능

5. 시그널 처리
    - Ctrl+C (SIGINT): foreground 프로세스 종료
    - Ctrl+Z (SIGTSTP): 프로세스 중지 → jobs에 반영
    - SIGCONT: 중지된 프로세스 재개 (bg/fg에서 사용됨)

6. 잡 테이블 관리
    - 잡 번호, PID, 상태(Running/Suspended) 저장 및 추적
    - 각 잡에 대해 fg/bg/kill 등 시스템 콜 적용 가능

[컴파일 및 실행 방법]

$ make
$ ./myshell

[테스트 예시 명령어]

CSE4100-SP-P2> sleep 10 &
CSE4100-SP-P2> jobs
[1] running sleep 10
CSE4100-SP-P2> fg %1
CSE4100-SP-P2> sleep 10
(Ctrl+Z)
CSE4100-SP-P2> jobs
[1] suspended sleep 10
CSE4100-SP-P2> bg %1
CSE4100-SP-P2> fg %1
CSE4100-SP-P2> kill %1

CSE4100-SP-P2> cat testfile.txt | grep -i abc &

[정리]

- make clean: 실행파일 및 .o 파일 제거
- 주요 system call: fork(), execvp(), pipe(), dup2(), waitpid(), kill(), signal()
