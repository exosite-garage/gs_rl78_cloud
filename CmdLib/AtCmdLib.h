/*-------------------------------------------------------------------------*
 * File:  AtCmdLib.h
 *-------------------------------------------------------------------------*
 * Description:
 *      The GainSpan AT Command Library (AtCmdLib) provides the functions
 *      that send AT commands to a GainSpan node and looks for a response.
 *      Parse commands are provided to interpret the response data.
 *-------------------------------------------------------------------------*/
#ifndef _GS_ATCMDLIB_H_
#define _GS_ATCMDLIB_H_

#include <stdint.h>
#include <stdbool.h>

/* Parsing related defines */
#define  HOST_APP_TCP_CLIENT_CID_OFFSET_BYTE        (8)  /* CID parameter offset in TCP connection response */
#define  HOST_APP_UDP_CLIENT_CID_OFFSET_BYTE        (8)  /* CID parameter offset in UDP connection response */
#define  HOST_APP_BULK_DATA_LEN_STRING_SIZE         (4)  /* Number of octets representing the data lenght field in bulk data transfer message */
#define  HOST_APP_RAW_DATA_STRING_SIZE_MAX          (4)  /* Number of octets representing the data lenght field in raw data transfer message*/
#define  HOST_APP_HTTP_RESP_DATA_LEN_STRING_SIZE    (4)  /* Number of octets representing the data lenght field in HTTP data transfer message*/
#define  HOST_APP_INVALID_CID                    (0xFF)  /* invalid CID */

typedef enum {
    HOST_APP_RET_STATUS_ERROR = 0,
    HOST_APP_RET_STATUS_OK = 1,
    HOST_APP_RET_STATUS_VALID_DATA = 2,
    HOST_APP_RET_STATUS_CONTROL_CODE = 3,
    HOST_APP_RET_STATUS_MAX
} HOST_APP_RET_STATUS_E;

typedef enum {
    HOST_APP_DISABLE = 0, HOST_APP_ENABLE = 1
} HOST_APP_STATUS_E;

typedef enum {
    HOST_APP_CON_TCP_SERVER,
    HOST_APP_CON_UDP_SERVER,
    HOST_APP_CON_UDP_CLIENT,
    HOST_APP_CON_TCP_CLIENT

} HOST_APP_CON_TYPE;

typedef enum {
    HOST_APP_RX_STATE_START = 0,
    HOST_APP_RX_STATE_CMD_RESP,
    HOST_APP_RX_STATE_ESCAPE_START,
    HOST_APP_RX_STATE_DATA_HANDLE,
    HOST_APP_RX_STATE_BULK_DATA_HANDLE,
    HOST_APP_RX_STATE_HTTP_RESPONSE_DATA_HANDLE,
    HOST_APP_RX_STATE_RAW_DATA_HANDLE,
    HOST_APP_RX_STATE_MAX
} HOST_APP_RX_STATE_E;

typedef enum {
    HOST_APP_MSG_ID_NONE = 0,
    HOST_APP_MSG_ID_OK = 1,
    HOST_APP_MSG_ID_INVALID_INPUT,
    HOST_APP_MSG_ID_ERROR,
    HOST_APP_MSG_ID_ERROR_IP_CONFIG_FAIL,
    HOST_APP_MSG_ID_ERROR_SOCKET_FAIL,
    HOST_APP_MSG_ID_DISCONNECT,
    HOST_APP_MSG_ID_DISASSOCIATION_EVENT,
    HOST_APP_MSG_ID_APP_RESET,
    HOST_APP_MSG_ID_OUT_OF_STBY_ALARM,
    HOST_APP_MSG_ID_OUT_OF_STBY_TIMER,
    HOST_APP_MSG_ID_UNEXPECTED_WARM_BOOT,
    HOST_APP_MSG_ID_OUT_OF_DEEP_SLEEP,
    HOST_APP_MSG_ID_WELCOME_MSG,
    HOST_APP_MSG_ID_STBY_CMD_ECHO,
    HOST_APP_MSG_ID_TCP_CON_DONE,
    HOST_APP_MSG_ID_RESPONSE_TIMEOUT,
    HOST_APP_MSG_ID_BULK_DATA_RX,
    HOST_APP_MSG_ID_DATA_RX,
    HOST_APP_MSG_ID_RAW_DATA_RX,
    HOST_APP_MSG_ID_ESC_CMD_OK,
    HOST_APP_MSG_ID_ESC_CMD_FAIL,
    HOST_APP_MSG_ID_HTTP_RESPONSE_DATA_RX,
    HOST_APP_MSG_ID_MAX
} HOST_APP_MSG_ID_E;

