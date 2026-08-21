/* $begin shellmain */
#include "csapp.h"
#include<errno.h>
#include<string.h>

#define MAXARGS  128
#define MAXQUEUE (MAXARGS*2)
#define BUILTIN_CD_ERROR 42
#define BUILTIN_SUCCESS 1
#define BUILTIN_IGNORE 2
#define NOT_BUILTIN 0
/* Function prototypes */
int eval(char* cmdline);
int parseline(char* buf, char** argv);
int builtin_command(char** argv);

typedef struct {
    char* Item[MAXQUEUE];
    int Front;
    int Rear;
    int size;
}Queue;
void initQueue(Queue* q) {
    q->size = 0;
    q->Front = 0;
    q->Rear = -1;
}
void EnQueue(Queue* q, char* data) {
    if (q->size >= MAXQUEUE) {
        printf("QUEUE IS FULL, CANT ENQUEUE\n");
        return;
    }
    q->Rear = (q->Rear + 1) % MAXQUEUE;
    q->Item[q->Rear] = data;
    q->size++;
    return;
}
char* DeQueue(Queue* q) {
    if (q->size == 0) {
        printf("QUEUE IS EMPTY, CANT DEQUEUE");
        return NULL;
    }
    char* data;
    data = q->Item[q->Front];
    q->Front = (q->Front + 1) % MAXQUEUE;
    q->size--;
    return data;
}

//"&&"로 연결된 여러 명령어를 쪼개고, 앞 공백을 제거해서 큐에 차례대로 넣은 후, 큐에 몇 개 들어갔는지 반환하는 함수
/*int ParsingUseStr(Queue* Q, char* command, char* parsing) {
    initQueue(Q);
    char* com1;
    char* com2;
    command[strlen(command) - 1] = ' ';//마지막 엔터 지우기
    com1 = strtok(command, parsing);//&&로 쪼개는법.
    while (com1 != NULL) {
        while (*com1 == ' ') {
            com1++;
        }
        EnQueue(Q, com1);
        com2 = strtok(NULL, parsing);//NULL에 대해 하면 이전값에 이어서 쪼갬
        com1 = com2;
    }
    return Q->size;
}
*/
int split_commands(Queue* q, char* input, const char* delimiter) {
    initQueue(q);
    input[strcspn(input, "\n")] = ' '; // '\n' 제거

    char* token = strtok(input, delimiter);
    while (token) {
        while (*token == ' ') token++; // 앞 공백 제거
        if (*token != '\0') EnQueue(q, token);
        token = strtok(NULL, delimiter);
    }
    return q->size;
}

//입력을 받아 &&로 명령어를 나눈 뒤, 순서대로 실행하다가 실패하면 나머지를 무시
int main()
{
    char cmdline[MAXLINE]; /* Command line */
    /*Queue andParsingQueue;
    int andParsingNumber = 0;
    int nowCom = 0;
    char* cmdAfterAndParsing;
    */
    Queue cmdQueue;
    int command_count = 0;
    int current = 0;
    char* current_command;

    while (1) {
        /* Read */
        printf("CSE4100-SP-P2> ");
        fgets(cmdline, MAXLINE, stdin);
        if (feof(stdin))
            exit(0);

        /* Evaluate */
        command_count = split_commands(&cmdQueue, cmdline, "&&");//반환값은 큐 사이즈임.
        current = 0;
        while (current++ < command_count) {
            if (cmdQueue.size == 0)break;
            current_command = DeQueue(&cmdQueue);
            if (eval(current_command) != 0) { //0을 반환하면 성공, 아니면 실패
                while (cmdQueue.size > 0) {//수행중에 오류잇으면 큐 비우고 끝내
                    DeQueue(&cmdQueue);
                }
                break;
            }
        }
    }
}
/* $end shellmain */

/* $begin eval */
/* eval - Evaluate a command line */

