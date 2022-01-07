#include "yc_exti.h"

static uint16_t ExtiEn[EXIT_Num]                = {0};
static uint16_t ExtiTrig[EXIT_Num]              = {0};  //Rising or Falling interrupt
static uint16_t ExtiRisingFallingTrig[EXIT_Num] = {0};	//Rising and Falling interrupt
static uint16_t ExtiStatus[EXIT_Num]            = {0};

extern void EXTI0_IRQHandler(void);
extern void EXTI1_IRQHandler(void);
extern void EXTI2_IRQHandler(void);

void EXTI_ClearITPendingBit(uint32_t EXTI_Line, uint16_t EXTI_PinSource)
{
    _ASSERT(IS_EXTI_LINE(EXTI_Line));
    _ASSERT(IS_EXTI_PIN_SOURCE(EXTI_PinSource));

    uint16_t GpioTrig = GPIO_TRIG_MODE(EXTI_Line);

    if (GpioTrig & EXTI_PinSource)
    {
        GpioTrig &= ~EXTI_PinSource;
    }
    else
    {
        GpioTrig |= EXTI_PinSource;
    }
    GPIO_TRIG_MODE(EXTI_Line) = GpioTrig;
    ExtiStatus[EXTI_Line]    &= ~EXTI_PinSource;
}

void EXTI_DeInit()
{
    uint32_t i;
    for (i = 0; i < EXIT_Num; i++)
    {
        ExtiEn[i]                = 0;
        GPIO_INTR_EN(i)          = ExtiEn[i];
        ExtiTrig[i]              = 0;
        ExtiRisingFallingTrig[i] = 0;
        ExtiStatus[i]            = 0;
        GPIO_TRIG_MODE(i)        = ExtiTrig[i];
    }
}

uint16_t EXTI_GetITLineStatus(uint32_t EXTI_Line)
{
    return ExtiStatus[EXTI_Line];
}

void EXTI_LineConfig(uint32_t EXTI_Line, uint16_t EXTI_PinSource, EXTI_TriggerTypeDef EXTI_Trigger)
{
    _ASSERT(IS_EXTI_LINE(EXTI_Line));
    _ASSERT(IS_EXTI_PIN_SOURCE(EXTI_PinSource));

    int i;
    uint16_t TempExtiTrig = GPIO_TRIG_MODE(EXTI_Line);

    for (i = 0; i < EXIT_Num; i++)
    {
        if (GPIO_INTR_EN(i) == 0x0000)
        {
            ExtiEn[i]                = 0;
            ExtiTrig[i]              = 0;
            ExtiRisingFallingTrig[i] = 0;
            ExtiStatus[i]            = 0;
        }
    }

    switch (EXTI_Trigger)
    {
    case EXTI_Trigger_Off:
        ExtiEn[EXTI_Line]      &= ~EXTI_PinSource;
        GPIO_INTR_EN(EXTI_Line) = ExtiEn[EXTI_Line];
        break;

    case EXTI_Trigger_Rising:
        ExtiTrig[EXTI_Line]              &= ~EXTI_PinSource;
        TempExtiTrig                     &= ~EXTI_PinSource;
        ExtiRisingFallingTrig[EXTI_Line] &= ~EXTI_PinSource;
        GPIO_TRIG_MODE(EXTI_Line)         = TempExtiTrig;
        ExtiEn[EXTI_Line]                |= EXTI_PinSource;
        GPIO_INTR_EN(EXTI_Line)           = ExtiEn[EXTI_Line];
        break;

    case EXTI_Trigger_Falling:
        ExtiTrig[EXTI_Line]              |= EXTI_PinSource;
        TempExtiTrig                     |= EXTI_PinSource;
        ExtiRisingFallingTrig[EXTI_Line] &= ~EXTI_PinSource;
        GPIO_TRIG_MODE(EXTI_Line)         = TempExtiTrig;
        ExtiEn[EXTI_Line]                |= EXTI_PinSource;
        GPIO_INTR_EN(EXTI_Line)           = ExtiEn[EXTI_Line];
        break;

    case EXTI_Trigger_Rising_Falling:
        ExtiTrig[EXTI_Line] &= ~EXTI_PinSource;
        for (i = 0; i < GPIO_PIN_NUM; i++)
        {
            if (EXTI_PinSource & 1 << i)
            {
                if (GPIO_ReadInputDataBit((GPIO_TypeDef)EXTI_Line, 1 << i))
                {
                    TempExtiTrig |= 1 << i;
                }
                else
                {
                    TempExtiTrig &= ~(1 << i);
                }
            }
        }
        ExtiRisingFallingTrig[EXTI_Line] |= EXTI_PinSource;
        GPIO_TRIG_MODE(EXTI_Line)         = TempExtiTrig;
        ExtiEn[EXTI_Line]                |= EXTI_PinSource;
        GPIO_INTR_EN(EXTI_Line)           = ExtiEn[EXTI_Line];
        break;

    default:
        break;
    }
}

void GPIO_IRQHandler()
{
    disable_intr(INTR_GPIO);

    int i, j;

    for (i = 0; i < EXIT_Num; i++)
    {
        for (j = 0; j < EXIT_Pin_Num; j++)
        {
            if ((ExtiEn[i] & (1 << j)))//Check whether gpio i interrupt are turned on
            {
                if ((((~(GPIO_TRIG_MODE(i))) & (1 << j)) >> j) == GPIO_ReadInputDataBit((GPIO_TypeDef) i, 1 << j))//Check for interruptions
                {

                    if (ExtiRisingFallingTrig[i] & (1 << j))
                    {
                        ExtiStatus[i] |= (1 << j);
                        switch (i)
                        {
                        case EXTI_Line0:
                            EXTI0_IRQHandler();
                            break;

                        case EXTI_Line1:
                            EXTI1_IRQHandler();
                            break;

                        case EXTI_Line2:
                            EXTI2_IRQHandler();
                            break;

                        default:
                            break;
                        }
                        EXTI_ClearITPendingBit(i, 1 << j);
                    }
                    else
                    {
                        if ((ExtiTrig[i] & (1 << j)) == (GPIO_TRIG_MODE(i) & (1 << j)))
                        {
                            ExtiStatus[i] |= (1 << j);
                            switch (i)
                            {
                            case EXTI_Line0:
                                EXTI0_IRQHandler();
                                break;

                            case EXTI_Line1:
                                EXTI1_IRQHandler();
                                break;

                            case EXTI_Line2:
                                EXTI2_IRQHandler();
                                break;

                            default:
                                break;
                            }
                        }
                        EXTI_ClearITPendingBit(i, 1 << j);
                    }
                }
            }
        }
    }
    enable_intr(INTR_GPIO);
}