#define  HOST_APP_CR_CHAR          0x0D     /* octet value in hex representing Carriage return    */
#define  HOST_APP_LF_CHAR          0x0A     /* octet value in hex representing Line feed             */
#define  HOST_APP_ESC_CHAR         0x1B     /* octet value in hex representing application level ESCAPE sequence */

/* Special characters used for data mode handling */
#define  HOST_APP_DATA_MODE_NORMAL_START_CHAR_S      'S'
#define  HOST_APP_DATA_MODE_NORMAL_END_CHAR_E        'E'
#define  HOST_APP_DATA_MODE_BULK_START_CHAR_Z        'Z'
#define  HOST_APP_DATA_MODE_BULK_START_CHAR_H        'H'
#define  HOST_APP_DATA_MODE_RAW_INDICATION_CHAR_COL  ':'
#define  HOST_APP_DATA_MODE_RAW_INDICATION_CHAR_R    'R'
#define  HOST_APP_DATA_MODE_ESC_OK_CHAR_O            'O'
#define  HOST_APP_DATA_MODE_ESC_FAIL_CHAR_F          'F'
#define HOST_APP_DATA_MODE_BULK_START_CHAR_G         'G'
#define HOST_APP_DATA_MODE_BULK_START_CHAR_K         'K'
/********** Following control octets are  used by SPI driver layer **********/
#define  HOST_APP_SPI_ESC_CHAR               (0xFB)    /* Start transmission indication */
#define  HOST_APP_SPI_IDLE_CHAR              (0xF5)    /* synchronous IDLE */
#define  HOST_APP_SPI_XOFF_CHAR              (0xFA)    /* Stop transmission indication */
#define  HOST_APP_SPI_XON_CHAR               (0xFD)    /* Start transmission indication */
#define  HOST_APP_SPI_INVALID_CHAR_ALL_ONE   (0xFF)    /* Invalid character possibly recieved during reboot */
#define  HOST_APP_SPI_INVALID_CHAR_ALL_ZERO  (0x00)    /* Invalid character possibly recieved during reboot */
#define  HOST_APP_SPI_READY_CHECK            (0xF3)    /* SPI link ready check */
#define  HOST_APP_SPI_READY_ACK              (0xF3)    /* SPI link ready response */
#define  HOST_APP_SPI_CTRL_BYTE_MASK         (0x80)    /* Control byte mask */

/*********** End of  SPI driver layer  specific defines**********************/
#define HOST_APP_TX_CMD_MAX_SIZE             (256)
#define HOST_APP_RX_CMD_MAX_SIZE             (512)

/* this should be tied to a timer, faster processor larger the number */
//#define  HOST_APP_RESPONSE_TIMEOUT_COUNT    (0xFFFFFFFF) /* Time out value for reponse message */
#define  HOST_APP_RESPONSE_TIMEOUT_COUNT    (0x2FFFFFF) // OK for SPI
/*************<Function prototye forward delarations >***************/

HOST_APP_MSG_ID_E AtLib_CommandSend(void);
HOST_APP_MSG_ID_E AtLibGs_Check(void);
HOST_APP_MSG_ID_E AtLibGs_SetEcho(uint8_t mode);
HOST_APP_MSG_ID_E AtLibGs_MACSet(int8_t *pAddr);
HOST_APP_MSG_ID_E AtLibGs_CalcNStorePSK(int8_t *pSsid, int8_t *pPsk);
HOST_APP_MSG_ID_E AtLibGs_WlanConnStat(void);
HOST_APP_MSG_ID_E AtLibGs_DHCPSet(uint8_t mode);
HOST_APP_MSG_ID_E AtLibGs_Assoc(int8_t *pSsid, int8_t *pBssid, int8_t *pChan);
HOST_APP_MSG_ID_E AtLibGs_TcpClientStart(
        int8_t *pRemoteTcpSrvIp,
        int8_t *pRemoteTcpSrvPort);
