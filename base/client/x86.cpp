#include <stdlib.h>
#include "client.h"

#if id386

#include <intrin.h>

static unsigned long bias;
static unsigned long *histogram;
static unsigned long start, range;

/*
 * The original implementation read the low 32 bits of RDTSC into EAX.
 * __rdtsc() is the compiler intrinsic equivalent; only the low 32 bits are
 * retained, preserving the original Win32 data size and histogram behavior.
 */
static unsigned long x86_ReadTSC32(void)
{
    return (unsigned long)__rdtsc();
}

void x86_TimerStart(void)
{
    start = x86_ReadTSC32();
}

void x86_TimerStop(void)
{
    unsigned long now = x86_ReadTSC32();
    unsigned long elapsed = now - start;

    if (elapsed < bias)
        return;

    elapsed -= bias;
    if (elapsed >= range)
        return;

    histogram[elapsed]++;
}

static unsigned long x86_TimerStopBias(void)
{
    return x86_ReadTSC32() - start;
}

void x86_TimerInit( unsigned long smallest, unsigned length )
{
    int i;
    unsigned long biastable[100];

    range = length;
    bias = 10000;

    for ( i = 0; i < 100; i++ )
    {
        x86_TimerStart();
        biastable[i] = x86_TimerStopBias();

        if ( bias > biastable[i] )
            bias = biastable[i];
    }

    bias += smallest;
    histogram = (unsigned long *)X_Malloc( range * sizeof( unsigned long ), MEM_TAG_MISC );
}

unsigned long *x86_TimerGetHistogram( void )
{
    return histogram;
}

#endif
