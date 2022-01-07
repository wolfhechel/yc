
uint16_t ISO7816_SendBytes_T1(uint8_t *pBuff, uint16_t Len, uint8_t state);

uint16_t ISO7816_TPDU_T1Send(uint8_t *pCommand, uint16_t CommandLength, uint8_t Type);
uint16_t ISO7816_TPDU_T1(uint8_t *pCommand, uint16_t CommandLength,
                         uint8_t *pResponse,  uint16_t *pResponseLength);


