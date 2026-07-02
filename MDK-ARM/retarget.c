/**
 ******************************************************************************
 * @file    retarget.c
 * @brief   Retarget stdio for Keil MDK-ARM (disable semihosting)
 *          Provides stub implementations for _sys_exit, _ttywrch, etc.
 *          Required when using standard library (non-microlib) without
 *          a debug console.
 ******************************************************************************
 */

#include <stdio.h>
#include <rt_misc.h>

#pragma import(__use_no_semihosting)

struct __FILE
{
    int handle;
};

FILE __stdout;
FILE __stdin;
FILE __stderr;

void _sys_exit(int x)
{
    (void)x;
    while (1);
}

void _ttywrch(int ch)
{
    (void)ch;
}

int fputc(int ch, FILE *f)
{
    (void)f;
    (void)ch;
    return ch;
}

int fgetc(FILE *f)
{
    (void)f;
    return 0;
}

int ferror(FILE *f)
{
    (void)f;
    return 0;
}
