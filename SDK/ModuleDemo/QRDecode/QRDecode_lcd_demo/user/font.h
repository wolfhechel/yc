#ifndef __FONT_H__
#define __FONT_H__

//常用ASCII表
//偏移量32
//ASCII字符集: !"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~
//PC2LCD2002取模方式设置：阴码+逐列式+顺向+C51格式
//总共：2个字符集（16*16和24*24），用户可以自行新增其他分辨率的字符集。
//每个字符所占用的字节数为:(size/8+((size%8)?1:0))*(size/2),其中size:是字库生成时的点阵大小(12/16/24...)

//1608 ASCII字符集点阵
extern const unsigned char asc2_1608[95][16];

//2412 ASICII字符集点阵
extern const unsigned char asc2_2412[95][36];

//3216 ASICII字符集点阵
extern const unsigned char asc2_3216[95][64];

/* YiChip图标 */
extern const unsigned char gImage_Yichip[7208];

/* 电量低 */
extern const unsigned char gImage_low_power[1232];

/* 电量1 */
extern const unsigned char gImage_power_1[1232];

/* 电量2 */
extern const unsigned char gImage_power_2[1232];

/* 电量3 */
extern const unsigned char gImage_power_3[1232];

/* 满电 */
extern const unsigned char gImage_full_energe[1232];

/* 充电 */
extern const unsigned char gImage_chrg_in[1232];

/* 无信号 */
extern const unsigned char gImage_single_l[1452];

/* 信号1 */
extern const unsigned char gImage_single1[1452];

/* 信号2 */
extern const unsigned char gImage_single2[1452];

/* 信号3 */
extern const unsigned char gImage_single3[1452];

/* 满信号 */
extern const unsigned char gImage_signal_h[1452];

/* 易兆微电子 */
extern const unsigned char Yichip_ch[];

/* 欢迎使用 */
extern const unsigned char Welcom_ch[];

/* 主菜单界面 */
extern const unsigned char xiaofei_ch[];
extern const unsigned char saoyisao_ch[];
extern const unsigned char chexiao_ch[];
extern const unsigned char tupianceshi_ch[];
extern const unsigned char guanli_ch[];
extern const unsigned char jiaoyichaxun_ch[];
extern const unsigned char jiesuan_ch[];
extern const unsigned char TPTest_ch[];
extern const unsigned char CARDTest_ch[];
extern const unsigned char cancelback_ch[];
extern const unsigned char calibrated_ch[];
extern const unsigned char confirm_ch[];
extern const unsigned char cancel_ch[];
extern const unsigned char clear_ch[];

/* 界面内容 */
extern const unsigned char shurujinge_ch[];
extern const unsigned char shurucuowu_ch[];

extern const unsigned char CARD_ch[];
extern const unsigned char gImage_msr[20008];
extern const unsigned char gImage_nfc[12808];
extern const unsigned char gImage_iccard[13778];

extern const unsigned char gImage_pic_test1[153608];
extern const unsigned char gImage_pic_test2[153608];

#endif
