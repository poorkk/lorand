#ifndef KD_STR_H_
#define KD_STR_H_

#include <stdio.h>
#include <stdlib.h>

/* error */
#define ASSERT(cond) \
do { \
    if (!(cond)) { \
        printf("ASSERT | file: %s  line: %d  func: %s\n", __FILE__, __LINE__, __FUNCTION__); \
        exit(-1); \
    } \
} while (0)

#define LOG printf

/* type */
#define bool char
#define true 1
#define false 0

/* string */
bool str_end_with(const char *buf, int buflen, char end, char skip);
const char *str_spilt(const char *data, char split, char *buf, size_t bufsz);
void str_test();
char *kd_strstr(char *buf, int buflen, const char *find);

/* array */
#define ARR_LEN(arr) ((int)sizeof((arr)) / sizeof((arr)[0]))

/* buffer */
typedef struct {
    char *buf;
    int used;
    int size;
} KdBuf;

KdBuf *buf_malloc(int bufsz);
void buf_write(KdBuf *buf, char *data, int datalen);
void buf_write_str(KdBuf *buf, const char *str);
void buf_reset(KdBuf *buf);
#define buf_freesz(kbuf) ((kbuf)->size - (kbuf)->used)
#define buf_setend(kbuf) ((kbuf)->buf[(kbuf)->used] = '\0')

#define MAX(a, b) ((a) > (b) ?  (a) : (b))
#define MIN(a, b) ((a) < (b) ?  (a) : (b))

#endif