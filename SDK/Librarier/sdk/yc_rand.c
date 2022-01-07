#include "yc_rand.h"

static uint32_t TRand(void *buf, uint32_t bytes)
{
    uint32_t i;
    for (i = 0; i < bytes; i++)
    {
        ((uint8_t *)buf)[i] = GetTRNGData_8bit();
    }
    return i;
}

uint32_t RAND_Rand(void *rand, uint32_t bytes)
{
    return TRand(rand, bytes);
}

uint32_t RAND_RandP(void *rand, uint32_t bytes, void *p_rng)
{
    return RAND_Rand(rand, bytes);
}

uint32_t RAND_RandCheck(void *rand, uint32_t bytes)
{
    return 0;
}

uint32_t RAND_Init()
{
    TRNG_Init();
    return 0;
}
