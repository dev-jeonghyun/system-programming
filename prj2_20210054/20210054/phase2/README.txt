[System Programming Project 2 - Phase 2]

- csapp.{c,h}
    Provided by CS:APP3e textbook
    Includes wrapper functions for Unix system calls

- myshell.{c,h}
    MyShell 구현 파일
    Phase 2 기능이 포함된 쉘

[구현한 주요 기능]

1. 기본 명령어 실행
    - Phase 1과 동일하게 ls, cat 등 외부 명령 실행 가능
    - 내부 명령어: cd, exit 지원

2. 파이프 (|) 처리
    - ls | grep abc, cat file.txt | grep test | sort 와 같은 명령어 체인 처리 가능
    - fork()를 통해 각 명령어마다 자식 프로세스 생성
    - pipe(), dup2()를 이용해 앞 명령의 출력을 다음 명령의 입력으로 연결
    - 마지막 명령을 제외한 프로세스는 부모가 wait 하지 않음, 마지막 명령은 wait 처리

3. 다중 파이프 처리
    - cat file.txt | grep abc | sort -r 처럼 여러 개의 pipe 연결 가능
    - 재귀 혹은 반복 구조를 사용해 파이프 구간마다 처리

[컴파일 및 실행 방법]

$ make
$ ./myshell

[테스트 예시 명령어]

CSE4100-SP-P2> ls | grep myshell
CSE4100-SP-P2> cat testfile.txt | grep -i abc | sort -r
CSE4100-SP-P2> echo "abc" | cat -n

[정리]

- make clean: 생성된 실행파일 및 오브젝트 삭제
- redirection(>, <)은 Phase 2에서는 구현하지 않음
- pipe 처리에 핵심적으로 사용된 system call: pipe(), fork(), dup2(), execvp()