int eval(char* cmdline)
{
    char* argv[MAXARGS]; /* Argument list execve() */
    char buf[MAXLINE];   /* Holds modified command line */
    int bg;              /* Should the job run in bg or fg? */
    pid_t pid;           /* Process id */

    int child_status;
    strcpy(buf, cmdline);
    bg = parseline(buf, argv);
    //argv에 대해 따옴표 제거해주기

    if (argv[0] == NULL)
        return 0;   /* Ignore empty lines */
    //0: 내장 명령어 아님 → 외부 명령어 실행, 1: 내장 명령어 실행 성공, 42 : 내장 명령어(cd) 실행 실패
    int builtin_result = builtin_command(argv);
    if (builtin_result == 0) { 
        if ((pid = Fork()) == 0) { //자식
            if (execvp(argv[0], argv) < 0) {
                printf("%s: command not found.\n", argv[0]);
                exit(EXIT_FAILURE);
            }
        }
        else {
            Waitpid(pid, &child_status, 0); //부모
            if ((WIFEXITED(child_status) && WEXITSTATUS(child_status)) != 0) {
                return -1;
            }
        }
    }
    else if (builtin_result == BUILTIN_CD_ERROR) {//cdError (빌트인에서 오류 뱉어냈으면)
        return -1;
    }
    return 0;
}
/* If first arg is a builtin command, run it and return true */
int builtin_command(char** argv)
{
    if (argv[0] == NULL) return NOT_BUILTIN;

    if (!strcmp(argv[0], "exit")) {
        exit(0);
    }
    if (!strcmp(argv[0], "&")) { /* Ignore singleton & */
        return BUILTIN_IGNORE;
    }
    //사용자가 그냥 cd만 입력한 경우 환경변수 HOME 디렉토리로 이동 시도, 실패하면 에러 메시지 출력 후 return 9
    if (!strcmp(argv[0], "cd")) {
        int cd_status;

        if (argv[1] == NULL || strlen(argv[1]) == 0) {
            cd_status = chdir(getenv("HOME"));
            if (cd_status != 0) {
                fprintf(stderr, "cd: No such file or directory\n");
                return BUILTIN_CD_ERROR;
            }
            return BUILTIN_SUCCESS;
        }
        //먼저 HOME으로 이동 후 다음 경로로 상대이동
        if (!strncmp(argv[1], "~", 1)) {
            cd_status = chdir(getenv("HOME"));
            if (cd_status != 0) {
                fprintf(stderr, "cd: %s: No such file or directory\n", argv[1]);
                return BUILTIN_CD_ERROR;
            }
            if (strlen(argv[1]) >= 3) {
                cd_status = chdir(argv[1] + 2);
                if (cd_status != 0) {
                    fprintf(stderr, "cd: %s: No such file or directory\n", argv[1]);
                    return BUILTIN_CD_ERROR;
                }
            }
            return BUILTIN_SUCCESS;
        }
        //그 외 일반적인 cd <경로> 입력
        else {
            cd_status = chdir(argv[1]);
            if (cd_status != 0) {
                fprintf(stderr, "cd: %s: No such file or directory\n", argv[1]);
                return BUILTIN_CD_ERROR;
            }
        }
        return BUILTIN_SUCCESS;
    }
    return NOT_BUILTIN;                    /* Not a builtin command */
}
/* $end eval */

/* $begin parseline */
/* parseline - Parse the command line and build the argv array */
int parseline(char* buf, char** argv) {
    char* delim; /* Points to first space delimiter */
    int argc;    /* Number of args */
    int bg;      /* Background job? */

    buf[strlen(buf) - 1] = ' ';   /* Replace trailing '\n' with space */
    while (*buf && (*buf == ' ')) /* Ignore leading spaces */
        buf++;

    /* Build the argv list */
    argc = 0;
    while ((delim = strchr(buf, ' '))) {
        /*if (*buf == '"' && delim > buf && *(delim - 1) == '"' && delim - buf > 1) {//양끝 쌍따옴표 안 인자 하나로 처리
            buf++;
            *(delim - 1) = '\0';
        }
        */
        if (*buf == '"') {
            buf++;                        // 앞쪽 쌍따옴표 건너뛰기
            delim = strchr(buf, '"');     // 다음 쌍따옴표까지 찾기
            if (delim != NULL) {
                *delim = '\0';            // 뒤쪽 따옴표 제거
                argv[argc++] = buf;       // 따옴표 제거된 인자 저장
                buf = delim + 1;          // 다음 위치로 이동
                while (*buf && *buf == ' ') buf++; // 공백 넘기기
                continue;                 // 다음 인자 처리로 이동
            }
        }
        argv[argc++] = buf;
        *delim = '\0';
        buf = delim + 1;
        while (*buf && (*buf == ' ')) /* Ignore spaces */
            buf++;
    }
    argv[argc] = NULL;

    if (argc == 0) /* Ignore blank line */
        return 1;

    /* Should the job run in the background? */
    if ((bg = (*argv[argc - 1] == '&')) != 0) { //백그라운드 작업이면 &를 제거
        argv[--argc] = NULL;
    }

    return bg;
}
/* $end parseline */