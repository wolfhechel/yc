/*
File Name    : yc_touch.c
Author       : Yichip
Version      : V1.0
Date         : 2020/1/14
Description  :
*/

#include "yc_touch.h"

Tp_Calibrate_CB DisplaySample[5];
Tp_Calibrate_CB ScreenSample[5];
Tp_Calibrate_CB PegPont;
Tp_Calibrate_CB CaliPont;
Tp_CalibrateSlope_CB fac;
Tp_CalibrateSlope_CB Constant;

/**
  * @brief  Gets the level difference result
  * @param  diff_value : value1
  *         com_value  : value2
  * @retval TRUE : in range ,FALSE: not in range
  */
static Boolean Tp_Get_Level_Result(uint32_t diff_value,uint32_t com_value)
{
    if((diff_value > (com_value + TP_LEVEL_DIFF)) || (diff_value < (com_value - TP_LEVEL_DIFF)))
    {
        return FALSE;
    }
    return TRUE;
}

/**
  * @brief  Get the ad value
  * @param  Axis :
  *         X_AXIS  : x axis
  *         Y_AXIS  : y axis
  * @retval  ad value
  */

static uint16_t Tp_GetCoordVal(TP_Axis_TypeDef Axis)
{
    uint8_t detect_chancl = 0;
    if(Axis == X_AXIS)
    {
        GPIO_Config(TP_Y_LOW_PORT, TP_Y_LOW_IO_PIN, OUTPUT_LOW);
        GPIO_Config(TP_Y_HIGH_PORT, TP_Y_HIGH_IO_PIN, OUTPUT_HIGH);
        GPIO_Config(TP_X_HIGH_PORT, TP_X_HIGH_IO_PIN, ANALOG);
        GPIO_Config(TP_X_LOW_PORT, TP_X_LOW_IO_PIN, ANALOG);
        detect_chancl = Y_DETECT_CHANNEL;
    }
    else if(Axis == Y_AXIS)
    {
        GPIO_Config(TP_Y_LOW_PORT, TP_Y_LOW_IO_PIN, ANALOG);
        GPIO_Config(TP_Y_HIGH_PORT, TP_Y_HIGH_IO_PIN, ANALOG);
        GPIO_Config(TP_X_HIGH_PORT, TP_X_HIGH_IO_PIN, OUTPUT_HIGH);
        GPIO_Config(TP_X_LOW_PORT, TP_X_LOW_IO_PIN, OUTPUT_LOW);
        detect_chancl = X_DETECT_CHANNEL;
    }
    delay_ms(1);
    return ADC_GetResult(detect_chancl);
}

/**
  * @brief  Get the x axis and  y axis  ad value
  * @param  adx : The address to which the x axis AD value is stored
  *         ady : The address to which the y axis AD value is stored
  * @retval  None
  */

void Tp_GetAdXY(uint16_t *adx,uint16_t *ady)
{
    *adx = Tp_GetCoordVal(X_AXIS);
    *ady = Tp_GetCoordVal(Y_AXIS);
    return ;
}

/**
  * @brief  Get the x axis and  y axis displayed coordinates
  * @param  Disx : The address to which the x axis  displayed coordinates is stored
  *         Disy : The address to which the y axis displayed coordinates is stored
  * @retval  None
  */

void Tp_GetPoint(uint16_t *Disx,uint16_t*Disy)
{
    /*get adc*/
    Tp_GetAdXY(Disx,Disy);

    /*get display coordinate*/
    *Disx = Tp_AbsolouteDifference(*Disx ,Constant.kx) * 100 / fac.kx;
    *Disy = Tp_AbsolouteDifference(*Disy ,Constant.ky) * 100 / fac.ky;
}

/**
  * @brief  Check whether the touch screen is pressed
  * @param  None
  * @retval TRUE : press FALSE : no press
  */
