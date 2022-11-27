#include <stdio.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
#define MAX_CACHE_NUM 10

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
static const char *conn_hdr = "Connection: close\r\n";
static const char *proxy_hdr = "Proxy-Connection: close\r\n";

typedef struct
{
    char obj[MAX_OBJECT_SIZE];
    char uri[MAXLINE];
    int LRU;
    int isEmpty;

    int read_cnt;
    sem_t w; 
    sem_t mutex;
} Block;

typedef struct
{
    int num;
    Block data[MAX_CACHE_NUM];
} Cache;

Cache cache;

void init_cache();
int get_cache(char *url, char *buf);
int get_index();
void update_LRU(int index);
void write_cache(char *uri, char *buf);

void *thread_func(void *vargp);
void proxy(int fd);
void parse_uri(char *uri, char *host, char *path, int *port);
void build_requesthdrs(rio_t *rp, char *request, char *host, char* port);
void clienterror(int fd, char *cause, char *errnum, 
		 char *shortmsg, char *longmsg);

int main(int argc, char **argv)
{
    int listenfd, *connfd;
    pthread_t tid;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;

    /* Check command line args */
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }

    signal(SIGPIPE, SIG_IGN);
    init_cache();

    listenfd = Open_listenfd(argv[1]);
    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Malloc(sizeof(int));
        *connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        Getnameinfo((SA *) &clientaddr, clientlen, hostname, MAXLINE, 
                    port, MAXLINE, 0);
        printf("Accepted connection from (%s, %s)\n", hostname, port);
        Pthread_create(&tid, NULL, thread_func, connfd); //处理函数，这个是create了一个新线程
    }
    return 0;
}

/* Thread routine */
void *thread_func(void *vargp)
{
    int connfd = *((int *)vargp);
    Pthread_detach(pthread_self());
    Free(vargp);
    proxy(connfd);
    Close(connfd);
    return NULL;
}

/*
 * proxy - proxy HTTP GET request
 */
/* $begin proxy */
void proxy(int fd)
{
    char buf[2*MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char host[MAXLINE], path[MAXLINE];
    char port_str[MAXLINE], request[MAXLINE], cache_buf[MAX_OBJECT_SIZE];;
    int clientfd;
    int port = 80, n, ret;
    rio_t rio, rio_client;

    /* Read request line and headers */
    Rio_readinitb(&rio, fd);
    if (!Rio_readlineb(&rio, buf, MAXLINE))
        return;
    sscanf(buf, "%s %s %s", method, uri, version);
    if (strcasecmp(method, "GET")) {
        clienterror(fd, method, "501", "Not Implemented",
                    "Proxy does not implement this method");
        return;
    }

    if ((ret = get_cache(uri, cache_buf)) != -1)
    {
        Rio_writen(fd, cache_buf, strlen(cache_buf));
        return;
    }

    /* Parse URI from GET request */
    parse_uri(uri, host, path, &port);
    sprintf(port_str, "%d", port);

    sprintf(buf, "GET %s HTTP/1.0\r\n", path);
    strcat(request, buf);
    // 继续读取剩余请求，构建请求体
    build_requesthdrs(&rio, request, host, port_str);
    printf("request:\n%s", request);

    // 与远端服务器建立请求
    clientfd = Open_clientfd(host, port_str);
    if(clientfd < 0){
        clienterror(fd, method, "500", "Connection failed", "connect to server failed");
        return;
    }
    // 将请求发送到指定网址
    Rio_writen(clientfd, request, strlen(request));
    // 将读取到的response写回原客户端程序
    Rio_readinitb(&rio_client, clientfd);
    
    int cache_size_buf = 0;
    while ((n = Rio_readlineb(&rio_client, buf, MAXLINE))) {
        cache_size_buf += n;
        if(cache_size_buf < MAX_OBJECT_SIZE)
            strcat(cache_buf, buf);
        Rio_writen(fd, buf, n);
    }
    if(cache_size_buf < MAX_OBJECT_SIZE) {
        write_cache(uri, cache_buf);
    }
    Close(clientfd);
}
/* $end proxy */

// 构建request
void build_requesthdrs(rio_t *rp, char *request, char *host, char* port) {
    char buf[MAXLINE];

    while(Rio_readlineb(rp, buf, MAXLINE) > 0) {          
        if (!strcmp(buf, "\r\n")) break;
        if (strstr(buf, "Host:") != NULL) continue;
        if (strstr(buf, "User-Agent:") != NULL) continue;
        if (strstr(buf, "Connection:") != NULL) continue;
        if (strstr(buf, "Proxy-Connection:") != NULL) continue;

        strcat(request, buf);
    }
    sprintf(buf, "Host: %s:%s\r\n", host, port);
    strcat(request, buf);
    strcat(request, user_agent_hdr);
    strcat(request, conn_hdr);
    strcat(request, proxy_hdr);
    strcat(request, "\r\n");
}

/*
 * parse_uri - parse URI into host, path, port
 */
/* $begin parse_uri */
void parse_uri(char *uri, char *host, char *path, int *port) {
    *port = 80;
    // remove http://
    char* url1 = strstr(uri,"//");
    if (url1 == NULL) {
        url1 = uri;
    } else {
        url1 += 2;
    }

    // printf("parse uri url1 %s\n", url1);

    char* url2 = strstr(url1, ":");
    if (url2 != NULL) {
        // find port
        *url2 = '\0';
        strncpy(host, url1, MAXLINE);
        sscanf(url2 + 1, "%d%s", port, path);
        *url2 = ':';
    } else {
        // none port
        url2 = strstr(url1, "/");
        if (url2 == NULL) {
            strncpy(host, url1, MAXLINE);
            strcpy(path, "");
            return;
        }
        *url2 = '\0';
        strncpy(host, url1, MAXLINE);
        *url2 = '/';
        strncpy(path, url2, MAXLINE);
    }
}
/* $end parse_uri */

/*
 * clienterror - returns an error message to the client
 */
/* $begin clienterror */
void clienterror(int fd, char *cause, char *errnum, 
		 char *shortmsg, char *longmsg) 
{
    char buf[MAXLINE];

    /* Print the HTTP response headers */
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n\r\n");
    Rio_writen(fd, buf, strlen(buf));

    /* Print the HTTP response body */
    sprintf(buf, "<html><title>Proxy Error</title>");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<body bgcolor=""ffffff"">\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "%s: %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<p>%s: %s\r\n", longmsg, cause);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<hr><em>The Proxy Web server</em>\r\n");
    Rio_writen(fd, buf, strlen(buf));
}
/* $end clienterror */

