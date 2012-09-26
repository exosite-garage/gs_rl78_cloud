/*-------------------------------------------------------------------------*
 * File:  App_Exosit.c
 *-------------------------------------------------------------------------*
 * Description:
 *     This demo reads the temperature and potentiometer and sends the
 *     data to the renesas.exosite.com website using an HTTP connection.
 *     Data is sent every 20 seconds.
 *-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*
 * Includes:
 *-------------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <HostApp.h>
#include <system/platform.h>
#include <CmdLib/AtCmdLib.h>
#include <sensors/Temperature_ADT7420.h>
#include <sensors/Potentiometer.h>
#include <sensors/eeprom.h>
#include <drv/Glyph/lcd.h>
#include <system/mstimer.h>
#include <system/console.h>
#include "Apps.h"

/*-------------------------------------------------------------------------*
 * Constants:
 *-------------------------------------------------------------------------*/
#define EXOSITE_DEMO_UPDATE_INTERVAL            4000 // ms

#define DATA_TO_SEND_TO_SERVER_ALL_LINES \
        "POST /api:v1/stack/alias HTTP/1.1\r\n" \
        "Host: m2.exosite.com\r\n" \
        "X-Exosite-CIK: %s\r\n" \
        "Content-Type: application/x-www-form-urlencoded; charset=utf-8\r\n" \
        "Content-Length: %d\r\n\r\n"   

#define GET_DATA_ALL_LINES \
        "GET /api:v1/stack/alias?%s HTTP/1.1\r\n" \
        "Host: m2.exosite.com\r\n" \
        "X-Exosite-CIK: %s\r\n" \
        "Accept: application/x-www-form-urlencoded; charset=utf-8\r\n\r\n"

#define DEVICE_ACTIVATE_ALL_LINE \
        "POST /provision/activate HTTP/1.1\r\n" \
        "Host: m2.exosite.com\r\n" \
        "Content-Type: application/x-www-form-urlencoded; charset=utf-8\r\n" \
        "Accept: text/plain; charset=utf-8\r\n" \
        "Content-Length: %d\r\n\r\n"

/* IP Address of the remote TCP Server */
#define EXOSITE_DEMO_REMOTE_TCP_SRVR_IP     "173.255.209.28" // m2.exosite.com
#define EXOSITE_DEMO_REMOTE_TCP_SRVR_PORT   "80"


#define DefinedCIK                          "Input User-Defined-CIK Here!"

typedef enum {
    EXOSITE_ACTIVATION = 1,
    EXOSITE_WRITE,
    EXOSITE_READ
} Exosite_State;

/*-------------------------------------------------------------------------*
 * Globals:
 *-------------------------------------------------------------------------*/
static int16_t G_adc_int[2] = { 0, 0 };
static char G_temp_int[2] = { 0, 0 };

static char G_activated = 0;
static char UserCIK[41];
static char myCIK[41];
static char G_command[HOST_APP_TX_CMD_MAX_SIZE];
static char WifiMAC[17];
static Exosite_State es = EXOSITE_ACTIVATION;


    
/*-------------------------------------------------------------------------*
 * Prototypes:
 *-------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
 * Routine:  TemperatureReading
 *---------------------------------------------------------------------------*
 * Description:
 *      Take a reading of a temperature and show it on the LCD display.
 * Inputs:
 *      void
 * Outputs:
 *      void
 *---------------------------------------------------------------------------*/
void TemperatureReading(void)
{
  char lcd_buffer[20];

  // Temperature sensor reading
  int16_t temp;
  temp = Temperature_ADT7420_Get();
  // Get the temperature and show it on the LCD
  G_temp_int[0] = (int16_t)temp / 16;
  G_temp_int[1] = (int16_t)((temp & 0x000F) * 10) / 16;

  /* Display the contents of lcd_buffer onto the debug LCD */
  sprintf((char *)lcd_buffer, "TEMP: %d.%d C", G_temp_int[0], G_temp_int[1]);
  DisplayLCD(LCD_LINE3, (const uint8_t *)lcd_buffer);
}

