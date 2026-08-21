/*
 echoserveri.c - An iterative echo server
 */
 /* $begin echoserverimain */
#include "csapp.h"
typedef struct {
    int ID;
    int left_stock;
    int price;
    int readcnt;
    sem_t mutex;
}item;

typedef struct {
    int maxfd;              // select()에 전달할 최대 fd
    fd_set read_set;        // 현재 관찰 중인 read fd 집합
    fd_set ready_set;       // select() 호출 후 준비된 fd 집합
    int nready;             // 준비된 fd 개수
    int maxi;               // clientfd 배열에서 가장 큰 인덱스
    int clientfd[FD_SETSIZE];   // 연결된 클라이언트들의 소켓 fd
    rio_t clientrio[FD_SETSIZE];// 각 클라이언트에 대한 RIO 버퍼
}pool;

typedef struct node {
    item item;
    struct node* left;
    struct node* right;
}node;

void echo(int connfd);
void init_pool(int, pool*);
void add_client(int connfd, pool* p);
void check_clients(pool* p);

void load_stock();
void save_stock();
void print_stock();
void sell(int a, int b);
int buy(int a, int b);
node* root;
void sigint_handler(int sig) {
    char sigbuf[MAXLINE];
    sigbuf[0] = '\0';
    //save_stock(root, sigbuf); 수정
    FILE* fp = Fopen("stock.txt", "w");
    print_stock(root, sigbuf);
    fputs(sigbuf, fp);
    Fclose(fp);
    exit(0);
}
// 시간 측정 시작
/*struct timeval start_time, end_time;
double total_elapsed;
int started = 0;
*/
int main(int argc, char** argv)
{
    Signal(SIGINT, sigint_handler);
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;  /* Enough space for any address */  //line:netp:echoserveri:sockaddrstorage
    static pool pool;

    char client_hostname[MAXLINE], client_port[MAXLINE];

    

    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }

    load_stock();

    listenfd = Open_listenfd(argv[1]);
    init_pool(listenfd, &pool);

    
    while (1) {
        // wait for listening / connected descriptor to become ready
        pool.ready_set = pool.read_set;
        pool.nready = Select(pool.maxfd + 1, &pool.ready_set, NULL, NULL, NULL);

        //if(FD_ISSET(STDIN_FILENO, &ready_set))

        if (FD_ISSET(listenfd, &pool.ready_set)) {
            clientlen = sizeof(struct sockaddr_storage);
            connfd = Accept(listenfd, (SA*)&clientaddr, &clientlen);
            /*
            if (!started) {
                gettimeofday(&start_time, NULL);
                started = 1;
            }
            */

            Getnameinfo((SA*)&clientaddr, clientlen, client_hostname, MAXLINE,
                client_port, MAXLINE, 0);
            printf("Connected to (%s, %s)\n", client_hostname, client_port);

            add_client(connfd, &pool);

        }
        check_clients(&pool);
        /*
        // 모든 클라이언트가 종료되었는지 확인
        int active_clients = 0;
        for (int i = 0; i <= pool.maxi; i++) {
            if (pool.clientfd[i] != -1)
                active_clients++;
        }

        // 모든 클라이언트 종료 시점에만 end_time 기록 및 출력
        if (started && active_clients == 0) {
            gettimeofday(&end_time, NULL);
            total_elapsed = (end_time.tv_sec - start_time.tv_sec)
                + (end_time.tv_usec - start_time.tv_usec) / 1e6;
            printf("\n[TOTAL TIME] All clients finished in %.6f seconds\n", total_elapsed);
            break; // 무한 루프 탈출
        }
        */
    }
    
    exit(0);
}


void init_pool(int listenfd, pool* p) {
    // init :  connected fd 없음
    int i;
    p->maxi = -1;
    for (i = 0; i < FD_SETSIZE; i++) {
        p->clientfd[i] = -1;
    }

    // init : listenfd 는 유일한 select read set 
    p->maxfd = listenfd;
    FD_ZERO(&p->read_set);
    FD_SET(listenfd, &p->read_set);
}

void add_client(int connfd, pool* p)
{
    int i;
    p->nready--;
    for (i = 0; i < FD_SETSIZE; i++)  /* Find an available slot */
        if (p->clientfd[i] < 0) {
            /* Add connected descriptor to the pool */
            p->clientfd[i] = connfd;                 //line:conc:echoservers:beginaddclient
            Rio_readinitb(&p->clientrio[i], connfd); //line:conc:echoservers:endaddclient

            /* Add the descriptor to descriptor set */
            FD_SET(connfd, &p->read_set); //line:conc:echoservers:addconnfd

            /* Update max descriptor and pool highwater mark */
            if (connfd > p->maxfd) //line:conc:echoservers:beginmaxfd
                p->maxfd = connfd; //line:conc:echoservers:endmaxfd
            if (i > p->maxi)       //line:conc:echoservers:beginmaxi
                p->maxi = i;       //line:conc:echoservers:endmaxi
            break;
        }
    if (i == FD_SETSIZE) /* Couldn't find an empty slot */
        app_error("add_client error: Too many clients");
}
/* $end add_client */