HOST_APP_MSG_ID_E AtLibGs_UdpClientStart(
        int8_t *pRemoteUdpSrvIp,
        int8_t *pRemoteUdpSrvPort,
        int8_t *pUdpLocalPort);

HOST_APP_MSG_ID_E AtLibGs_CloseAll(void);
HOST_APP_MSG_ID_E AtLibGs_ATA(void);
HOST_APP_MSG_ID_E AtLibGs_ATZ1(void);


HOST_APP_MSG_ID_E AtLibGs_BatteryChkStart(uint32_t interval);
HOST_APP_MSG_ID_E AtLibGs_GotoSTNDBy(
        int8_t *nsec,
        uint32_t dealy,
        uint32_t alarm1_Pol,
        uint32_t alarm2_Pol);
HOST_APP_MSG_ID_E AtLibGs_PSK(int8_t *pPsk);
HOST_APP_MSG_ID_E AtLibGs_SetWEP1(int8_t *pPsk);
HOST_APP_MSG_ID_E AtLibGs_EnableDeepSleep(void);
HOST_APP_MSG_ID_E AtLibGs_StoreNwConn(void);
HOST_APP_MSG_ID_E AtLibGs_ReStoreNwConn(void);
HOST_APP_MSG_ID_E AtLibGs_IPSet(
        int8_t* pIpAddr,
        int8_t* pSubnet,
        int8_t *pGateway);

HOST_APP_MSG_ID_E AtLibGs_SaveProfile(uint8_t profile);
HOST_APP_MSG_ID_E AtLibGs_LoadProfile(uint8_t profile);
HOST_APP_MSG_ID_E AtLibGs_ResetFactoryDefaults(void);
HOST_APP_MSG_ID_E AtLibGs_GetRssi(void);
HOST_APP_MSG_ID_E AtLibGs_DisAssoc(void);
HOST_APP_MSG_ID_E AtLibGs_FWUpgrade(
        int8_t *pSrvip,
        uint32_t srvport,
        uint32_t srcPort,
        int8_t *pSrcIP);
HOST_APP_MSG_ID_E AtLibGs_BatteryChkStop(void);
HOST_APP_MSG_ID_E AtLibGs_MultiCast(uint8_t mode);
HOST_APP_MSG_ID_E AtLibGs_Version(void);
HOST_APP_MSG_ID_E AtLibGs_Mode(uint32_t mode);
HOST_APP_MSG_ID_E AtLibGs_UdpServer_Start(int8_t *pUdpSrvPort);
HOST_APP_MSG_ID_E AtLibGs_TcpServer_Start(int8_t* pTcpSrvPort);
HOST_APP_MSG_ID_E AtLibGs_DNSLookup(int8_t* pUrl);
HOST_APP_MSG_ID_E AtLibGs_Close(uint8_t cid);
HOST_APP_MSG_ID_E AtLibGs_SetWRetryCount(uint32_t count);
HOST_APP_MSG_ID_E AtLibGs_GetErrCount(void);
HOST_APP_MSG_ID_E AtLibGs_EnableRadio(uint8_t mode);
HOST_APP_MSG_ID_E AtLibGs_EnablePwSave(uint8_t mode);
HOST_APP_MSG_ID_E AtLibGs_SetTime(int8_t* pDate, int8_t *pTime);
HOST_APP_MSG_ID_E AtLibGs_EnableExternalPA(uint8_t mode);
HOST_APP_MSG_ID_E AtLibGs_SyncLossInterval(uint16_t interval);
HOST_APP_MSG_ID_E AtLibGs_PSPollInterval(uint16_t interval);
HOST_APP_MSG_ID_E AtLibGs_SetTxPower(uint32_t power);
HOST_APP_MSG_ID_E AtLibGs_SetDNSServerIP(int8_t *pDNS1, int8_t *pDNS2);
HOST_APP_MSG_ID_E AtLibGs_EnableAutoConnect(uint8_t mode);
void AtLibGs_SwitchFromAutoToCmd(void);
HOST_APP_MSG_ID_E AtLibGs_StoreWAutoConn(int8_t * pSsid, uint8_t channel);
HOST_APP_MSG_ID_E AtLibGs_StoreNAutoConn(int8_t* pIpAddr, int16_t pRmtPort);
HOST_APP_MSG_ID_E AtLibGs_StoreATS(uint8_t param, uint8_t value);
HOST_APP_MSG_ID_E AtLibGs_BData(uint8_t mode);
HOST_APP_MSG_ID_E AtLibGs_StartWPSPUSH(void);
HOST_APP_MSG_ID_E AtLibGs_StartWPSPIN(int8_t* pin);
HOST_APP_MSG_ID_E AtLibGs_SetWAUTH(uint8_t mode);
HOST_APP_MSG_ID_E AtLibGs_SetSecurityType(uint8_t type);
HOST_APP_MSG_ID_E AtLibGs_GetMAC(void);

