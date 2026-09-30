#include "data.h"

#include <math.h>
#include <stdbool.h>

#define TWO_PI_F             6.2831853071795864769f
#define SAMPLE_RATE_F        10000.0f
#define SINE_CHANNEL_COUNT   (SIGNAL_CH - 1U)
#define TRIANGLE_CHANNEL     (SIGNAL_CH - 1U)
#define TRIANGLE_MAX_COUNT   1000U

static float g_phaseStep[SINE_CHANNEL_COUNT];
static float g_sine[SINE_CHANNEL_COUNT];
static float g_cosine[SINE_CHANNEL_COUNT];
static float g_sineStep[SINE_CHANNEL_COUNT];
static float g_cosineStep[SINE_CHANNEL_COUNT];
static uint16_t g_sampleIndex;
static uint16_t g_triangleCount;
static bool g_triangleIncreasing;

void Signal_Init(void)
{
    uint16_t channel;

    for(channel = 0U; channel < SINE_CHANNEL_COUNT; channel++)
    {
        g_phaseStep[channel] =
            TWO_PI_F * (float)(channel + 1U) / SAMPLE_RATE_F;
        g_sine[channel] = 0.0f;
        g_cosine[channel] = 1.0f;
        g_sineStep[channel] = sinf(g_phaseStep[channel]);
        g_cosineStep[channel] = cosf(g_phaseStep[channel]);
    }

    g_sampleIndex = 0U;
    g_triangleCount = 0U;
    g_triangleIncreasing = true;
}

void Signal_Generate(float out[SIGNAL_CH])
{
    uint16_t channel;
    float nextSine;
    float nextCosine;

    /* Channels 0...8: 1...9 Hz sine waves. */
    for(channel = 0U; channel < SINE_CHANNEL_COUNT; channel++)
    {
        out[channel] = g_sine[channel];
        nextSine = (g_sine[channel] * g_cosineStep[channel]) +
                   (g_cosine[channel] * g_sineStep[channel]);
        nextCosine = (g_cosine[channel] * g_cosineStep[channel]) -
                     (g_sine[channel] * g_sineStep[channel]);
        g_sine[channel] = nextSine;
        g_cosine[channel] = nextCosine;
    }

    /* Channel 9: 0 -> 1000 -> 0, changing by exactly one per sample. */
    out[TRIANGLE_CHANNEL] = (float)g_triangleCount;

    if(g_triangleIncreasing)
    {
        if(g_triangleCount >= TRIANGLE_MAX_COUNT)
        {
            g_triangleIncreasing = false;
            g_triangleCount--;
        }
        else
        {
            g_triangleCount++;
        }
    }
    else
    {
        if(g_triangleCount == 0U)
        {
            g_triangleIncreasing = true;
            g_triangleCount++;
        }
        else
        {
            g_triangleCount--;
        }
    }

    g_sampleIndex++;
    if(g_sampleIndex >= (uint16_t)SAMPLE_RATE_F)
    {
        /* The integer-Hz sine channels are at phase zero once per second. */
        for(channel = 0U; channel < SINE_CHANNEL_COUNT; channel++)
        {
            g_sine[channel] = 0.0f;
            g_cosine[channel] = 1.0f;
        }
        g_sampleIndex = 0U;
    }
}
