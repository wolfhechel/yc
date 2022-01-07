#ifndef  __KSCAN_H__
#define  __KSCAN_H__

#include "yc3121.h"
#include "yc_gpio.h"
#include "yc_timer.h"
#include "board_config.h"

#define SHAKE_DELAY	5

#if (BOARD_VER == MPOS_BOARD_V2_1)

    #define KEY_S1	KEY_V1
    #define KEY_S2	KEY_V2
    #define KEY_S3	KEY_V6
    #define KEY_S4	KEY_V3
    #define KEY_S5	KEY_V7
    #define KEY_S6	KEY_V10
    #define KEY_S7	KEY_V4
    #define KEY_S8	KEY_V8
    #define KEY_S9	KEY_V11
    #define KEY_S10	KEY_V13
    #define KEY_S11	KEY_V5
    #define KEY_S12	KEY_V9
    #define KEY_S13	KEY_V12
    #define KEY_S14	KEY_V14
    #define KEY_S15	KEY_V15

    #define KEY_V1	30
    #define KEY_V2	31
    #define KEY_V3	32
    #define KEY_V4	33
    #define KEY_V5	34
    #define KEY_V6	35
    #define KEY_V7	36
    #define KEY_V8	37
    #define KEY_V9	38
    #define KEY_V10	39
    #define KEY_V11	40
    #define KEY_V12	41
    #define KEY_V13	42
    #define KEY_V14	43
    #define KEY_V15	44

#elif (BOARD_VER == EPOS_BOARD_V1_0)

    #define KEY_1  32
    #define KEY_2  35
    #define KEY_3  38
    #define KEY_4  41
    #define KEY_5  44
    #define KEY_6  31
    #define KEY_7  34
    #define KEY_8  40
    #define KEY_9  37
    #define KEY_10 33
    #define KEY_11 30
    #define KEY_12 43
    #define KEY_13 39
    #define KEY_14 42
    #define KEY_15 45
    #define KEY_16 47
    #define KEY_17 46
    #define KEY_18 36

    #define KEY_NUM_0   KEY_11
    #define KEY_NUM_1   KEY_6
    #define KEY_NUM_2   KEY_1
    #define KEY_NUM_3   KEY_10
    #define KEY_NUM_4   KEY_7
    #define KEY_NUM_5   KEY_2
    #define KEY_NUM_6   KEY_18
    #define KEY_NUM_7   KEY_9
    #define KEY_NUM_8   KEY_3
    #define KEY_NUM_9   KEY_13
    #define KEY_UP      KEY_8
    #define KEY_DOWN    KEY_4
    #define KEY_CONFIRM KEY_16
    #define KEY_CANCEL  KEY_15
    #define KEY_CLEAR   KEY_17
    #define KEY_MENU    KEY_14
    #define KEY_F1      KEY_12
    #define KEY_F2      KEY_5

#endif

/**
  * @brief  Keyboard scan function.
  * @param  None.
  * @retval key assignments
  */
uint8_t KEY_Scan(void);

/**
  * @brief  Keyboard release detection.
  * @param  None.
  * @retval None.
  */
void KEY_Scan_check(void);

#endif