uint8_t AtLib_ParseTcpClientCid(void);
uint8_t AtLib_ParseUdpClientCid(void);
uint8_t AtLib_ParseWlanConnStat(void);
uint8_t AtLib_ParseGetMacResponse(char *pMAC);
uint8_t AtLib_ParseNodeIpAddress(int8_t *pIpAddr);
uint8_t AtLib_ParseRssiResponse(int16_t *pRssi);
uint8_t AtLib_ParseUdpServerStartResponse(uint8_t *pConnId);
uint8_t AtLib_ParseTcpServerStartResponse(uint8_t *pConnId);
uint8_t AtLib_ParseDNSLookupResponse(char *ipAddr);
uint8_t AtLib_ParseWPSPUSHResponse(void);
uint8_t AtLib_ParseWPSPINResponse(uint32_t pin);

HOST_APP_MSG_ID_E AtLib_CommandSend(void);
void AtLib_DataSend(const uint8_t *pTxData, uint32_t dataLen);
void AtLib_SendTcpData(uint8_t cid, const uint8_t *txBuf, uint32_t dataLen);
void AtLib_SendUdpData(
        uint8_t cid,
        const uint8_t *txBuf,
        uint32_t dataLen,
        HOST_APP_CON_TYPE conType,
        const uint8_t *pUdpClientIP,
        uint16_t udpClientPort);

void AtLib_BulkDataTransfer(uint8_t cid, const uint8_t *pData, uint32_t dataLen);
HOST_APP_MSG_ID_E AtLib_checkEOFMessage(const uint8_t * pBuffer);
void AtLib_ReceiveDataHandle(void);
HOST_APP_MSG_ID_E AtLib_ReceiveDataProcess(uint8_t rxData);
HOST_APP_MSG_ID_E AtLib_ResponseHandle(void);
HOST_APP_MSG_ID_E AtLib_ProcessRxChunk(const uint8_t *rxBuf, uint32_t bufLen);
void AtLib_ProcessIncomingData(uint8_t cid, uint8_t rxData);
void AtLib_LinkCheck(void);
void AtLib_FlushIncomingMessage(void);
uint8_t AtLib_IsNodeResetDetected(void);
void AtLib_SetNodeResetFlag(void);
void AtLib_ClearNodeResetFlag(void);
uint8_t AtLib_IsNodeAssociated(void);
void AtLib_SetNodeAssociationFlag(void);
void AtLib_ClearNodeAssociationFlag(void);

void AtLib_ClearAllCid(void);
void AtLib_SaveTcpCid(uint8_t cid);
void AtLib_SaveUdpCid(uint8_t cid);

void AtLib_ClearAllCid(void);
uint8_t AtLib_GetTcpCid(void);
uint8_t AtLib_GetUdpCid(void);

void AtLib_FlushRxBuffer(void);
int32_t AtLib_strcasecmp(const char *s1, const char *s2);
void AtLib_ConvertNumberTo4DigitASCII(uint32_t myNum, int8_t *pStr);

extern void AtLib_Init(void);

// User supplied routines
extern void App_ProcessIncomingData(uint8_t cid, uint8_t rxData);
void App_DelayMS(uint32_t cnt);
void App_Write(const uint8_t *txData, uint32_t dataLength);
bool App_Read(uint8_t *rxData, uint32_t dataLength, uint8_t blockFlag);

#endif /* _GS_ATCMDLIB_H_ */

/*-------------------------------------------------------------------------*
 * End of File:  AtCmdLib.h
 *-------------------------------------------------------------------------*/
