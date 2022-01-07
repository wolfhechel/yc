
uint16_t ISO7816_RecBytes(uint8_t *RxBuf, uint16_t RxLen);

uint16_t ISO7816_SendBytes_T0(uint8_t *pBuff, uint16_t Len);

uint16_t ISO7816_TPDU_T0(uint8_t *pCommand, uint16_t CommandLength,
                         uint8_t *pResponse, uint16_t *pResponseLength);