/*---------------------------------------------------------------------------*
 * Routine:  PotentiometerReading
 *---------------------------------------------------------------------------*
 * Description:
 *      Take a reading of the potentiometer and show it on the LCD display.
 * Inputs:
 *      void
 * Outputs:
 *      void
 *---------------------------------------------------------------------------*/
void PotentiometerReading(void)
{
  char lcd_buffer[20];

  // Temperature sensor reading
  int32_t percent;
  percent = Potentiometer_Get();
  G_adc_int[0] = (int16_t)(percent / 10);
  G_adc_int[1] = (int16_t)(percent % 10);

  sprintf((char *)lcd_buffer, " POT: %d.%d %%", G_adc_int[0], G_adc_int[1]);
  /* Display the contents of lcd_buffer onto the debug LCD */
  DisplayLCD(LCD_LINE4, (const uint8_t *)lcd_buffer);
}

/*---------------------------------------------------------------------------*
 * Routine:  RSSIReading
 *---------------------------------------------------------------------------*
 * Description:
 *      Take a reading of the RSSI level with the WiFi and show it on
 *      the LCD display.
 * Inputs:
 *      void
 * Outputs:
 *      void
 *---------------------------------------------------------------------------*/
void RSSIReading(void)
{
  int16_t rssi;
  char line[20];
  int rssiFound = 0;

  if (AtLib_IsNodeAssociated()) {
    if (AtLibGs_GetRssi() == HOST_APP_MSG_ID_OK) {
      if (AtLib_ParseRssiResponse(&rssi)) {
        sprintf(line, "RSSI: %d", rssi);
        DisplayLCD(LCD_LINE5, (const uint8_t *)line);
        rssiFound = 1;
      }
    }
  }

  if (!rssiFound) {
    DisplayLCD(LCD_LINE5, "RSSI: ----");
  }
}

/*---------------------------------------------------------------------------*
 * Routine:  UpdateReadings
 *---------------------------------------------------------------------------*
 * Description:
 *      Takes a reading of temperature and potentiometer and show
 *      on the LCD.
 * Inputs:
 *      void
 * Outputs:
 *      void
 *---------------------------------------------------------------------------*/
void UpdateReadings(void)
{
    TemperatureReading();
    PotentiometerReading();
    DisplayLCD(LCD_LINE7, "");
    DisplayLCD(LCD_LINE8, "");
}

/*---------------------------------------------------------------------------*
 * Routine:  WIFI_init
 *---------------------------------------------------------------------------*
 * Description:
 *      Initial setting + DHCP and show status on the LCD.
 *
 * Inputs:
 *      void
 * Outputs:
 *      HOST_APP_MSG_ID_E
 *---------------------------------------------------------------------------*/