/* $begin check_clients */
void check_clients(pool* p)
{
    int i, connfd, n;
    char buf[MAXLINE];
    char obuf[MAXLINE];
    rio_t rio;



    for (i = 0; (i <= p->maxi) && (p->nready > 0); i++) {
        connfd = p->clientfd[i];
        rio = p->clientrio[i];

        /* If the descriptor is ready, echo a text line from it */
        if ((connfd > 0) && (FD_ISSET(connfd, &p->ready_set))) {
            p->nready--;
            if ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {
                printf("Server received %d bytes on fd %d\n", n, connfd);
                //Rio_writen(connfd, buf, n); //line:conc:echoservers:endecho
                //printf("%s",buf);

                // exit 명령 처리 추가
                if (strncmp(buf, "exit", 4)==0) {
                    printf("Client on fd %d requested exit.\n", connfd);
                    Close(connfd);
                    FD_CLR(connfd, &p->read_set);
                    p->clientfd[i] = -1;
                    continue;
                }

                obuf[0] = '\0';
                if (strncmp(buf, "show", 4) == 0) {
                    print_stock(root, obuf);
                    //Rio_writen(connfd, obuf, MAXLINE);
                }
                else if (strncmp(buf, "buy ", 4) == 0) {
                    int id, N;
                    if (sscanf(buf + 4, "%d %d", &id, &N) == 2) {
                        if (buy(id, N)) {
                            strcpy(obuf, "[buy] success\n");
                        }
                        else {
                            strcpy(obuf, "Not enough left stock\n");
                        }
                        //Rio_writen(connfd, obuf, MAXLINE);
                    }
                }
                else if (strncmp(buf, "sell ", 5) == 0) {
                    int id, N;
                    if (sscanf(buf + 5, "%d %d", &id, &N) == 2) {
                        strcpy(obuf, "[sell] success\n");
                        sell(id, N);
                        //Rio_writen(connfd, obuf, MAXLINE);
                    }
                }
                Rio_writen(connfd, obuf, MAXLINE);

            }

            /* EOF detected, remove descriptor from pool */
            else {
                obuf[0] = '\0';
                printf("%d closed\n", connfd);
                Close(connfd); //line:conc:echoservers:closeconnfd
                FD_CLR(connfd, &p->read_set); //line:conc:echoservers:beginremove
                p->clientfd[i] = -1;          //line:conc:echoservers:endremove
            }
        }
    }
}
/* $end echoserverimain */
// void command(){

// };
void load_stock() {
    FILE* pFile = fopen("stock.txt", "r");
    if (pFile == NULL) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    int id, left, pri;

    while (fscanf(pFile, "%d %d %d", &id, &left, &pri) == 3) {
        node* new_node = malloc(sizeof(node));
        if (new_node == NULL) {
            perror("Failed to allocate memory");
            exit(EXIT_FAILURE);
        }

        new_node->item.ID = id;
        new_node->item.left_stock = left;
        new_node->item.price = pri;
        new_node->left = NULL;
        new_node->right = NULL;

        if (root == NULL) {
            root = new_node;
            continue;
        }

        node* curr = root;
        node* parent = NULL;

        while (curr != NULL) {
            parent = curr;
            if (id < curr->item.ID) {
                curr = curr->left;
            }
            else {
                curr = curr->right;
            }
        }

        // 부모 노드 기준으로 좌/우에 연결
        if (id < parent->item.ID) {
            parent->left = new_node;
        }
        else {
            parent->right = new_node;
        }
        
    }

    fclose(pFile);
}
void save_stock(node* NODE, char* obuf) {
    FILE* fp = Fopen("stock.txt", "w");
    print_stock(NODE, obuf);
    fputs(obuf, fp);
    Fclose(fp);
}
void sell(int id, int n) {
    node* curr = root;

    while (curr != NULL) {
        if (curr->item.ID == id) {
            curr->item.left_stock += n;
            return;
        }

        if (id > curr->item.ID) {
            curr = curr->right;
        }
        else {
            curr = curr->left;
        }
    }

}
int buy(int id, int n) {
    node* curr = root;
    while (curr != NULL) {
        if (curr->item.ID == id) {
            if (curr->item.left_stock >= n) {
                curr->item.left_stock -= n;
                return 1;
            }
            else {
                return 0;
            }
        }

        if (id > curr->item.ID) {
            curr = curr->right;
        }
        else {
            curr = curr->left;
        }
    }
}

void print_stock(node* NODE, char* obuf) { //preorder
    node* curr = NODE;
    char str[MAXLINE];
    if (curr != NULL) {
        sprintf(str, "%d %d %d\n", curr->item.ID, curr->item.left_stock, curr->item.price);
    }
    strcat(obuf, str);
    if (curr->left != NULL) {
        print_stock(curr->left, obuf);
    }
    if (curr->right != NULL) {
        print_stock(curr->right, obuf);
    }
}