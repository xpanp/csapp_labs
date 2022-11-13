#include <getopt.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cachelab.h"

#define FILENAME_MAX_LEN 1024

typedef struct{
	int valid_bit;
	int tag;
	int stamp;
} cache_line;

typedef struct{
    int s;
    int S;
    int E;
    int b;
    int v;
    int stamp;
    int hit_cnt;
    int miss_cnt;
    int evic_cnt;
    cache_line **c_line;
} cache_ctl;

typedef struct{
    char op;
    unsigned addr;
    int size;
} mem_op;

void print_usage()
{
    printf("Usage: ./csim [-hv] -s <num> -E <num> -b <num> -t <file>\n"
            "Options:\n"
            "  -h         Print this help message.\n"
            "  -v         Optional verbose flag.\n"
            "  -s <num>   Number of set index bits.\n"
            "  -E <num>   Number of lines per set.\n"
            "  -b <num>   Number of block offset bits.\n"
            "  -t <file>  Trace file.\n\n"
            "Examples:\n"
            "  linux>  ./csim -s 4 -E 1 -b 4 -t traces/yi.trace\n"
            "  linux>  ./csim -v -s 8 -E 2 -b 4 -t traces/yi.trace\n");
}

void init(cache_ctl* c_ctl, int s, int E, int b, int v){
    int S = (1 << s);
    c_ctl->s = s;
    c_ctl->S = S;
    c_ctl->E = E;
    c_ctl->b = b;
    c_ctl->v = v;
    c_ctl->hit_cnt = 0;
    c_ctl->miss_cnt = 0;
    c_ctl->evic_cnt = 0;
    c_ctl->stamp = 1;

    c_ctl->c_line = (cache_line**)malloc(sizeof(cache_line*) * S);
	for(int i = 0; i < S; i++) {
        *(c_ctl->c_line+i) = (cache_line*)malloc(sizeof(cache_line) * E);
    }

	for(int i = 0; i < S; i++){
		for(int j = 0; j < E; j++){
			c_ctl->c_line[i][j].valid_bit = 0;
			c_ctl->c_line[i][j].tag = 0xffffffff;
			c_ctl->c_line[i][j].stamp = 0;
		}
	}
}

int get_tag(int addr, int s, int b) {
    return (addr >> (s+b));
}

int get_set(int addr, int s, int b) {
    return (addr >> b) & (0xffffffff >> (32-s));
}

// 找到tag相等的位置
int find_tag(cache_ctl* c_ctl, int set, int tag) {
    int res = -1;
    cache_line **c_line = c_ctl->c_line;
    for (size_t i = 0; i < c_ctl->E; i++)
    {
        if (c_line[set][i].valid_bit == 1 && tag == c_line[set][i].tag) {
            res = i;
            break;
        }
    }
    return res;
}

// 没有tag相等，找一个未被初始化的位置
int find_valid(cache_ctl* c_ctl, int set) {
    int res = -1;
    cache_line **c_line = c_ctl->c_line;
    for (size_t i = 0; i < c_ctl->E; i++)
    {
        if (c_line[set][i].valid_bit == 0) {
            res = i;
        }
    }
    return res;
}

// 使用lru策略找一个可以被替换掉的位置
int find_evic(cache_ctl* c_ctl, int set) {
    int res = -1;
    cache_line **c_line = c_ctl->c_line;
    int min_stamp = __INT_MAX__;
    for (size_t i = 0; i < c_ctl->E; i++)
    {
        // 找一个更新时间最早的块
        if (c_line[set][i].stamp < min_stamp) {
            min_stamp = c_line[set][i].stamp;
            res = i;
        }
    }
    return res;
}

// 更新一个具体的块
void update_one(cache_ctl* c_ctl, int set, int e, int tag) {
    c_ctl->c_line[set][e].valid_bit = 1;
    c_ctl->c_line[set][e].tag = tag;
    c_ctl->c_line[set][e].stamp = c_ctl->stamp;
}

// 寻址并更新cache
void update(cache_ctl* c_ctl, mem_op* m_op) {
    int tag, set, res;
    tag = get_tag(m_op->addr, c_ctl->s, c_ctl->b);
    set = get_set(m_op->addr, c_ctl->s, c_ctl->b);

    res = find_tag(c_ctl, set, tag);
    if (res > -1) {
        c_ctl->hit_cnt++;
        update_one(c_ctl, set, res, tag);
        return;
    }
    c_ctl->miss_cnt++;

    res = find_valid(c_ctl, set);
    if (res > -1) {
        update_one(c_ctl, set, res, tag);
        return;
    }
    c_ctl->evic_cnt++;

    res = find_evic(c_ctl, set);
    update_one(c_ctl, set, res, tag);
    return;
}

// 释放二维数组
void free_cache_ctl(cache_ctl* c_ctl) {
    for(int i = 0; i < c_ctl->S; i++) {
        free(c_ctl->c_line[i]);
    }
    free(c_ctl->c_line);
}

// 处理输入文件
void deal_func(cache_ctl* c_ctl, const char* tracefile) {
    FILE * pFile;
    pFile = fopen(tracefile,"r");
    mem_op m_op;
    // Reading lines like " M 20,1" or "L 19,3"
    while(fscanf(pFile," %c %x,%d", &m_op.op, &m_op.addr, &m_op.size) > 0)
    {
        switch (m_op.op)
        {
            case 'L':
                update(c_ctl, &m_op);
                break;
            case 'M':   // load一次，还需要store一次
                update(c_ctl, &m_op);
            case 'S':
                update(c_ctl, &m_op);
                break;
            default:
                break;
        }
        c_ctl->stamp++; // 更新时钟
    }
    fclose(pFile);
}

int main(int argc, char** argv)
{
    int opt, s, E, b;
    int verbose = 0;
    size_t f_len;
    char tracefile[FILENAME_MAX_LEN] = {0};
    // 循环处理输入参数
    while(-1 != (opt = getopt(argc, argv, "hvs:E:b:t:"))) {
        /* determine which argument it’s processing */
        switch(opt) {
            case 'h':
                print_usage();
                return 0;
            case 'v':
                verbose = 1;
                break;
            case 's':
                s = atoi(optarg);
                break;
            case 'E':
                E = atoi(optarg);
                break;
            case 'b':
                b = atoi(optarg);
                break;
            case 't':
                f_len = strlen(optarg);
                if (f_len >= FILENAME_MAX_LEN) {
                    printf("Trace file len: %ld, out of range!", f_len);
                    return -1;
                }
                strncpy(tracefile, optarg, f_len);
                break;
            default:
                printf("wrong argument\n");
                break;
        }
	}
    
    if (s <= 0 || E <= 0 || b <= 0) {
        printf("wrong argument\n");
        return -1;
    }

    cache_ctl c_ctl;
    init(&c_ctl, s, E, b, verbose);

    deal_func(&c_ctl, tracefile);

    printSummary(c_ctl.hit_cnt, c_ctl.miss_cnt, c_ctl.evic_cnt);

    free_cache_ctl(&c_ctl);
    return 0;
}