Boolean Tp_DetectPressing(void)
{
    uint16_t AdcPress = 0;
    GPIO_Config(TP_Y_LOW_PORT, TP_Y_LOW_IO_PIN, OUTPUT_LOW);
    GPIO_Config(TP_Y_HIGH_PORT, TP_Y_HIGH_IO_PIN, OUTPUT_HIGH);
    GPIO_Config(TP_X_HIGH_PORT, TP_X_HIGH_IO_PIN, ANALOG);
    GPIO_Config(TP_X_LOW_PORT, TP_X_LOW_IO_PIN, PULL_UP);
    AdcPress = ADC_GetResult(PRESS_DETECT_CHANNEL);

    if((AdcPress + 3)  < REGULT_ADC_MAX)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/**
  * @brief  Draw cross 
  * @param  x: x axis
  *         y: y axis
  *         c: color
  * @retval None
  */
static void Tp_Draw_Cross(uint16_t x,uint16_t y,uint16_t c)
{
    uint8_t s_offect = 10;
    /*get screen type tft or lcd*/
    St7899_tft_DrawLine(x - s_offect,y,x + s_offect,y,c);
    St7899_tft_DrawLine(x,y - s_offect,x ,y+s_offect,c);
}

/**
  * @brief  Gets the filtered AD value
  * @param  None
  * @retval The x and y coordinates returned
  */
static Tp_Calibrate_CB *Read_Adc_Result(void)
{
    uint8_t t = 0;
    uint8_t t1 = 0;
    uint8_t count = 0;
    uint16_t temp = 0;
    uint16_t Tpx = 0;
    uint16_t Tpy = 0;
    static Tp_Calibrate_CB screen;
    int databuffer[2][10] = {{0},{0}};
    while(count < 6)
    {
        if(Tp_DetectPressing())
        {
            Tp_GetAdXY(&Tpx,&Tpy);
            databuffer[0][count] = Tpx;
            databuffer[1][count] = Tpy;
            TP_PRINT("0x%x,0x%x\n", databuffer[0][count], databuffer[1][count]);
            count ++;
        }
    }
    if(count == 6)
    {
        do
        {
            t1 = 0;
            for(t = 0; t < count -1 ; t++)
            {
                if(databuffer[0][t] > databuffer[0][t + 1])
                {
                    temp=databuffer[0][t+1];
                    databuffer[0][t+1]=databuffer[0][t];
                    databuffer[0][t]=temp;
                    t1=1;
                }
            } 
        }while(t1); // x sort
        do
        {
            t1=0;
			for(t=0;t<count-1;t++)
			{
				if(databuffer[1][t]>databuffer[1][t+1])
				{
					temp=databuffer[1][t+1];
					databuffer[1][t+1]=databuffer[1][t];
					databuffer[1][t]=temp;
					t1=1;
				}
			}
        }while(t1); // y sort
        screen.x = 0;
        screen.y = 0;
        for( count = 2; count < 5; count ++)
        {
            screen.x += databuffer[0][count];
            screen.y += databuffer[1][count];
        }
        screen.x /= 3;
        screen.y /= 3;
        TP_PRINT("screen.x : 0x%x,screen.y : 0x%x \n",screen.x,screen.y);
        for(count = 2; count < 5; count ++)
        {
            if((((screen.x-databuffer[0][count])<4)
            &&((databuffer[0][count]-screen.x)<4))
            &&(((screen.y-databuffer[1][count])<4)
            &&((databuffer[1][count]-screen.y)<4)))
            {
                ;/*continue detection*/
            }
            else
            {
                return 0;/*data exception*/
            }
        }
        return &screen;
    }
    return 0;
}

/**
  * @brief  Touch the calibration
  * @param  None
  * @retval TRUE : calibration success, FALSE : calibration fail
  */

Boolean Tp_Calibrate(void)
{
    uint8_t i = 0;
    int level_diff_buff[4] = {0};
    int dis_diff_buff[4] = {0};
    Tp_Calibrate_CB *ptr;
    /* read calibrate data to flash */
    /* get screen type tft or lcd */
    DisplaySample[0].x = LEFT_UP_X;
    DisplaySample[0].y = LEFT_UP_Y;
    DisplaySample[1].x = RIGHT_UP_X;
    DisplaySample[1].y = RIGHT_UP_Y;
    DisplaySample[2].x = LEFT_DOWN_X;
    DisplaySample[2].y = LEFT_DOWN_Y;
    DisplaySample[3].x = RIGHT_DOWN_X;
    DisplaySample[3].y = RIGHT_DOWN_Y;
    DisplaySample[4].x = MIDDLE_X;
    DisplaySample[4].y = MIDDLE_Y;

    uint8_t size_sample =  sizeof(DisplaySample)/sizeof(Tp_Calibrate_CB);
    TP_PRINT("szie_d : %d\n",sizeof(DisplaySample)/sizeof(Tp_Calibrate_CB));
    
    for(i = 0; i< size_sample ; i++)
    {
        /*Please align the calibration with the cross, otherwise the calibration will not work*/
        Tp_Draw_Cross(DisplaySample[i].x,DisplaySample[i].y,BLACK);
        
        do
        {
            ptr = Read_Adc_Result();
        }
        while(ptr == (void*)0);
        
        ScreenSample[i].x = ptr->x;
        ScreenSample[i].y = ptr->y;
        
        Tp_Draw_Cross(DisplaySample[i].x,DisplaySample[i].y,WHITE);
        
        if(i< (size_sample - 1))
        {
            delay_ms(500);
        }
    }
    
    /*Calculate the ADC difference*/
    /*x0 ~ x2*/
    level_diff_buff[0] = Tp_AbsolouteDifference(ScreenSample[0].x,ScreenSample[2].x);
    
    /*x1 ~ x3*/
    level_diff_buff[1] = Tp_AbsolouteDifference(ScreenSample[1].x,ScreenSample[3].x);
    
    /*y0 ~ y1*/
    level_diff_buff[2] = Tp_AbsolouteDifference(ScreenSample[0].y,ScreenSample[1].y);
    
    /*y2 ~ y3*/
    level_diff_buff[3] = Tp_AbsolouteDifference(ScreenSample[2].y,ScreenSample[3].y);
    
    for(i = 0; i < 4 ; i ++)
    {
        if(Tp_Get_Diff_Result(level_diff_buff[i],TP_LEVEL_DIFF) == FALSE)
        {
            TP_PRINT("level_diff_buff[%d] = %d  err \n",i,level_diff_buff[i] );
            return FALSE;
        }
    }
    
    /*x0 ~ x1*/
    dis_diff_buff[0] = Tp_AbsolouteDifference(ScreenSample[0].x,ScreenSample[1].x);
    
    /*x2 ~ x3*/
    dis_diff_buff[1] = Tp_AbsolouteDifference(ScreenSample[2].x,ScreenSample[3].x);
    

    if(Tp_Get_Level_Result(dis_diff_buff[0],dis_diff_buff[1]) == FALSE || dis_diff_buff[0] < TOP_SIZE_X || dis_diff_buff[1] < TOP_SIZE_X)
    {
        TP_PRINT("dis_diff_buffx[0] : %d,dis_diff_buffx[1] : %d err \n",dis_diff_buff[0],dis_diff_buff[1]);
        return FALSE;
    }
    
    
    /*y0 ~ y2*/
    dis_diff_buff[2] = Tp_AbsolouteDifference(ScreenSample[0].y,ScreenSample[2].y);
    
    /*y1 ~ y3*/
    dis_diff_buff[3] = Tp_AbsolouteDifference(ScreenSample[1].y,ScreenSample[3].y);
    
    if(Tp_Get_Level_Result(dis_diff_buff[2],dis_diff_buff[3]) == FALSE || dis_diff_buff[2] < TOP_SIZE_Y || dis_diff_buff[3] < TOP_SIZE_Y)
    {
        TP_PRINT("dis_diff_buffy[%d] : %d err \n",i,dis_diff_buff[i] );
        return FALSE;
    }
    
    
    /*Calculation of kx ky*/
    
     fac.kx = (((dis_diff_buff[0] * 100) /  (Tp_AbsolouteDifference(DisplaySample[0].x,DisplaySample[1].x))) + \
               ((dis_diff_buff[1] * 100) /  (Tp_AbsolouteDifference(DisplaySample[2].x,DisplaySample[3].x))))/2 ;
    
        
     fac.ky = (((dis_diff_buff[2] * 100) /  (Tp_AbsolouteDifference(DisplaySample[0].y,DisplaySample[2].y))) + \
           ((dis_diff_buff[3] * 100) /  (Tp_AbsolouteDifference(DisplaySample[1].y,DisplaySample[3].y))))/2 ;

    Constant.kx = ScreenSample[0].x + ((DisplaySample[0].x)* fac.kx)/100;// y = -kx + b
    Constant.ky = ScreenSample[0].y + ((DisplaySample[0].y)* fac.ky)/100;

    TP_PRINT("fac.kx : %d __ fac.ky: %d\n",fac.kx,fac.ky);
    TP_PRINT("constant.kx : %d __ constant.ky: %d\n",Constant.kx,Constant.ky);
    
    /*Calculate the coordinate of the center point from kx, ky,DisplaySample[4].x,DisplaySample[4].y*/
    
    PegPont.x = Tp_AbsolouteDifference(fac.kx*DisplaySample[4].x/100,Constant.kx);
    PegPont.y = Tp_AbsolouteDifference(fac.ky*DisplaySample[4].y/100,Constant.ky);
    
    TP_PRINT("PegPont.x : %x __ PegPont.y: %x\n",PegPont.x,PegPont.y);
    
    /*Check the calculation coordinates and measurement coordinates of the center point*/
    
    
    uint16_t center_x = Tp_AbsolouteDifference(ScreenSample[4].x,PegPont.x);
    uint16_t center_y = Tp_AbsolouteDifference(ScreenSample[4].y,PegPont.y);
    
    TP_PRINT("center_x: %d __ center_y: %d\n",center_x,center_y);
    
    CaliPont.x = DisplaySample[4].x;
    CaliPont.y = DisplaySample[4].y;

    TP_PRINT(" CaliPont.x = %d\n",CaliPont.x);
    TP_PRINT(" CaliPont.y = %d\n",CaliPont.y);
    
    if(Tp_Get_Diff_Result(center_x,TP_LEVEL_DIFF) ==  FALSE || Tp_Get_Diff_Result(center_y,TP_LEVEL_DIFF) == FALSE)
    {
        TP_PRINT("Tp_Get_Diff_Result center err \n");
        return FALSE;
    }
    
    /*Record calibration value*/
    
    TP_PRINT(" fac.kx : %d \n",  fac.kx);
    TP_PRINT(" fac.ky : %d \n", fac.ky);
    TP_PRINT(" PegPont.x : %x \n", PegPont.x);
    TP_PRINT(" PegPont.y : %x \n", PegPont.y);
    
    return TRUE;
}