HOST_APP_MSG_ID_E WIFI_init(void)
{
  HOST_APP_MSG_ID_E rxMsgId = HOST_APP_MSG_ID_NONE;
  // Check the link
#ifdef HOST_APP_DEBUG_ENABLE
    ConsolePrintf("Checking link\r\n");
#endif

  AtLib_Init();
  // Wait for the banner
  MSTimerDelay(500);

  /* Send command to check */
  do {
    AtLib_FlushIncomingMessage();
    DisplayLCD(LCD_LINE8, "Checking...");
    rxMsgId = AtLibGs_Check();
  } while (HOST_APP_MSG_ID_OK != rxMsgId);

  /* Get MAC Address & Show */
  rxMsgId = AtLibGs_GetMAC();    
  if (rxMsgId == HOST_APP_MSG_ID_OK)
    AtLib_ParseGetMacResponse(WifiMAC);
  DisplayLCD(LCD_LINE5, "MAC ADDRESS");   
  DisplayLCD(LCD_LINE6, (const uint8_t *)WifiMAC);

  do {
    AtLib_FlushIncomingMessage();
    DisplayLCD(LCD_LINE8, "Disassociate");
    rxMsgId = AtLibGs_DisAssoc();
  } while (HOST_APP_MSG_ID_OK != rxMsgId);
    
    // Enable DHCP
  do { 
    DisplayLCD(LCD_LINE8, "DHCP On...");
    rxMsgId = AtLibGs_DHCPSet(1);
  } while (HOST_APP_MSG_ID_OK != rxMsgId);
    
#ifdef HOST_APP_SEC_WEP
  // Set AT+WAUTH=2 for WEP
  do {
    DisplayLCD(LCD_LINE8, " WEP AUTH " );
    rxMsgId = AtLibGs_SetWAUTH(2);
  } while (HOST_APP_MSG_ID_OK != rxMsgId);
  // Set WEP
  do {
    rxMsgId = AtLibGs_SetWEP1(HOST_APP_AP_SEC_WEP);
  } while (HOST_APP_MSG_ID_OK != rxMsgId);
    /* Security Configuration */
  do {
    rxMsgId = AtLibGs_SetSecurityType(2);        // WEP
    } while (HOST_APP_MSG_ID_OK != rxMsgId);
#endif
    
#ifdef HOST_APP_SEC_PSK
  /* Store the PSK value. This call takes might take few seconds to return */
  do {
    DisplayLCD(LCD_LINE8, "Setting PSK");
    rxMsgId = AtLibGs_CalcNStorePSK(HOST_APP_AP_SSID, HOST_APP_AP_SEC_PSK);
  } while (HOST_APP_MSG_ID_OK != rxMsgId);
#endif

#ifdef HOST_APP_SEC_OPEN
  /* Store the PSK value. This call takes might take few seconds to return */
  do {
    DisplayLCD(LCD_LINE8, "No Security" );
    rxMsgId = AtLibGs_SetWAUTH(1);
  } while (HOST_APP_MSG_ID_OK != rxMsgId);
#endif

#ifdef HOST_APP_WPA
  // Set AT+WAUTH=0 for WPA or WPA2
  do {
    DisplayLCD(LCD_LINE8, "   WPA   " );
    rxMsgId = AtLibGs_SetWAUTH(0);
  } while (HOST_APP_MSG_ID_OK != rxMsgId);

  /* Store the PSK value. This call takes might take few seconds to return */
  do {
    DisplayLCD(LCD_LINE8, "Setting PSK");
    rxMsgId = AtLibGs_CalcNStorePSK(HOST_APP_AP_SSID, HOST_APP_AP_SEC_PSK);
  } while (HOST_APP_MSG_ID_OK != rxMsgId);   

  /* Security Configuration */
  do {
    DisplayLCD(LCD_LINE8, "   WPA   ");
    rxMsgId = AtLibGs_SetSecurityType(4);
  } while (HOST_APP_MSG_ID_OK != rxMsgId);
#endif

#ifdef HOST_APP_WPA2
  // Set AT+WAUTH=0 for WPA or WPA2
  do {
    DisplayLCD(LCD_LINE8, "  WPA2   " );
    rxMsgId = AtLibGs_SetWAUTH(0);
  } while (HOST_APP_MSG_ID_OK != rxMsgId);
    
  /* Store the PSK value. This call takes might take few seconds to return */
  do {
    DisplayLCD(LCD_LINE8, "Setting PSK");
    rxMsgId = AtLibGs_CalcNStorePSK(HOST_APP_AP_SSID, HOST_APP_AP_SEC_PSK);
  } while (HOST_APP_MSG_ID_OK != rxMsgId);   

  /* Security Configuration */
  do {
    DisplayLCD(LCD_LINE8, "  Set WPA  ");
    rxMsgId = AtLibGs_SetSecurityType(8);
  } while (HOST_APP_MSG_ID_OK != rxMsgId);
#endif

  /* Clera MAC Address and show Exosite */
  DisplayLCD(LCD_LINE6, "  EXOSITE  ");
  DisplayLCD(LCD_LINE5, "           ");

  /* Enable Bulk transfer */
//  do {
//    rxMsgId = AtLibGs_BData(1);        
//  } while (HOST_APP_MSG_ID_OK != rxMsgId);  

/*
  // If need,Set the MAC Address
  do {
    DisplayLCD(LCD_LINE8, "Set MAC...");
    rxMsgId = AtLibGs_MACSet(HOST_APP_GS_NODE_MAC_ID);
  } while (HOST_APP_MSG_ID_OK != rxMsgId);

*/ 

  return rxMsgId;
}

