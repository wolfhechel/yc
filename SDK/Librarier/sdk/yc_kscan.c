#include "yc_kscan.h"

#if (BOARD_VER == MPOS_BOARD_V2_1)
const uint8_t keyMatrix[5][5] =
{
    {KEY_S1,     0,       0,       0,       0   },
    {KEY_S2,  KEY_S3,     0,       0,       0   },
    {KEY_S4,  KEY_S5,  KEY_S6,     0,       0   },
    {KEY_S7,  KEY_S8,  KEY_S9,  KEY_S10,    0   },
    {KEY_S11, KEY_S12, KEY_S13, KEY_S14, KEY_S15}
};
#elif (BOARD_VER == EPOS_BOARD_V1_0)
const uint8_t keyMatrix[6][5] =
{
    {KEY_1,     0,        0,      0,      0},
    {KEY_2,  KEY_6,       0,      0,      0},
    {KEY_3,  KEY_7,  KEY_12,      0,      0},
    {KEY_4,  KEY_8,  KEY_11, KEY_13,      0},
    {KEY_5,  KEY_9,  KEY_10, KEY_14, KEY_15},
    {KEY_16, KEY_17, KEY_18,      0,      0}
};
#endif

/**
  * @brief  Keyboard related gpio initializes pull-up.
  * @param  None
  * @retval None
  */
static void Key_Gpio_Init_IPU(void)
{
#if (BOARD_VER == MPOS_BOARD_V2_1)
    GPIO_InitTypeDef GPIO_InitStruct;

    GPIO_InitStruct.GPIO_Pin = Line_1;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIO_GROUP_Line_1, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = Line_2;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIO_GROUP_Line_2, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = Line_3;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIO_GROUP_Line_3, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = Line_4;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIO_GROUP_Line_4, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = Line_5;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIO_GROUP_Line_5, &GPIO_InitStruct);

