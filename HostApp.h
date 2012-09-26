#ifndef HOST_APP_H_
#define HOST_APP_H_

#include <drv\SPI_CSI10.h>

#define VERSION_MAJOR       1
#define VERSION_MINOR       0
#define VERSION_TEXT        "1.01"

//#define HOST_APP_DEBUG_ENABLE  // output information on the serial port to PC
//#define HOST_APP_TCP_DEBUG   // Post TCP Connection error to Exosite

// Choose one of the following:  SPI or UART communications
// NOTE that the GainSpan module requires the correct firmware to be loaded.
#define  HOST_APP_INTERFACE_SPI      /* SPI interface is used for GS1011 S2w App communication */
//#define  HOST_APP_INTERFACE_UART   /* UART interface is used for GS1011 S2w App communication */

#define SPI_WIFI_CHANNEL    SPI_CSI10_WIFI_CHANNEL  /* Use this line if you connect to Application Header (J6) */
//#define SPI_WIFI_CHANNEL    SPI_CSI10_PMOD1_CHANNEL   /* Use this line if you connect to PMOD1 (J11) */

#define SPI_BITS_PER_SECOND         312500   // Max 857142
#define UART0_BAUD_RATE             115200   // COM Port - 115200 max
#define UART2_BAUD_RATE             9600     // WIFI Application Header UART

// Demo configuration

/* Access Point (AP) channel here. For auto channel, AP channel is NULL  */
#define HOST_APP_AP_CHANNEL          NULL

// Enable one of the securities below 
//-----------------------------------------------------------------------------
/*  Exosite demo WPA security  */
#define HOST_APP_AP_SSID              "exosite_wireless_dmz" 
#define HOST_APP_AP_SEC_PSK           "09243A7441"           
#define HOST_APP_WPA2

//-----------------------------------------------------------------------------
/* Exosite demo WEP security */
//#define  HOST_APP_SEC_WEP
//#define  HOST_APP_AP_SEC_WEP         "09243A7441"
//#define  HOST_APP_AP_SSID            "exosite_wireless_dmz"

//-----------------------------------------------------------------------------
/*  Exosite demo OPEN security  */
//#define HOST_APP_AP_SSID            "exosite_demo"
//#define HOST_APP_SEC_OPEN

//-----------------------------------------------------------------------------
/*  Exosite demo WPA2 security     */
//#define HOST_APP_AP_SSID              "exosite_demo_wpa"
//#define HOST_APP_AP_SEC_PSK           "09243A7441"
//#define HOST_APP_WPA2

#endif