/*---------------------------------------------------------------------------*
 * Routine:  WIFI_Associate
 *---------------------------------------------------------------------------*
 * Description:
 *      Association and show result on the LCD
 * Inputs:
 *      void
 * Outputs:
 *      HOST_APP_MSG_ID_E
 *---------------------------------------------------------------------------*/
HOST_APP_MSG_ID_E WIFI_Associate(void)
{
  HOST_APP_MSG_ID_E rxMsgId = HOST_APP_MSG_ID_NONE;

  DisplayLCD(LCD_LINE7, " Connecting ");
  /* Associate to a particular AP specified by SSID  */
  rxMsgId = AtLibGs_Assoc(HOST_APP_AP_SSID, NULL, HOST_APP_AP_CHANNEL);
  if (HOST_APP_MSG_ID_OK != rxMsgId) {
    /* Association error - we can retry */
#ifdef HOST_APP_DEBUG_ENABLE
        ConsolePrintf("\n Association error - retry now \n");
#endif
    DisplayLCD(LCD_LINE7, "** Failed **");
    MSTimerDelay(2000);
    DisplayLCD(LCD_LINE7, "");
  } else {
    /* Association success */
    AtLib_SetNodeAssociationFlag();
    DisplayLCD(LCD_LINE7, " Connected");
    MSTimerDelay(2000);
    DisplayLCD(LCD_LINE7, "");
  }

  return rxMsgId;
}

/*---------------------------------------------------------------------------*
 * Routine: Get Device CIK 
 *---------------------------------------------------------------------------*
 * Description:
 *      Read CIK from EEPROM address 0x0400
 * Inputs:
 *      void
 * Outputs:
 *      int16_t  result > 0   ==> success
 *               result <=0   ==> fail
 *---------------------------------------------------------------------------*/
int16_t GetUserCIK(void)
{
  int16_t result = 0;
  int16_t index = 0;
  result =  EEPROM_Seq_Read(5,(uint8_t*)UserCIK, 40);

  if( result > 0) {
    // verify CIK 
    while(index < 40) {
      if (((UserCIK[index] >= 0x30) && (UserCIK[index] <= 0x39)) ||
          ((UserCIK[index] >= 0x61) && (UserCIK[index] <= 0x7A)) ) {
        index++;
      } else {
        index = 40;
        result = 0;
      }
    }
  }
  
  return result; 
}

/*---------------------------------------------------------------------------*
 * Routine: Store Device CIK 
 *---------------------------------------------------------------------------*
 * Description:
 *      Write CIK at EEPROM address 0x0400
 * Inputs:
 *      void
 * Outputs:
 *      void
 *---------------------------------------------------------------------------*/
void StoreCIK(void)
{
  EEPROM_Write(5,myCIK);
}

/*---------------------------------------------------------------------------*
 * Routine: Set Device CIK 
 *---------------------------------------------------------------------------*
 * Description:
 *      Set CIK at EEPROM address 0x0400
 * Inputs:
 *      char *pcik 
 * Outputs:
 *      void
 *---------------------------------------------------------------------------*/
void SetCIK(char *pcik)
{
  EEPROM_Write(5,pcik);
}
/*---------------------------------------------------------------------------*
 * Routine: Activate device 
 *---------------------------------------------------------------------------*
 * Description:
 *      Request a CIK from Exosite Cloud to activate device
 * Inputs:
 *      void
 * Outputs:
 *      int16_t  result > 0   ==> success
 *               result <=0   ==> fail
 *---------------------------------------------------------------------------*/