#elif (BOARD_VER == EPOS_BOARD_V1_0)
    GPIO_InitTypeDef GPIO_InitStruct;

    GPIO_InitStruct.GPIO_Pin = KEY_PIN_1;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(KEY_PORT_1, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = KEY_PIN_2;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(KEY_PORT_2, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = KEY_PIN_3;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(KEY_PORT_3, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = KEY_PIN_4;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(KEY_PORT_4, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = KEY_PIN_5;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(KEY_PORT_5, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = KEY_PIN_6;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(KEY_PORT_6, &GPIO_InitStruct);
#endif
}

/**
  * @brief  Keyboard GPIO level detection.
  * @param  scanColumn: Scan column selection.
  * @retval None
  */
static void Key_IODirection(uint8_t scanColumn)
{
#if (BOARD_VER == MPOS_BOARD_V2_1)
    switch (scanColumn)
    {
    case	0:
        GPIO_Config(GPIO_GROUP_Line_1, Line_1, OUTPUT_LOW);
        break;
    case	1:
        GPIO_Config(GPIO_GROUP_Line_2, Line_2, OUTPUT_LOW);
        break;
    case	2:
        GPIO_Config(GPIO_GROUP_Line_3, Line_3, OUTPUT_LOW);
        break;
    case	3:
        GPIO_Config(GPIO_GROUP_Line_4, Line_4, OUTPUT_LOW);
        break;
    case	0xff:
        GPIO_Config(GPIO_GROUP_Line_1, Line_1, OUTPUT_LOW);
        GPIO_Config(GPIO_GROUP_Line_2, Line_2, OUTPUT_LOW);
        GPIO_Config(GPIO_GROUP_Line_3, Line_3, OUTPUT_LOW);
        GPIO_Config(GPIO_GROUP_Line_4, Line_4, OUTPUT_LOW);
        GPIO_Config(GPIO_GROUP_Line_5, Line_5, OUTPUT_LOW);
        break;
    default:
        break;
    }
#elif (BOARD_VER == EPOS_BOARD_V1_0)
    switch (scanColumn)
    {
    case	0:
        GPIO_Config(KEY_PORT_1, KEY_PIN_1, OUTPUT_LOW);
        break;
    case	1:
        GPIO_Config(KEY_PORT_2, KEY_PIN_2, OUTPUT_LOW);
        break;
    case	2:
        GPIO_Config(KEY_PORT_3, KEY_PIN_3, OUTPUT_LOW);
        break;
    case	3:
        GPIO_Config(KEY_PORT_4, KEY_PIN_4, OUTPUT_LOW);
        break;
    case	4:
        GPIO_Config(KEY_PORT_5, KEY_PIN_5, OUTPUT_LOW);
        break;
    case	0xff:
        GPIO_Config(KEY_PORT_1, KEY_PIN_1, OUTPUT_LOW);
        GPIO_Config(KEY_PORT_2, KEY_PIN_2, OUTPUT_LOW);
        GPIO_Config(KEY_PORT_3, KEY_PIN_3, OUTPUT_LOW);
        GPIO_Config(KEY_PORT_4, KEY_PIN_4, OUTPUT_LOW);
        GPIO_Config(KEY_PORT_5, KEY_PIN_5, OUTPUT_LOW);
        GPIO_Config(KEY_PORT_6, KEY_PIN_6, OUTPUT_LOW);
        break;
    default:
        break;
    }
#endif
}

/**
  * @brief  Gets the keyboard IO input status.
  * @param  keyIO
  * @retval None
  */
static uint8_t Key_GetIoValue(uint8_t keyIO)
{
    uint8_t temp = 0xff;
#if (BOARD_VER == MPOS_BOARD_V2_1)
    switch (keyIO)
    {
    case 0:
        if (0 == GPIO_ReadInputDataBit(GPIO_GROUP_Line_2, Line_2))
            temp = 1;
        else if (0 == GPIO_ReadInputDataBit(GPIO_GROUP_Line_3, Line_3))
            temp = 2;
        else if (0 == GPIO_ReadInputDataBit(GPIO_GROUP_Line_4, Line_4))
            temp = 3;
        else if (0 == GPIO_ReadInputDataBit(GPIO_GROUP_Line_5, Line_5))
            temp = 4;
        break;
    case 1:
        if (0 == GPIO_ReadInputDataBit(GPIO_GROUP_Line_3, Line_3))
            temp = 2;
        else if (0 == GPIO_ReadInputDataBit(GPIO_GROUP_Line_4, Line_4))
            temp = 3;
        else if (0 == GPIO_ReadInputDataBit(GPIO_GROUP_Line_5, Line_5))
            temp = 4;
        break;
    case 2:
        if (0 == GPIO_ReadInputDataBit(GPIO_GROUP_Line_4, Line_4))
            temp = 3;
        else if (0 == GPIO_ReadInputDataBit(GPIO_GROUP_Line_5, Line_5))
            temp = 4;
        break;
    case 3:
        if (0 == GPIO_ReadInputDataBit(GPIO_GROUP_Line_5, Line_5))
            temp = 4;
        break;
    case 0xff:
        if (0 == GPIO_ReadInputDataBit(GPIO_GROUP_Line_1, Line_1))
            temp = 0;
        else if (0 == GPIO_ReadInputDataBit(GPIO_GROUP_Line_2, Line_2))
            temp = 1;
        else if (0 == GPIO_ReadInputDataBit(GPIO_GROUP_Line_3, Line_3))
            temp = 2;
        else if (0 == GPIO_ReadInputDataBit(GPIO_GROUP_Line_4, Line_4))
            temp = 3;
        else if (0 == GPIO_ReadInputDataBit(GPIO_GROUP_Line_5, Line_5))
            temp = 4;
        break;
    default:
        break;
    }
    return temp;
#elif (BOARD_VER == EPOS_BOARD_V1_0)
    switch (keyIO)
    {
    case 0:
        if (0 == GPIO_ReadInputDataBit(KEY_PORT_2, KEY_PIN_2))
            temp = 0;
        else if (0 == GPIO_ReadInputDataBit(KEY_PORT_3, KEY_PIN_3))
            temp = 1;
        else if (0 == GPIO_ReadInputDataBit(KEY_PORT_4, KEY_PIN_4))
            temp = 2;
        else if (0 == GPIO_ReadInputDataBit(KEY_PORT_5, KEY_PIN_5))
            temp = 3;
        else if (0 == GPIO_ReadInputDataBit(KEY_PORT_6, KEY_PIN_6))
            temp = 4;
        break;
    case 1:
        if (0 == GPIO_ReadInputDataBit(KEY_PORT_3, KEY_PIN_3))
            temp = 1;
        else if (0 == GPIO_ReadInputDataBit(KEY_PORT_4, KEY_PIN_4))
            temp = 2;
        else if (0 == GPIO_ReadInputDataBit(KEY_PORT_5, KEY_PIN_5))
            temp = 3;
        else if (0 == GPIO_ReadInputDataBit(KEY_PORT_6, KEY_PIN_6))
            temp = 4;
        break;
    case 2:
        if (0 == GPIO_ReadInputDataBit(KEY_PORT_4, KEY_PIN_4))
            temp = 2;
        else if (0 == GPIO_ReadInputDataBit(KEY_PORT_5, KEY_PIN_5))
            temp = 3;
        else if (0 == GPIO_ReadInputDataBit(KEY_PORT_6, KEY_PIN_6))
            temp = 4;
        break;
    case 3:
        if (0 == GPIO_ReadInputDataBit(KEY_PORT_5, KEY_PIN_5))
            temp = 3;
        else if (0 == GPIO_ReadInputDataBit(KEY_PORT_6, KEY_PIN_6))
            temp = 4;
        break;
    case 4:
        if (0 == GPIO_ReadInputDataBit(KEY_PORT_6, KEY_PIN_6))
            temp = 4;
        break;
    case 0xff:
        if (0 == GPIO_ReadInputDataBit(KEY_PORT_1, KEY_PIN_1))
            temp = 0;
        else if (0 == GPIO_ReadInputDataBit(KEY_PORT_2, KEY_PIN_2))
            temp = 1;
        else if (0 == GPIO_ReadInputDataBit(KEY_PORT_3, KEY_PIN_3))
            temp = 2;
        break;
    default:
        break;
    }
    return temp;
#endif
}

static uint8_t GetKeyCode(void)
{
    uint8_t i, keyvalue;
#if (BOARD_VER == MPOS_BOARD_V2_1)
    for (i = 0; i < 4; i++)
    {
        Key_Gpio_Init_IPU();
        delay_ms(5);

        keyvalue = Key_GetIoValue(0xff);

        if (keyvalue != 0xff)
        {
            return  keyMatrix[4][keyvalue];
        }

        Key_IODirection(i);
        delay_ms(SHAKE_DELAY);
        keyvalue = Key_GetIoValue(i);
        if (keyvalue != 0xff)
        {
            return (keyMatrix[keyvalue - 1][i]);
        }
    }

    return 0;
#elif (BOARD_VER == EPOS_BOARD_V1_0)
    Key_Gpio_Init_IPU();
    delay_ms(1);

    keyvalue = Key_GetIoValue(0xff);

    if (keyvalue != 0xff)
    {
        return  keyMatrix[5][keyvalue];
    }

    for (i = 0; i < 5; i++)
    {
        Key_Gpio_Init_IPU();
        delay_ms(1);

        Key_IODirection(i);
        delay_ms(SHAKE_DELAY);

        keyvalue = Key_GetIoValue(i);
        if (keyvalue != 0xff)
        {
            return (keyMatrix[keyvalue][i]);
        }
    }

    return 0;
#endif
}

uint8_t KEY_Scan(void)
{
    uint8_t KeyCode = 0, KeyCode2 = 0;

    KeyCode = GetKeyCode();
    if (KeyCode != 0)
    {
        delay_ms(SHAKE_DELAY);
        KeyCode2 = GetKeyCode();
        if (KeyCode2 == KeyCode)
        {
            return KeyCode2;
        }
    }

    return 0;
}

void KEY_Scan_check(void)
{
    while (GetKeyCode());
}