//初始化Cache
void init_cache()
{
    cache.num = 0;
    int i;
    for (i = 0; i < MAX_CACHE_NUM; i++)
    {
        cache.data[i].LRU = 0;
        cache.data[i].isEmpty = 1;
        Sem_init(&cache.data[i].w, 0, 1);
        Sem_init(&cache.data[i].mutex, 0, 1);
        cache.data[i].read_cnt = 0;
    }
}

// 从Cache中找到内容，找到则复制到buf中，未找到则返回-1
int get_cache(char *url, char *buf)
{
    int pos = -1;
    for (int i = 0; i < MAX_CACHE_NUM; i++)
    {
        P(&cache.data[i].mutex);
        cache.data[i].read_cnt++;
        if (cache.data[i].read_cnt == 1)
            P(&cache.data[i].w);
        V(&cache.data[i].mutex);

        if ((cache.data[i].isEmpty == 0) && (strcmp(url, cache.data[i].uri) == 0)) {
            pos = i;
            strcpy(buf, cache.data[i].obj);
            cache.data[i].LRU = __INT_MAX__;
        }  

        P(&cache.data[i].mutex);
        cache.data[i].read_cnt--;
        if (cache.data[i].read_cnt == 0)
            V(&cache.data[i].w);
        V(&cache.data[i].mutex);

        if (pos != -1)
            break;
    }

    return pos;
}

void write_cache(char *uri, char *buf)
{
    int i = get_index();
    //加写锁
    P(&cache.data[i].w);
    //写入内容
    strcpy(cache.data[i].obj, buf);
    strcpy(cache.data[i].uri, uri);
    cache.data[i].isEmpty = 0;
    cache.data[i].LRU = __INT_MAX__;
    update_LRU(i);

    V(&cache.data[i].w);
}

//找到可以存放的缓存行
int get_index()
{
    int min = __INT_MAX__;
    int minindex = 0;
    int i;
    for (i = 0; i < MAX_CACHE_NUM; i++)
    {
        //读锁
        P(&cache.data[i].mutex);
        cache.data[i].read_cnt++;
        if (cache.data[i].read_cnt == 1)
            P(&cache.data[i].w);
        V(&cache.data[i].mutex);

        if (cache.data[i].isEmpty == 1)
        {
            minindex = i;
            P(&cache.data[i].mutex);
            cache.data[i].read_cnt--;
            if (cache.data[i].read_cnt == 0)
                V(&cache.data[i].w);
            V(&cache.data[i].mutex);
            break;
        }
        if (cache.data[i].LRU < min)
        {
            minindex = i;
        }

        P(&cache.data[i].mutex);
        cache.data[i].read_cnt--;
        if (cache.data[i].read_cnt == 0)
            V(&cache.data[i].w);
        V(&cache.data[i].mutex);
    }

    return minindex;
}

void update_LRU(int index)
{
    for (int i = 0; i < MAX_CACHE_NUM; i++)
    {
        if (cache.data[i].isEmpty == 0 && i != index)
        {
            P(&cache.data[i].w);
            cache.data[i].LRU--;
            V(&cache.data[i].w);
        }
    }
}

