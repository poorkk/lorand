#ifndef TYTE_H_
#define TYTE_H_

typedef unsigned char uchar;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;

#define VAR_ARR_SIZE 0
#define ARR_LEN(arr) (sizeof((arr)) / sizeof((arr)[0]))

#define bool char
#define true  1
#define false 0

#endif