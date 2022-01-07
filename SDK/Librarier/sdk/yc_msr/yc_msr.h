/*
File Name    : msr.h
Author       : Yichip
Version      : V1.0
Date         : 2018/5/30
Description  : MSR encapsulation
*/

#ifndef __YC_MSR_H__
#define __YC_MSR_H__

#include "yc3121.h"
#include "yc_timer.h"

//Track select option
#define TRACK_SELECT_1	 0x01
#define	TRACK_SELECT_2	 0x02
#define	TRACK_SELECT_3	 0x04
#define	TRACK_SELECT_ALL 0x10		//Return read data that all specific track hava data.


#define	MAX_TRACK_DATA_SIZE		128
#define MAX_TRACK_RAW_DATA_SZIE 512
#define MAX_TRACK_NUM			3


//Detect swiping card,detect_swiping_card() return value.
#define	DETECT_SWIPING_CARD			0x01
#define	DETECT_NO_SWIPING_CARD		0x02
#define	DETECT_TIMEOUT				0x03
#define	DETECT_HARD_WAKEUP			0x04

//Read card data result
#define	MSR_SUCCESS					0
#define INVALID_ADDR		1		//track1 or track2 or track3 address invalid
#define	PARITY_ERR			2		//parity  error
#define	LENGTH_ERR			3		//track data length error
#define	TRACK_NO_DATA		4		//track have no data that to decode.
#define	HAVE_NO_ZERO		5



typedef struct track_data_s
{
    int len;
    uint8_t buf[MAX_TRACK_DATA_SIZE];
}track_data;


//Swipe direction
typedef enum swiping_direction_e
{
    SD_BIDIRECTION			= 0,			//bidirection
    SD_FORWARD_DIRECTION	= 1,
    SD_REVERSE_DIRECTION	= 2
}swiping_direction;


//Card type, detect card type when swipe card.
typedef enum card_type_e
{
    CARD_TYPE_ISO_ABA	= 0,
    CARD_TYPE_AAMVA		= 1,
    CARD_TYPE_IBM		= 2,
    CARD_TYPE_IBM_755	= 3,
    CARD_TYPE_JIS		= 4,
    CARD_TYPE_777		= 5,
    CARD_TYPE_CADMV		= 6,
    CARD_TYPE_OTHER		= 7,
    CARD_TYPE_UNKNOW	= 0xFF
}card_type;


//track signal select
typedef enum signal_fortrack
{
        signal_t3 = 0,
        signal_t1 = 1,
        signal_t2 = 2
} select_signal;

/**
 * @brief Initialize MSR
 *
 * @param none
 *
 * @retval none
 */
void MSR_Init(void);

/**
 * @brief deInitialize MSR
 *
 * @param none
 *
 * @retval none
 */
void MSR_DeInit(void);


/**
 * @brief Set swiping direction.
 * @param none :
         dir		swipe_direction

 * @retval none
 */
void MSR_SetSwipeDir(swiping_direction dir);

/**
 * @brief Get current swiping direction.
 * @param none :
         dir		swipe_direction
 * @retval none
 */
swiping_direction MSR_GetSwipeDir(void);

/**
 * @brief Detecting MSR card.
 *
 * @param  :
 *			DETECT_SWIPING_CARD		Have swiping card action.
 *			DETECT_NO_SWIPING_CARD	Haven't swiping card action.
 *			DETECT_HARD_WAKEUP		May be set hard wakeup.
 *
 * @retval none
 */
 uint8_t MSR_DetectSwipingCard(void);

/**
 * @brief	: Get decode data that specified track.
 * @param  :
 *			option			track select option, bit 0, 1, 2 to track 1, 2, 3. bit 4 to all track.
 *			ptrack			track 1,2 and 3 buffer, at least have 3 items.(output)
 *			cfmt				card format @ card_type
 *			tflag 			bit0 bit1 bit2 is the decode result . 1:success 0:failed
 * retval:
 *			MSR_SUCCESS		decoding successful finished.
 *			INVALID_ADDR	track buffer address is invalid.
 *			PARITY_ERR		parity error.
 *			LENGTH_ERR		length exceed max length.
 *			TRACK_NO_DATA	part track have no data that must readed
*/
uint8_t MSR_GetDecodeData(track_data  *ptrack, uint8_t options, uint8_t *cfmt, uint8_t *tflag);

 /**
 * @brief	:  Get special track raw data, nothing to do within get operation.
 * @param  :
 *			buf			save track raw data buffer address
                        (Store by track code number start from bit 0)

 *			track_idx		track index, 0~2 for track 1~3
 * @retval: data len
 * @description Every byte 0-4bit is effective ,5-7bit is useless;get rawdata must befaor decode data.
*/
int  MSR_GetTrackRawData(uint8_t  *buf, uint8_t track_idx);

 /**
 * @brief	:  ENABLE or DISABLE track interrupt.
 * @param  : NewState: ENABLE or DISABLE
 *					 TrackSel: bit 0 for track 1;bit 1 for track 2;bit 2 for track 3;bit 4 for all track
*/
void MSR_ITEnable(FunctionalState NewState, uint8_t TrackSel);

 /**
 * @brief	: Clear MSR interrupt.
 * @param  :TrackSel: bit 0 for track 1;bit 1 for track 2;bit 2 for track 3;bit 4 for all track.
*/
void  MSR_ClearIT(uint8_t TrackSel );

 /**
 * @brief	:MSR Module software reset
 * @param  none
**/
void MSR_SoftRest(void);

/**
 * @brief	:MSR Module ENABLE
 * @param £ºNewState :ENABLE or DISABLE
**/
void MSR_ENABLE(FunctionalState NewState);

/**
 * @brief	:read the  track data  write to memory down statu.
 * @param £ºTrackSel :TRACK_SELECT_1, TRACK_SELECT_2, TRACK_SELECT_3 ,TRACK_SELECT_ALL
**/
Boolean MSR_CheckTrackWRDone(uint8_t TrackSel);


/**
 * @brief	MSR AGC CONFIG
 * @param £ºtrack :TRACK_SELECT_1, TRACK_SELECT_2, TRACK_SELECT_3
 * @param £ºgain_fixed :max value is 0x3f
**/
void MSR_agc_config(int track,  uint32_t gain_fixed);


/**
 * @brief	MSR track select(call after MSR_Init)
 * @param £ºtrack :TRACK_SELECT_1, TRACK_SELECT_2 or TRACK_SELECT_3
 * @return £ºnone
**/
void MSR_track_select(int track);


/**
 * @brief  get MSR lib version
 * @param  none
 * @return lib version:
 *					bit0~bit15:low version number
 *					bit0~bit15:high version number
 */
uint32_t MSR_GetVersion(void);
#endif /*__YC_MSR_H__*/

/*-----------------------------------*/