void DeviceActivation(void)
{
  char content[256];
  HOST_APP_MSG_ID_E rxMsgId = HOST_APP_MSG_ID_NONE;
  //Get MAC address
  rxMsgId = AtLibGs_GetMAC();

  if (rxMsgId == HOST_APP_MSG_ID_OK)
    AtLib_ParseGetMacResponse(WifiMAC);

  if (strlen(WifiMAC) >0)
    sprintf(content, "vendor=%s&model=%s&sn=%s&osn=Micrium-Ex3&osv=3.01.2",
            "renesas", "YRDKRL78GSWIFI",WifiMAC);

  sprintf(G_command, DEVICE_ACTIVATE_ALL_LINE, strlen(content));
  strcat(G_command, content);
}

void Exosite_Write(char *pContent)
{
  sprintf(G_command, DATA_TO_SEND_TO_SERVER_ALL_LINES,myCIK, strlen(pContent));
  strcat(G_command, pContent);
}

void Exosite_Read(char *pContent)
{
  sprintf(G_command, GET_DATA_ALL_LINES, pContent,myCIK);
}

/*---------------------------------------------------------------------------*
 * Routine:  ParseGet
 *---------------------------------------------------------------------------*
 * Description:
 *      Below is a GET receive data
 *      the value next '=' is we want 
 *========================================
 *HTTP/1.1 200 OK
 *Date: Fri, 09 Mar 2012 xx:xx:xx GMT
 *Server: misultin/0.8.1-exosite
 *Connection: Keep-Alive
 *Content-Length: 10
 *
 *led_ctrl=1
 *========================================
 * Inputs:
 *      char *pData
 * Outputs:
 *      void
 *---------------------------------------------------------------------------*/
void ParseGet(char *pData)
{
  char *pGet;

  pGet = strchr(pData,'='); 
  if (pGet) {
    pGet++;
    
    if (*pGet == '1') {
      /* ON LED */
      P5  &= 0x03U;
      P6  &= 0xF3U;
    } else { 
      /* OFF LED */                
      P5  |= 0x3CU;
      P6  |= 0x0CU;
    }
  }
}

/*---------------------------------------------------------------------------*
 * Routine:  ParseCIK
 *---------------------------------------------------------------------------*
 * Description:
 *      Below is a CIK receive data
 *      the Last line 40 chars is we want 
 *========================================
 *HTTP/1.1 200 OK
 *Date: Fri, 09 Mar 2012 08:55:47 GMT
 *Server: misultin/0.8.1-exosite
 *Connection: Keep-Alive
 *Content-Length: 40
 *Content-Type: text/plain; charset=utf-8
 *
 *c0b0d02aa0228fba01f9829081987b52d1a11029
 *========================================
 * Inputs:
 *      char *pData
 * Outputs:
 *      void
 *---------------------------------------------------------------------------*/
void ParseCIK(char *pData)
{
  char *pCIK;
  
  pCIK = strstr(pData,"\r\n\r\n");
  
  if(pCIK) {
    pCIK+=4;
    
    for (uint8_t i =0; i<40; i++)
      myCIK[i]=*(pCIK+i);
                            
    myCIK[40]='\0';
    StoreCIK();
    MSTimerDelay(200);
    // Read CIK back  
    if (GetUserCIK() > 0) {
      es = EXOSITE_WRITE;
      G_activated = 1;
    }
  }
}

/*---------------------------------------------------------------------------*
 * Routine:  ParseReceiveData
 *---------------------------------------------------------------------------*
 * Description:
 *      Parse "HTTP/1.1 XXX"  
 * Inputs:
 *      char *pData
 * Outputs:
 *      void
 *---------------------------------------------------------------------------*/
