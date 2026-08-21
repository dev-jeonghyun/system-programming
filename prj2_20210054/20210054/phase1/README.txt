[System Programming Project 2 - Phase 1]

- csapp.{c,h}
    Provided by CS:APP3e textbook
    Includes wrapper functions for Unix system calls

- myshell.{c,h}
    MyShell 구현 파일
    Phase 1 기능이 포함된 쉘

[구현한 주요 기능]

1. 기본 명령어 실행
    - 외부 명령어 실행 (ls, cat, touch 등)
    - 사용자 입력을 파싱하고 자식 프로세스에서 execvp()를 통해 실행

2. 내부 명령어 지원
    - cd: 디렉토리 이동
    - exit: 쉘 종료 (모든 자식 프로세스 종료 포함)

3. 파일/디렉토리 조작
    - mkdir, rmdir
    - echo, touch, cat

4. 프로세스 실행 구조
    - fork()로 자식 프로세스를 생성 후 exec()로 명령 실행
    - 부모 프로세스는 wait()로 자식 종료 대기

[컴파일 및 실행 방법]

$ make
$ ./myshell

[테스트 예시 명령어]

CSE4100-SP-P2> ls
CSE4100-SP-P2> mkdir testdir
CSE4100-SP-P2> cd testdir
CSE4100-SP-P2> touch file.txt
CSE4100-SP-P2> echo "hello" > file.txt
CSE4100-SP-P2> cat file.txt
CSE4100-SP-P2> cd ..
CSE4100-SP-P2> rmdir testdir
CSE4100-SP-P2> exit

[정리]

- make clean: 생성된 실행파일 및 오브젝트 삭제
- cd, exit는 부모 프로세스에서 직접 처리되고, 나머지는 자식 프로세스를 통해 실행된다.
