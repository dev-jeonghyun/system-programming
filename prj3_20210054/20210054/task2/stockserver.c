/*
 echoserveri.c - An iterative echo server
 */
 /* $begin echoserverimain */

 // #define
#define NTHREADS 40
#define SBUFSIZE 2000

#include "csapp.h"
//시간 측정

/*
struct timeval start_time, end_time;
double total_elapsed;
int started = 0;
int remaining_clients = 0;
sem_t mutex_remaining;
*/

typedef struct {
    int ID;
    int left_stock;
    int price;
    int readcnt;
    sem_t mutex;
    sem_t w;
}item;

typedef struct {
    int* buf; /* Buffer array */
    int n; /* Maximum number of slots */
    int front; /* buf[(front+1)%n] is the first item */
    int rear; /* buf[rear%n] is the last item */
    sem_t mutex; /* Protects accesses to buf */
    sem_t slots; /* Counts available slots */
    sem_t items; /* Counts available items */
} sbuf_t;

typedef struct node {
    item item;
    struct node* left;
    struct node* right;
}node;

void echo(int connfd);

void sbuf_init(sbuf_t* sp, int n)
{
    sp->buf = Calloc(n, sizeof(int));
    sp->n = n;
    sp->front = sp->rear = 0;
    Sem_init(&sp->mutex, 0, 1);
    Sem_init(&sp->slots, 0, n);
    Sem_init(&sp->items, 0, 0);
}

void sbuf_deinit(sbuf_t* sp)
{
    Free(sp->buf);
}
void sbuf_insert(sbuf_t* sp, int item)
{
    P(&sp->slots);
    P(&sp->mutex);
    sp->buf[(++sp->rear) % (sp->n)] = item;
    V(&sp->mutex);
    V(&sp->items);
}
int sbuf_remove(sbuf_t* sp)
{
    int item;
    P(&sp->items);
    P(&sp->mutex);
    item = sp->buf[(++sp->front) % (sp->n)];
    V(&sp->mutex);
    V(&sp->slots);
    return item;
}

void* thread(void* vargp);

static int byte_cnt; /* Byte counter */
static sem_t mutex; /* and the mutex that protects it */
static void init_echo_cnt(void)
{
    Sem_init(&mutex, 0, 1);
    byte_cnt = 0;
}

void load_stock();
void save_stock();
void print_stock();
void sigint_handler(int sig);
void sell(int a, int b);
int buy(int a, int b);
sbuf_t sbuf;
node* root;

void sigint_handler(int sig) {
    char sigbuf[MAXLINE];
    sigbuf[0] = '\0';
    //save_stock(root, sigbuf);
    FILE* fp = Fopen("stock.txt", "w");
    sigbuf[0] = '\0';
    print_stock(root, sigbuf);
    fputs(sigbuf, fp);
    Fclose(fp);
    exit(0);
}



void echo_cnt(int connfd) {
    int n;
    char buf[MAXLINE];
    char obuf[MAXLINE];
    rio_t rio;
    static pthread_once_t once = PTHREAD_ONCE_INIT;

    Pthread_once(&once, init_echo_cnt);
    Rio_readinitb(&rio, connfd);
    while ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {
        // P(&mutex);
        byte_cnt += n;
        printf("thread %d received %d (%d total) bytes on fd %d\n",
            (int)pthread_self(), n, byte_cnt, connfd);
        obuf[0] = '\0';

        //  exit 명령 처리 추가
        if (strncmp(buf, "exit", 4) == 0) {
            printf("Client on fd %d exited.\n", connfd);
            break; // 루프 탈출 후 연결 종료
        }

        if (strncmp(buf, "show", 4) == 0) {
            P(&root->item.mutex);
            root->item.readcnt++;
            if (root->item.readcnt == 1) {
                P(&root->item.w);
            }
            V(&root->item.mutex);

            print_stock(root, obuf);
            P(&root->item.mutex);
            root->item.readcnt--;
            if (root->item.readcnt == 0) {
                V(&root->item.w);
            }
            V(&root->item.mutex);
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
            }
        }
        else if (strncmp(buf, "sell ", 5) == 0) {
            int id, N;
            if (sscanf(buf + 5, "%d %d", &id, &N) == 2) {
                sell(id, N);
                strcpy(obuf, "[sell] success\n");
               
            }
        }
        Rio_writen(connfd, obuf, MAXLINE);
        // obuf[0]='\0'
        // save_stock(root,obuf);
        // V(&mutex);
    }
    Close(connfd);

    //시간 측정
    /*
    P(&mutex_remaining);
    remaining_clients--;
    if (remaining_clients == 0) {
        gettimeofday(&end_time, NULL);
        total_elapsed = (end_time.tv_sec - start_time.tv_sec)
            + (end_time.tv_usec - start_time.tv_usec) / 1e6;
        printf("\n[TOTAL TIME] All clients finished in %.6f seconds\n", total_elapsed);
    }
    V(&mutex_remaining);
    */
    //이거 여기에 넣으면 thread에서 뺴도 된다함. (둘 다 테스트)
}