void ParseReceiveData(char* pData)
{
  static uint8_t UnauthCount = 0;
  
  if (strcmp(pData,"200") == 0) {
    DisplayLCD(LCD_LINE7, " Connected! ");
    pData+= 5;
    UnauthCount = 0;
    if (!G_activated)
      ParseCIK(pData);
    else
      ParseGet(pData);
  } else if (strcmp(pData, "204") == 0) {
    // Exosite Write OK
    DisplayLCD(LCD_LINE7, " Connected!  ");
    DisplayLCD(LCD_LINE8, " Write OK! ");
    UnauthCount=0;
  } else if (strcmp(pData, "401") == 0) {
    /* Wrong CIK */
    UnauthCount++;

    if (UnauthCount > 3) {
      es = EXOSITE_ACTIVATION;
      G_activated = 0;
      UnauthCount=0;
    } 
  } else if (strcmp(pData, "409") == 0) {
      UnauthCount=5;
  } else { 
      DisplayLCD(LCD_LINE7, "             ");
      switch(es)
      {
      case EXOSITE_ACTIVATION: 
           DisplayLCD(LCD_LINE8, "  Activation ");
           break;
      case EXOSITE_WRITE: 
           DisplayLCD(LCD_LINE8, "  Write fail ");
           break;     
      case EXOSITE_READ: 
           DisplayLCD(LCD_LINE8, "   Read fail ");
           break;
      }
  }
}

/*---------------------------------------------------------------------------*
 * Routine:  App_Exosite
 *---------------------------------------------------------------------------*
 * Description:
 *      Take a reading of temperature and potentiometer and send to the
 *      Exosite Cloud using a TCP connection.
 * Inputs:
 *      void
 * Outputs:
 *      void
 *---------------------------------------------------------------------------*/