/////////////////////////////////////////////////////////////////////////



int main(int argc, char** argv)
{
    Signal(SIGINT, sigint_handler);
    int listenfd, connfd, i;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;  /* Enough space for any address */  //line:netp:echoserveri:sockaddrstorage
    pthread_t tid;
    

    
    char client_hostname[MAXLINE], client_port[MAXLINE];

    //시간 측정
    //Sem_init(&mutex_remaining, 0, 1);

    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }

    load_stock();

    listenfd = Open_listenfd(argv[1]);

    sbuf_init(&sbuf, SBUFSIZE);
    for (i = 0; i < NTHREADS; i++)/* Create a pool of worker threads */
        Pthread_create(&tid, NULL, thread, NULL);
    while (1) {
        clientlen = sizeof(struct sockaddr_storage);
        connfd = Accept(listenfd, (SA*)&clientaddr, &clientlen);
        //시간 측정
        /*
        P(&mutex_remaining);
        if (!started) {
            gettimeofday(&start_time, NULL);
            started = 1;
        }
        remaining_clients++;  // 연결된 클라이언트 수 증가
        V(&mutex_remaining);
        //시간 측정
        */
        sbuf_insert(&sbuf, connfd); /* Insert connfd in buffer */

        Getnameinfo((SA*)&clientaddr, clientlen, client_hostname, MAXLINE,
            client_port, MAXLINE, 0);
        printf("Connected to (%s, %s)\n", client_hostname, client_port);

    
    }
}
/////////////////////////////////////////////////////////////////////////



void load_stock() {
    FILE* pFile = fopen("stock.txt", "r");
    if (pFile == NULL) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    /*int a, b, c;
    node* now;
    node* nn;
    node* pre;
    */
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
        //Sem_init(&now->item.w, 0, 1);
        //Sem_init(&now->item.mutex, 0, 1);

        Sem_init(&new_node->item.w, 0, 1);
        Sem_init(&new_node->item.mutex, 0, 1);
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
void sell(int id, int quantity) {
    node* curr = root;

    while (curr != NULL) {
        if (curr->item.ID == id) {
            P(&curr->item.w);
            curr->item.left_stock += quantity;
            V(&curr->item.w);
            return;
        }
        else if (curr->item.ID < id) {
            curr = curr->right;
        }
        else {
            curr = curr->left;
        }
    }
}

int buy(int id, int quantity) {
    node* curr = root;

    while (curr != NULL) {
        if (curr->item.ID == id) {
            P(&curr->item.w);
            if (curr->item.left_stock >= quantity) {
                curr->item.left_stock -= quantity;
                V(&curr->item.w);
                return 1;
            }
            else {
                V(&curr->item.w);
                return 0;
            }
        }
        else if (curr->item.ID < id) {
            curr = curr->right;
        }
        else {
            curr = curr->left;
        }
    }

    return 0;
}

void save_stock(node* NODE, char* obuf) {
    FILE* fp = Fopen("stock.txt", "w");
    obuf[0] = '\0';
    print_stock(NODE, obuf);
    fputs(obuf, fp);
    Fclose(fp);
}

void print_stock(node* NODE, char* obuf) {
    node* now = NODE;
    char str[MAXLINE];
    if (now != NULL) {
        sprintf(str, "%d %d %d\n", now->item.ID, now->item.left_stock, now->item.price);
    }
    strcat(obuf, str);
    if (now->left != NULL) {
        print_stock(now->left, obuf);
    }
    if (now->right != NULL) {
        print_stock(now->right, obuf);
    }
}

void* thread(void* vargp) {
    Pthread_detach(pthread_self());
    while (1) {
        int connfd = sbuf_remove(&sbuf);
        echo_cnt(connfd);
        //Close(connfd);
    }
}