void App_Exosite(void)
{
  uint8_t cid = 0;
  uint8_t ping = 0;
  uint32_t start;
  uint32_t end;
  uint32_t postTime;
  uint32_t count;
  uint8_t RWcount = 10;
  char *pRx1,*pRx2;
#ifdef HOST_APP_TCP_DEBUG 
  static uint16_t parsererror = 0;
  static uint8_t updateError = 1;
#endif 
//SetCIK(DefinedCIK);

  HOST_APP_MSG_ID_E rxMsgId = HOST_APP_MSG_ID_NONE;
  static char content[256];
  // Give the unit a little time to start up
  // (300 ms for GS1011 and 1000 ms for GS1500)
  MSTimerDelay(1000);

  rxMsgId = WIFI_init();

  //if (rxMsgId != HOST_APP_MSG_ID_OK) 
  WIFI_Associate();
   
  if (GetUserCIK() > 0){
    strcpy(myCIK,UserCIK);
    es = EXOSITE_WRITE;
    G_activated = 1;
    RWcount = 0;
  } else {
    DisplayLCD(LCD_LINE8, " Activating");
  }

  while (1) {
    start = MSTimerGet();
    UpdateReadings();

    // Do we need to connect to the AP?
    if (!AtLib_IsNodeAssociated())
      WIFI_Associate();

    // Send data
    if (AtLib_IsNodeAssociated()) {
      RSSIReading();
      DisplayLCD(LCD_LINE8, " Sending...");

      rxMsgId = AtLibGs_TcpClientStart(EXOSITE_DEMO_REMOTE_TCP_SRVR_IP,
               EXOSITE_DEMO_REMOTE_TCP_SRVR_PORT);
      if (HOST_APP_MSG_ID_OK != rxMsgId) {
        /* TCP connection error */
#ifdef HOST_APP_DEBUG_ENABLE
        ConsolePrintf("\n TCP Connection ERROR !\n");
#endif
        AtLibGs_CloseAll();
        DisplayLCD(LCD_LINE7, "         ");
        continue;
      }

      /* Extract the CID from the response */
      MSTimerDelay(300);
      cid = AtLib_ParseTcpClientCid();
      /* Save CID value for future reference */
      AtLib_SaveTcpCid(cid);
      if (HOST_APP_INVALID_CID == cid) {
        /* TCP connection response parsing error */
#ifdef HOST_APP_DEBUG_ENABLE
        ConsolePrintf("\nTCP connection response parsing error!\n");
#endif
        // Eat the extra data and start over
        AtLib_FlushIncomingMessage();
        cid = AtLib_ParseTcpClientCid();
        
        if (HOST_APP_INVALID_CID == cid) {
#ifdef HOST_APP_TCP_DEBUG
          if (!updateError) {
            updateError = 1;
            parsererror++;
          }
#endif
          AtLibGs_CloseAll();
          DisplayLCD(LCD_LINE7, "          ");
        }
        
        continue;
      }

      App_PrepareIncomingData(cid);
      postTime = MSTimerGet();

      while (MSTimerDelta(postTime) < 5000) { // wait up to 5 seconds for a response 

        switch(es)
        {
        case EXOSITE_ACTIVATION:
             DeviceActivation();
             break;   
        case EXOSITE_WRITE:
#ifdef HOST_APP_TCP_DEBUG
             if (updateError) {
               sprintf(content, "temp=%d.%d&adc1=%d.%d&ping=%d&pecount=%d\r\n",
                       G_temp_int[0],G_temp_int[1], G_adc_int[0], G_adc_int[1],
                       ping,parsererror);
               updateError = 0;
             } else {
               sprintf(content, "temp=%d.%d&adc1=%d.%d&ping=%d\r\n",
                       G_temp_int[0],G_temp_int[1], G_adc_int[0], G_adc_int[1],
                       ping);
             }
#else
             sprintf(content, "temp=%d.%d&adc1=%d.%d&ping=%d\r\n",
                     G_temp_int[0],G_temp_int[1], G_adc_int[0], G_adc_int[1],
                     ping);
#endif
             ping++;
             if (ping >= 100)
               ping = 0;
             Exosite_Write(content);
             break;
        case EXOSITE_READ:
             sprintf(content, "led_ctrl");
             Exosite_Read(content);
             break;
        }

        /* Use bulk Transfer */
        //AtLib_BulkDataTransfer(cid, (uint8_t *)G_command, strlen(G_command));

        /* Use normal Transfer */
        AtLib_SendTcpData(cid, (uint8_t *)G_command, strlen(G_command));

        AtLib_ReceiveDataHandle();

#ifdef HOST_APP_DEBUG_ENABLE
        if (G_receivedCount > 0) {
          ConsoleSendString((const char *)G_received + 2);
          ConsoleSendString("\r\n");
        }
#endif
        /* Check Receive data */ 
        if (G_receivedCount > 17) {
          pRx1 = (char*)&G_received[11];
          pRx2 = strstr(pRx1+6,"TTP/1.1");

          G_received[2 + 8] = '\0';
          G_received[2 + 12] = '\0';

          if (strcmp((const char *)G_received + 2, "HTTP/1.1") == 0) {
            /* HTTP Result */
            ParseReceiveData(pRx1);
            
            if (pRx2){
              pRx2+=8;
              *(pRx2+3) ='\0';
              ParseReceiveData(pRx2);
            }    
          }
          break;
        }
        MSTimerDelay(500);

        if (G_activated) {
          RWcount++;
          if ( RWcount > 2)
            RWcount=0;
                
          if ( RWcount == 0)
            es = EXOSITE_WRITE;
          else
            es = EXOSITE_READ;
         }
       } // Receive while loop end

       AtLibGs_Close(cid);
       DisplayLCD(LCD_LINE7, "");
     }

     /* Wait a little bit after server disconnecting client before connecting again*/
     while (1) {
       end = MSTimerGet();
       if ((start + EXOSITE_DEMO_UPDATE_INTERVAL - end)
           < EXOSITE_DEMO_UPDATE_INTERVAL) {
         count = start + EXOSITE_DEMO_UPDATE_INTERVAL - end;
         if (count > 250){
           count = 250;
           MSTimerDelay(count);
           UpdateReadings();
         }
       } else {
         break;
       }
     }
  }
}
