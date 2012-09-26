/*-------------------------------------------------------------------------*
 * File:  AtCmdLib.c
 *-------------------------------------------------------------------------*
 * Description:
 *      The GainSpan AT Command Library (AtCmdLib) provides the functions
 *      that send AT commands to a GainSpan node and looks for a response.
 *      Parse commands are provided to interpret the response data.
 *-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*
 * Includes:
 *-------------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h> /* for sprintf(), strstr(), strlen() , strtok() and strcpy()  */
#include <stdlib.h>
#include <ctype.h>
#include "HostApp.h"
#include "AtCmdLib.h"
#include <system/console.h>
#include <system/mstimer.h>
#include <system/platform.h>

/*-------------------------------------------------------------------------*
 * Constants:
 *-------------------------------------------------------------------------*/
#define ATLIB_RESPONSE_HANDLE_TIMEOUT   20000 /* ms */

/*-------------------------------------------------------------------------*
 * Globals:
 *-------------------------------------------------------------------------*/
/* Receive buffer to save async and response message from S2w App Node */
uint8_t MRBuffer[HOST_APP_RX_CMD_MAX_SIZE];
uint16_t MRBufferIndex = 0;

/* Command transmit buffer */
char G_ATCmdBuf[HOST_APP_TX_CMD_MAX_SIZE];

/* Flag to indicate whether S2w Node is currently associated or not */
static uint8_t nodeAssociationFlag = true;
static uint8_t nodeResetFlag = false; /* Flag to indicate whether S2w Node has rebooted after initialisation  */
static uint8_t tcpClientCid = HOST_APP_INVALID_CID; /* TCP client CID */
static uint8_t udpClientCid = HOST_APP_INVALID_CID; /* UDP client CID */

/*-------------------------------------------------------------------------*
 * Function Prototypes:
 *-------------------------------------------------------------------------*/
void AtLib_FlushRxBuffer(void);

/*---------------------------<AT command list >--------------------------------------------------------------------------
 _________________________________________________________________________________________________________________________
 AT Command                                                                   Description and AT command library API Name
 _________________________________________________________________________________________________________________________

 ATE<0|1>                                                                             Disable/enable echo
 API Name: AtLibGs_SetEcho

 AT&W<0|1>                                                                            Save Settings to profile 0/1
 API Name: AtLibGs_SaveProfile

 ATZ<0|1>                                                                             Load Settings from profile 0/1
 API Name: AtLibGs_LoadProfile

 AT+EXTPA=<1/0>                                                                       Enable/disable external PA.
 API Name: AtLibGs_EnableExternalPA

 AT+WSYNCINTRL=<interval>                                                             configure the sync loss interval in TBTT interval
 API Name: AtLibGs_SyncLossInterval

 AT+PSPOLLINTRL=<interval>                                                            configure the Ps poll interval in Seconds
 API Name: AtLibGs_PSPollInterval

 AT+BDATA=<0/1>                                                                       Bulk data reception enable/disable
 API Name: AtLibGs_BData

 AT&F                                                                                 Reset to factory defaults
 API Name:AtLibGs_ResetFactoryDefaults

 AT+WM=<0|1>                                                                          Set mode to Infrastructure (0) or ad-hoc (1)
 API Name: AtLibGs_Mode

 AT+WA=<SSID>[[,<BSSID>][,<Ch>]]                                                      Associate to network or form ad-hoc network
 API Name: AtLibGs_Assoc

 AT+WD or ATH                                                                         Disassociate from current network
 API Name:AtLibGs_DisAssoc

 AT+WRSSI=?                                                                           Query the current RSSI value
 API Name: AtLibGs_GetRssi

 AT+WWPA=passphrase                                                                   Set WPA passphrase (8 - 63 chars)
 API Name: AtLibGs_SetPassPhrase

 AT+WRETRY=n                                                                          Set the wireless retry count
 API Name: AtLibGs_SetWRetryCount

 AT+WRXACTIVE=<0|1>                                                                   Enable/disable the radio
 API Name: AtLibGs_EnableRadio

 AT+WRXPS=<0|1>                                                                       Enable/disable 802.11 power save
 API Name: AtLibGs_EnablePwSave

 AT+WP=<power>                                                                        Set the transmit power
 API Name: AtLibGs_SetTxPower

 AT+NMAC=<?>/<mac>                                                                    Get/Set MAC address and store in flash
 API Name: AtLibGs_GetMAC / AtLibGs_MACSet

 AT+NDHCP=<0|1>                                                                       Disable/Enable DHCP
 API Name: AtLibGs_DHCPSet

 AT+NSET=<IP>,<NetMask>,<Gateway>                                                     Configure network address
 API Name: AtLibGs_IPSet

 AT+NCTCP=<IP>,<Port>                                                                 Open TCP client
 API Name: AtLibGs_TcpClientStart

 AT+NCUDP=<IP>,<RemotePort>,[<LocalPort>                                              Open UDP client
 API Name: AtLibGs_UdpClientStart

 AT+NSTCP=<Port>                                                                      Open TCP server on Port
 API Name: AtLibGs_TcpServer_Start

 AT+NSUDP=<Port>                                                                      Open UDP server on Port
 API Name: AtLibGs_UdpServer_Start

 AT+NCLOSE=cid                                                                        Close the specified connection
 API Name: AtLibGs_Close

 AT+NCLOSEALL                                                                         Close all connections
 API Name: AtLibGs_CloseAll

 AT+WAUTO=<Mode>,<SSID>,<bssid>,[Channel]                                             Configure auto connect wireless settings
 API Name: AtLibGs_StoreWAutoConn

 AT+NAUTO=<Type>,<Protocol>,<DestIP>,<DestPort>                                       Configure auto connection
 Client(0)/server(1), protocol UDP(0)/TCP(1),and host.
 API Name: AtLibGs_StoreNAutoConn

 ATC<0|1>                                                                             Disable/enable auto connection
 API Name: AtLibGs_EnableAutoConnect

 +++                                                                                  Switch to Command mode while in auto-connect mode
 API Name: AtLibGs_SwitchFromAutoToCmd

 ATSparam=value                                                                       Set configuration parameters
 Network connection timeout (0) / Auto associate timeout (1)
 TCP connection timeout (2) / Association retry count (3)
 Nagle algorithm wait time (4)
 API Name: AtLibGs_StoreATS

 AT+PSDPSLEEP                                                                         Enable deep sleep
 API Name: AtLibGs_EnableDeepSleep

 AT+PSSTBY=<n>[,<delay time>,<alarm1-pol>,<alarm2-pol>]                               Standby request for n milliseconds
 API Name: AtLibGs_GotoSTNDBy

 AT+STORENWCONN                                                                       Store the nw context
 API Name: AtLibGs_StoreNwConn

 AT+RESTORENWCONN                                                                     Restore the nw context
 API Name: AtLibGs_ReStoreNwConn

 AT+FWUP=<SrvIp>,<SrvPort>,<SrcPort>                                                  Start FW Upgrade
 API Name: AtLibGs_FWUpgrade

 AT+WPAPSK=<SSID>,<PassPhrase>                                                        Calculate and store the PSK
 API Name: AtLibGs_CalcNStorePSK

 AT+NSTAT=?                                                                           Display current network context
 API Name: AtLibGs_WlanConnStat

 AT+VER=?                                                                             Get the Version Info
 API Name: AtLibGs_Version

 AT+DNSLOOKUP=<URL>,[<RETRY>,<TIMEOUT-S>]                                             Get the ip from host name
 API Name: AtLibGs_DNSLookup

 AT+DNSSET=<DNS1-IP>,[<DNS2-IP>]                                                      Set static DNS IP addresses
 API Name: AtLibGs_SetDNSServerIP

 AT+MCSTSET=<0/1>                                                                     enable/disable the multicast recv
 API Name: AtLibGs_MultiCast

 AT+BCHKSTRT=<Measure interval>                                                       Start the batt chk
 API Name: AtLibGs_BatteryChkStart


 AT+BCHKSTOP                                                                          Stop the batt chk
 API Name: AtLibGs_BatteryChkStop

 AT+ERRCOUNT=?                                                                        Get the error counts
 API Name: AtLibGs_GetErrCount

 AT+SETTIME=<dd/mm/yyyy>,<HH:MM:SS>                                                   Set the system time
 API Name: AtLibGs_SetTime

 AT+WWPS=<1/2>,<wps pin>                                                              Associate to an AP using WPS.
 1 - Push Button mathod.
 2 - PIN mathod. Provide <wps pin> only in case of PIN mathod
 API Name: AtLibGs_StartWPSPUSH / AtLibGs_StartWPSPIN


 AT&Y<0|1>                                                                            Set default power on profile to profile 0/1
 API Name: Not Available

 AT&V                                                                                 Output current configuration
 API Name: Not Available

 AT+WS[=<SSID>[,<BSSID>][,<Ch>][,<ScanTime>]]                                         Perform wireless scan
 API Name: Not Available

 AT+WRATE=?                                                                           Query the current WiFi rate used
 API Name: Not Available

 AT+WWEPn=<key>                                                                       Set WEP key (10 or 26 hex digits) with index n (1-4)
 API Name: Not Available

 AT+WAUTH=<authmode>                                                                  Set authmode (1->open,2->shared)
 API Name: AtLibGs_SetWAUTH

 AT+WSTATUS                                                                           Display current Wireless Status
 API Name: Not Available

 AT+NMAC2=<?>/<mac>                                                                   Get/Set MAC address and store in RTC
 API Name: Not Available

 AT+SETSOCKOPT=<cid>,<type>,<parameter>,<value>,<length                               Set options of a socket specified by cid
 API Name: Not Available

 ATA                                                                                  Initiate AutoConnect
 API Name: Not available

 ATA2                                                                                 Initiate AutoConnect-tcp/udp level
 API Name: Not available

 ATO                                                                                  Return to Auto Data mode
 API Name: Not available

 ATI<n>                                                                               Display identification number n
 API Name: Not available

 AT+WPSK=<PSK>                                                                        Store the PSK
 API Name: Not available

 AT+CID=?                                                                             Display The CID info
 API Name: Not available

 AT+BCHK=<?>/<Measure interval>                                                       Get/Set batt chk param
 API Name: Not available

 AT+BATTVALGET                                                                        Get the latest battery value stored in RTC
 API Name: Not available


 AT+BATTLVLSET=<Warning Level>,<warning Freq>,<Standby Level>                         Set batt warning level, frequency of reporting warning
 and batt standby levl
 API Name: Not available

 AT+PING=<Ip>,<Trails>,<Interval>,<Len>,<TOS>,<TTL>,<PAYLAOD(16 Bytes)>               Starts Ping
 API Name: Not available

 AT+TRACEROUTE=<Ip>,<Interval>,<MaxHops>,<MinHops>,<TOS>                              Starts Trace route
 API Name: Not available


 AT+GETTIME=?                                                                         Get the system time in Milli-seconds since Epoch(1970)
 API Name: Not available

 AT+DGPIO=<GPIO_PIN>,<1-SET/0-RESET>                                                  Set/reset a gpio pin
 API Name: Not available

 AT+TCERTADD=<name>,<format>,<size>,<location>\n\r<ESC>W<data of size above>          Provisions a certificate.
 format-binary/ascii(0/1),location-FLASH/RAM.
 Follow the escape sequence to send data.
 API Name: Not available

 AT+TCERTDEL=<name>                                                                   Delete a certificate
 API Name: Not available


 AT+WEAPCONF=<outer authtype>,<inner authtype>,<user name>,<password>                 Configure auth type,user name and password for EAP
 API Name: Not available

 AT+WEAP=<type>,<format>,<size>,<location>\n\r<ESC>W<data of size above>              Provision certificate for EAP TLS.
 Type-CA/CLIENT/PUB_KEY(0/1/2),
 format-binary/ascii(0/1),location- flash/RAM(0/1).
 Follow the escape sequence to send data.
 API Name: Not available

 AT+SSLOPEN=<cid>,<name>                                                              Opens a ssl connection. name-Name of certificate to use
 API Name: Not available

 AT+SSLCLOSE=<cid>                                                                    Close a SSL connection
 API Name: Not available

 AT+HTTPOPEN=<hostName/ip addr>,[<port>,<secured/non secured>,<certificate name>]     Opens a http/https connection
 API Name: Not available

 AT+HTTPCLOSE=<cid>                                                                   Closes a http connection
 API Name: Not available
 AT+HTTPSEND=<cid>,<Method>,<TimeOut>,<Page>[,<Size>]\n\r<ESC>H<data of size above>
 Send a Get or POST request.Method- GET/HEAD/POST(1/2/3)
 Follow the escape sequence to send data.
 API Name: Not available

 AT+HTTPCONF=<Param>,<Value>                                                          Configures http parameters.
 API Name: Not available

 AT+WEBPROV=<user name>,<passwd>,<ip addr><subnet mask> <gateway>                      start web server. username passwd are used for authentication
 The server is atarted with the given ip addr, subnetmask
 and gateway
 API Name: Not available

 AT+WEBPROV=<user name>,<passwd>                                                       start web server. username passwd are used for authentication
 API Name: Not available

 AT+WEBLOGOADD=<size>                                                                  add webserver logo of size <size>. After issuing
 the command, send <esc> followed by l/L   and
 send the content of the logo file
 API Name: Not available

 AT+NRAW=<0/1/2>                                                                      Enable Raw Packet transmission.
 API Name: Not available

 ATV<0|1>                                                                             Disable/enable verbose responses
 API Name: Not Available
 _________________________________________________________________________________________________________________________*/

/*---------------------------------------------------------------------------*
 * Routine:  AtLibGs_Check
 *---------------------------------------------------------------------------*
 * Description:
 *      Send command:
 *          AT
 *      and wait for response.
 * Inputs:
 *      void
 * Outputs:
 *      HOST_APP_MSG_ID_E -- response type
 *---------------------------------------------------------------------------*/
HOST_APP_MSG_ID_E AtLibGs_Check(void)
{
    HOST_APP_MSG_ID_E rxMsgId;

    /* Construct the AT command */
    sprintf(G_ATCmdBuf, "\r\nAT\r\n");

    rxMsgId = AtLib_CommandSend();

    return rxMsgId;
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLibGs_Check
 *---------------------------------------------------------------------------*
 * Description:
 *      Send command to turn on or off the character echo:
 *          ATE<0|1>
 *      and wait for response.
 * Inputs:
 *      uint8_t mode -- 1=to turn on echo, 0=turn off echo
 * Outputs:
 *      HOST_APP_MSG_ID_E -- response type
 *---------------------------------------------------------------------------*/
HOST_APP_MSG_ID_E AtLibGs_SetEcho(uint8_t mode)
{
    HOST_APP_MSG_ID_E rxMsgId;

    /* Construct the AT command */
    sprintf(G_ATCmdBuf, "ATE" _F8_ "\r\n", mode);

    /* Send command to S2w App node */
    rxMsgId = AtLib_CommandSend();

    return rxMsgId;
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLibGs_MACSet
 *---------------------------------------------------------------------------*
 * Description:
 *      Set the network MAC address for this module.
 *      Sends the command:
 *          AT+NMAC=<mac address>
 *      and waits for a response.
 *      <mac address> is the format "00:11:22:33:44:55"
 * Inputs:
 *      uint8_t mode -- 1=to turn on echo, 0=turn off echo
 * Outputs:
 *      HOST_APP_MSG_ID_E -- response type
 *---------------------------------------------------------------------------*/
HOST_APP_MSG_ID_E AtLibGs_MACSet(int8_t *pAddr)
{
    HOST_APP_MSG_ID_E rxMsgId;

    /* Construct the AT command */
    sprintf(G_ATCmdBuf, "AT+NMAC=%s\r\n", pAddr);

    /* Send command to S2w App node */
    rxMsgId = AtLib_CommandSend();

    return rxMsgId;
}
/*---------------------------------------------------------------------------*
 * Routine:  AtLibGs_ATA
 *---------------------------------------------------------------------------*
 * Description:
 *      Set the network MAC address for this module.
 *      Sends the command:
 *          ATA
 *      and waits for a response"CONNECT"
 * Inputs:
 *      
 * Outputs:
 *      HOST_APP_MSG_ID_E -- response type
 *---------------------------------------------------------------------------*/
HOST_APP_MSG_ID_E AtLibGs_ATA(void)
{
    HOST_APP_MSG_ID_E rxMsgId;

    /* Construct the AT command */
    sprintf(G_ATCmdBuf, "ATA\r\n");

    /* Send command to S2w App node */
    rxMsgId = AtLib_CommandSend();

    return rxMsgId;
}

HOST_APP_MSG_ID_E AtLibGs_ATZ1(void)
{
    HOST_APP_MSG_ID_E rxMsgId;

    /* Construct the AT command */
    sprintf(G_ATCmdBuf, "ATZ1\r\n");

    /* Send command to S2w App node */
    rxMsgId = AtLib_CommandSend();

    return rxMsgId;
}
/*---------------------------------------------------------------------------*
 * Routine:  AtLibGs_GetMAC
 *---------------------------------------------------------------------------*
 * Description:
 *      Get the network MAC address for this module.
 *      Sends the command:
 *          AT+NMAC=?
 *      and waits for a response.
 *      Then call AtLib_ParseGetMacResponse() to get the response.
 * Inputs:
 *      void
 * Outputs:
 *      HOST_APP_MSG_ID_E -- response type
 *---------------------------------------------------------------------------*/
HOST_APP_MSG_ID_E AtLibGs_GetMAC(void)
{
    HOST_APP_MSG_ID_E rxMsgId;

    /* Construct the AT command */
    sprintf(G_ATCmdBuf, "AT+NMAC=?\r\n");

    /* Send command to S2w App node */
    rxMsgId = AtLib_CommandSend();

    return rxMsgId;
}
/*---------------------------------------------------------------------------*
 * Routine:  AtLibGs_CalcNStorePSK
 *---------------------------------------------------------------------------*
 * Description:
 *      Get the network MAC address for this module.
 *      Sends the command:
 *          AT+WPAPSK=<SSID>,<PassPhrase>
 *      and waits for a response.
 * Inputs:
 *      int8_t *pSsid -- SSID (1 to 32 characters)
 *      int8_t *pPsk -- Pass phrase (8 to 63 characters)
 * Outputs:
 *      HOST_APP_MSG_ID_E -- response type
 *---------------------------------------------------------------------------*/
HOST_APP_MSG_ID_E AtLibGs_CalcNStorePSK(int8_t *pSsid, int8_t *pPsk)
{
    HOST_APP_MSG_ID_E rxMsgId;

    /* Construct the AT command */
    sprintf(G_ATCmdBuf, "AT+WPAPSK=%s,%s\r\n", pSsid, pPsk);

    /* Send command to S2w App node */
    rxMsgId = AtLib_CommandSend();

    return rxMsgId;
}
/*---------------------------------------------------------------------------*
 * Routine:  AtLibGs_WEP1
 *---------------------------------------------------------------------------*
 * Description:
 *      Set the WEP key for association
 *      and waits for a response.
 * Inputs:
 *      int8_t *pWEP -- Pass phrase (10-26 characters)
 * Outputs:
 *      HOST_APP_MSG_ID_E -- response type
 *---------------------------------------------------------------------------*/
HOST_APP_MSG_ID_E AtLibGs_SetWEP1( int8_t *pWEP)
{
    HOST_APP_MSG_ID_E rxMsgId;

    /* Construct the AT command */
    sprintf(G_ATCmdBuf, "AT+WWEP1=%s\r\n", pWEP);

    /* Send command to S2w App node */
    rxMsgId = AtLib_CommandSend();

    return rxMsgId;
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLibGs_WlanConnStat
 *---------------------------------------------------------------------------*
 * Description:
 *      Get the network MAC address for this module.
 *      Sends the command:
 *          AT+NSTAT=?
 *      and waits for a response.
 *      Use the routine AtLib_ParseNodeIpAddress() or
 *      AtLib_ParseRssiResponse() to parse the returned data for RSSI or
 *      AtLib_ParseWlanConnStat() to parse for the wireless connection status.
 * Inputs:
 *      void
 * Outputs:
 *      HOST_APP_MSG_ID_E -- response type
 *---------------------------------------------------------------------------*/
HOST_APP_MSG_ID_E AtLibGs_WlanConnStat(void)
{
    HOST_APP_MSG_ID_E rxMsgId;

    /* Construct the AT command */
    sprintf(G_ATCmdBuf, "AT+NSTAT=?\r\n");

    /* Send command to S2w App node */
    rxMsgId = AtLib_CommandSend();

    return rxMsgId;
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLibGs_SetWAUTH
 *---------------------------------------------------------------------------*
 * Description:
 *      Set authentication mode.
 *      Sends the command:
 *          AT+WAUTH=<0|1|2>
 *      and waits for a response.
 * Inputs:
 *      uint8_t mode -- 0=None, 1=Open, 2=Share with WEP 
 * Outputs:
 *      HOST_APP_MSG_ID_E -- response type
 *---------------------------------------------------------------------------*/
HOST_APP_MSG_ID_E AtLibGs_SetWAUTH(uint8_t mode)
{
    HOST_APP_MSG_ID_E rxMsgId;

    /* Construct the AT command */
    sprintf(G_ATCmdBuf, "AT+WAUTH=" _F8_ "\r\n", mode);

    /* Send command to S2w App node */
    rxMsgId = AtLib_CommandSend();

    return rxMsgId;
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLibGs_SetSecurityType
 *---------------------------------------------------------------------------*
 * Description:
 *      Set authentication mode.
 *      Sends the command:
 *          AT+WSEC=  
 *                  0       Auto
 *                  1       Open
 *                  2       Wep
 *                  4       WPA
 *                  8       WPA2
 *                  16      WPA  - Enterprise
 *                  32      WPA2 - Enterprise
 *                  64      WPA2 - acs +tkip security
 *      and waits for a response.
 * Inputs:
 *      uint8_t mode -- 0=disable DHCP, 1=enable DHCP
 * Outputs:
 *      HOST_APP_MSG_ID_E -- response type
 *---------------------------------------------------------------------------*/
HOST_APP_MSG_ID_E AtLibGs_SetSecurityType(uint8_t type)
{
    HOST_APP_MSG_ID_E rxMsgId;

    /* Construct the AT command */
    sprintf(G_ATCmdBuf, "AT+WSEC="_F8_"\r\n", type);

    /* Send command to S2w App node */
    rxMsgId = AtLib_CommandSend();

    return rxMsgId;
}
/*---------------------------------------------------------------------------*
 * Routine:  AtLibGs_DHCPSet
 *---------------------------------------------------------------------------*
 * Description:
 *      Enable or disable DHCP.
 *      Sends the command:
 *          AT+NDHCP=<0|1>
 *      and waits for a response.
 * Inputs:
 *      uint8_t mode -- 0=disable DHCP, 1=enable DHCP
 * Outputs:
 *      HOST_APP_MSG_ID_E -- response type
 *---------------------------------------------------------------------------*/
HOST_APP_MSG_ID_E AtLibGs_DHCPSet(uint8_t mode)
{
    HOST_APP_MSG_ID_E rxMsgId;

    /* Construct the AT command */
    sprintf(G_ATCmdBuf, "AT+NDHCP=" _F8_ "\r\n", mode);

    /* Send command to S2w App node */
    rxMsgId = AtLib_CommandSend();

    return rxMsgId;
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLibGs_Assoc
 *---------------------------------------------------------------------------*
 * Description:
 *      Associate to network or form ad-hoc network
 *      Sends the command:
 *          AT+WA=<SSID>[[,<BSSID>][,<Ch>]]
 *      and waits for a response.
 * Inputs:
 *      int8_t *pSsid -- SSID to connect to (1 to 32 characters)
 *      int8_t *pBssid -- Ad-hoc network id, or 0 for none
 *      int8_t *pChan -- Channel of network
 * Outputs:
 *      HOST_APP_MSG_ID_E -- response type
 *---------------------------------------------------------------------------*/
HOST_APP_MSG_ID_E AtLibGs_Assoc(int8_t *pSsid, int8_t *pBssid, int8_t *pChan)
{
    HOST_APP_MSG_ID_E rxMsgId;

    /* Construct the AT command */
    if (pChan) {
        sprintf(G_ATCmdBuf, "AT+WA=%s,%s,%s\r\n", pSsid,
                (pBssid) ? pBssid : "", pChan);
    } else {
        sprintf(G_ATCmdBuf, "AT+WA=%s\r\n", pSsid);
    }

    /* Send command to S2w App node */
    rxMsgId = AtLib_CommandSend();

    return rxMsgId;
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLibGs_TcpClientStart
 *---------------------------------------------------------------------------*
 * Description:
 *      Open TCP client connection.
 *      Sends the command:
 *          AT+NCTCP=<IP>,<Port>
 *      and waits for a response.
 *      <IP> is in the format "12:34:56:78"
 *      <Port> is the number of the port connection.
 *      Use AtLib_ParseTcpClientCid() afterward to parse the response.
 * Inputs:
 *      int8_t *pRemoteTcpSrvIp -- IP address string in format 12:34:56:78
 *      int8_t *pRemoteTcpSrvPort -- Port string
 * Outputs:
 *      HOST_APP_MSG_ID_E -- response type
 *---------------------------------------------------------------------------*/
HOST_APP_MSG_ID_E AtLibGs_TcpClientStart(
        int8_t *pRemoteTcpSrvIp,
        int8_t *pRemoteTcpSrvPort)
{
    HOST_APP_MSG_ID_E rxMsgId;

    /* Construct the AT command */
    sprintf(G_ATCmdBuf, "AT+NCTCP=%s,%s\r\n", pRemoteTcpSrvIp,
            pRemoteTcpSrvPort);

    /* Send command to S2w App node */
    rxMsgId = AtLib_CommandSend();

    return rxMsgId;
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLibGs_UdpClientStart
 *---------------------------------------------------------------------------*
 * Description:
 *      Open UDP client
 *      Sends the command:
 *          AT+NCUDP=<IP>,<RemotePort>,[<LocalPort>]
 *      and waits for a response.
 *      <IP> is in the format "12:34:56:78"
 *      <RemotePort> is the number of the port connection.
 *      <LocalPort> is the port on this machine.
 *      Use AtLib_ParseUdpClientCid() afterward to parse the response.
 * Inputs:
 *      int8_t *pRemoteUdpSrvIp -- IP address string in format 12:34:56:78
 *      int8_t *pRemoteUdpSrvPort -- Remote connection port string
 *      int8_t *pUdpLocalPort -- Local port string
 * Outputs:
 *      HOST_APP_MSG_ID_E -- response type
 *---------------------------------------------------------------------------*/
HOST_APP_MSG_ID_E AtLibGs_UdpClientStart(
        int8_t *pRemoteUdpSrvIp,
        int8_t *pRemoteUdpSrvPort,
        int8_t *pUdpLocalPort)
{
    HOST_APP_MSG_ID_E rxMsgId;

    /* Construct the AT command */
    sprintf(G_ATCmdBuf, "AT+NCUDP=%s,%s,%s\r\n", pRemoteUdpSrvIp,
            pRemoteUdpSrvPort, pUdpLocalPort);

    /* Send command to S2w App node */
    ConsolePrintf("{\r\n");
    rxMsgId = AtLib_CommandSend();
    ConsolePrintf("}\r\n");

    return rxMsgId;
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLibGs_CloseAll
 *---------------------------------------------------------------------------*
 * Description:
 *      Close all open connections.
 *      Sends the command:
 *          AT+NCLOSEALL
 *      and waits for a response.
 * Inputs:
 *      void
 * Outputs:
 *      HOST_APP_MSG_ID_E -- response type
 *---------------------------------------------------------------------------*/
HOST_APP_MSG_ID_E AtLibGs_CloseAll(void)
{
    HOST_APP_MSG_ID_E rxMsgId;

    /* Construct the AT command */
    sprintf(G_ATCmdBuf, "AT+NCLOSEALL\r\n");

    /* Send command to S2w App node */
    rxMsgId = AtLib_CommandSend();

    return rxMsgId;
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLibGs_BatteryChkStart
 *---------------------------------------------------------------------------*
 * Description:
 *      Start the batt check.
 *      Sends the command:
 *          AT+BCHKSTRT=<Measure interval>
 *      and waits for a response.
 * Inputs:
 *      uint32_t interval -- Interval of 1..100
 * Outputs:
 *      HOST_APP_MSG_ID_E -- response type
 *---------------------------------------------------------------------------*/
HOST_APP_MSG_ID_E AtLibGs_BatteryChkStart(uint32_t interval)
{
    HOST_APP_MSG_ID_E rxMsgId;

    /* Construct the AT command */
    sprintf(G_ATCmdBuf, "AT+BCHKSTRT=" _F32_ "\r\n", interval);

    /* Send command to S2w App node */
    rxMsgId = AtLib_CommandSend();

    return rxMsgId;
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLibGs_GotoSTNDBy
 *---------------------------------------------------------------------------*
 * Description:
 *      Start the batt check.
 *      Sends the command:
 *          AT+PSSTBY=x[,<DELAY TIME>,<ALARM1 POL>,<ALARM2 POL>]
 *      and waits for a response.
 * Inputs:
 *      uint32_t *nsec -- Standby time in milliseconds
 *      uint32_t dealy -- Delay time in milliseconds before going into
 *                          standby.
 *      uint32_t alarm1_Pol -- polarity of pin 31 that will trigger an alarm
 *      uint32_t alarm2_Pol -- polarity of pin 36 that will trigger an alarm
 * Outputs:
 *      HOST_APP_MSG_ID_E -- response type
 *---------------------------------------------------------------------------*/
HOST_APP_MSG_ID_E AtLibGs_GotoSTNDBy(
        int8_t *nsec,
        uint32_t dealy,
        uint32_t alarm1_Pol,
        uint32_t alarm2_Pol)
{
    HOST_APP_MSG_ID_E rxMsgId;

    /* Construct the AT command */
    sprintf(G_ATCmdBuf, "AT+PSSTBY=%s," _F32_ "," _F32_ "," _F32_ "\r\n", nsec, dealy, alarm1_Pol,
            alarm2_Pol);

    /* Send command to S2w App node */
    rxMsgId = AtLib_CommandSend();

    return rxMsgId;
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLibGs_SetPassPhrase
 *---------------------------------------------------------------------------*
 * Description:
 *      Set WPA passphrase (8 - 63 chars).
 *      Sends the command:
 *          AT+WWPA=<passphrase>
 *      and waits for a response.
 * Inputs:
 *      int8_t *pPhrase -- Passphrase string (8 - 63 characters)
 * Outputs:
 *      HOST_APP_MSG_ID_E -- response type
 *---------------------------------------------------------------------------*/
HOST_APP_MSG_ID_E AtLibGs_SetPassPhrase(int8_t *pPhrase)
{
    HOST_APP_MSG_ID_E rxMsgId;

    /* Construct the AT command */
    sprintf(G_ATCmdBuf, "AT+WWPA=%s\r\n", pPhrase);

    /* Send command to S2w App node */
    rxMsgId = AtLib_CommandSend();

    return rxMsgId;
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLibGs_SetPassPhrase
 *---------------------------------------------------------------------------*
 * Description:
 *      Enable deep sleep.
 *      Sends the command:
 *          AT+PSDPSLEEP
 *      and waits for a response.
 * Inputs:
 *      void
 * Outputs:
 *      HOST_APP_MSG_ID_E -- response type
 *---------------------------------------------------------------------------*/
HOST_APP_MSG_ID_E AtLibGs_EnableDeepSleep(void)
{
    HOST_APP_MSG_ID_E rxMsgId;

    /* Construct the AT command */
    sprintf(G_ATCmdBuf, "AT+PSDPSLEEP\n");

    /* Send command to S2w App node */
    rxMsgId = AtLib_CommandSend();

    return rxMsgId;
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLibGs_StoreNwConn
 *---------------------------------------------------------------------------*
 * Description:
 *      Store the network context.
 *      Sends the command:
 *          AT+STORENWCONN
 *      and waits for a response.
 * Inputs:
 *      void
 * Outputs:
 *      HOST_APP_MSG_ID_E -- response type
 *---------------------------------------------------------------------------*/
HOST_APP_MSG_ID_E AtLibGs_StoreNwConn(void)
{
    HOST_APP_MSG_ID_E rxMsgId;

    /* Construct the AT command */
    sprintf(G_ATCmdBuf, "AT+STORENWCONN\r\n");

    /* Send command to S2w App node */
    rxMsgId = AtLib_CommandSend();

    return rxMsgId;
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLibGs_ReStoreNwConn
 *---------------------------------------------------------------------------*
 * Description:
 *      Restore the network context.
 *      Sends the command:
 *          AT+STORENWCONN
 *      and waits for a response.
 * Inputs:
 *      void
 * Outputs:
 *      HOST_APP_MSG_ID_E -- response type
 *---------------------------------------------------------------------------*/
HOST_APP_MSG_ID_E AtLibGs_ReStoreNwConn(void)
{
    HOST_APP_MSG_ID_E rxMsgId;

    /* Construct the AT command */
    sprintf(G_ATCmdBuf, "AT+RESTORENWCONN\r\n");

    /* Send command to S2w App node */
    rxMsgId = AtLib_CommandSend();

    return rxMsgId;

}

/*---------------------------------------------------------------------------*
 * Routine:  AtLibGs_IPSet
 *---------------------------------------------------------------------------*
 * Description:
 *      Configure network IP address.
 *      Sends the command:
 *          AT+NSET=<IP>,<NetMask>,<Gateway>
 *      and waits for a response.
 *      <IP> is in the format ##:##:##:##
 *      <Netmask> is in the format ##:##:##:##
 *      <Gateway> is in the format ##:##:##:##
 * Inputs:
 *      int8_t* pIpAddr -- IP Address string
 *      int8_t* pSubnet -- Subnet mask string
 *      int8_t* pGateway -- Gateway string
 * Outputs:
 *      HOST_APP_MSG_ID_E -- response type
 *---------------------------------------------------------------------------*/
HOST_APP_MSG_ID_E AtLibGs_IPSet(
        int8_t* pIpAddr,
        int8_t* pSubnet,
        int8_t *pGateway)
{
    HOST_APP_MSG_ID_E rxMsgId;

    /* Construct the AT command */
    sprintf(G_ATCmdBuf, "AT+NSET=%s,%s,%s\r\n", pIpAddr, pSubnet, pGateway);

    /* Send command to S2w App node */
    rxMsgId = AtLib_CommandSend();

    return rxMsgId;
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLibGs_SaveProfile
 *---------------------------------------------------------------------------*
 * Description:
 *      Save Settings to profile 0 or 1.
 *      Sends the command:
 *          AT&W<0|1>
 *      and waits for a response.
 * Inputs:
 *      uint8_t profile -- profile 0 or 1
 * Outputs:
 *      HOST_APP_MSG_ID_E -- response type
 *---------------------------------------------------------------------------*/
HOST_APP_MSG_ID_E AtLibGs_SaveProfile(uint8_t profile)
{
    HOST_APP_MSG_ID_E rxMsgId;

    /* Construct the AT command */
    sprintf(G_ATCmdBuf, "AT&W" _F8_ "\r\n", profile);

    /* Send command to S2w App node */
    rxMsgId = AtLib_CommandSend();

    return rxMsgId;
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLibGs_LoadProfile
 *---------------------------------------------------------------------------*
 * Description:
 *      Load Settings from profile 0 or 1.
 *      Sends the command:
 *          ATZ<0|1>
 *      and waits for a response.
 * Inputs:
 *      uint8_t profile -- profile 0 or 1
 * Outputs:
 *      HOST_APP_MSG_ID_E -- response type
 *---------------------------------------------------------------------------*/
HOST_APP_MSG_ID_E AtLibGs_LoadProfile(uint8_t profile)
{
    HOST_APP_MSG_ID_E rxMsgId;

    /* Construct the AT command */
    sprintf(G_ATCmdBuf, "ATZ" _F8_ "\r\n", profile);

    /* Send command to S2w App node */
    rxMsgId = AtLib_CommandSend();

    return rxMsgId;
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLibGs_ResetFactoryDefaults
 *---------------------------------------------------------------------------*
 * Description:
 *      Reset factory defaults.
 *      Sends the command:
 *          AT&F
 *      and waits for a response.
 * Inputs:
 *      void
 * Outputs:
 *      HOST_APP_MSG_ID_E -- response type
 *---------------------------------------------------------------------------*/
HOST_APP_MSG_ID_E AtLibGs_ResetFactoryDefaults(void)
{
    HOST_APP_MSG_ID_E rxMsgId;

    /* Construct the AT command */
    sprintf(G_ATCmdBuf, "AT&F\r\n");

    /* Send command to S2w App node */
    rxMsgId = AtLib_CommandSend();

    return rxMsgId;
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLibGs_GetRssi
 *---------------------------------------------------------------------------*
 * Description:
 *      Query the current RSSI value.
 *      Sends the command:
 *          AT+WRSSI=?
 *      and waits for a response.
 *      Call AtLib_ParseRssiResponse() to get the final result.
 * Inputs:
 *      void
 * Outputs:
 *      HOST_APP_MSG_ID_E -- response type
 *---------------------------------------------------------------------------*/
HOST_APP_MSG_ID_E AtLibGs_GetRssi(void)
{
    HOST_APP_MSG_ID_E rxMsgId;

    /* Construct the AT command */
    sprintf(G_ATCmdBuf, "AT+WRSSI=?\r\n");

    /* Send command to S2w App node */
    rxMsgId = AtLib_CommandSend();

    return rxMsgId;
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLibGs_DisAssoc
 *---------------------------------------------------------------------------*
 * Description:
 *      Disassociate from current network.
 *      Sends the command:
 *          AT+WD
 *      and waits for a response.
 * Inputs:
 *      void
 * Outputs:
 *      HOST_APP_MSG_ID_E -- response type
 *---------------------------------------------------------------------------*/
HOST_APP_MSG_ID_E AtLibGs_DisAssoc(void)
{
    HOST_APP_MSG_ID_E rxMsgId;

    /* Construct the AT command */
    sprintf(G_ATCmdBuf, "AT+WD\r\n");

    /* Send command to S2w App node */
    rxMsgId = AtLib_CommandSend();

    return rxMsgId;
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLibGs_FWUpgrade
 *---------------------------------------------------------------------------*
 * Description:
 *      Initiate a firmware update to the given server.
 *      Sends the command:
 *          AT+FWUP=<SrvIp>,<SrvPort>,<SrcPort>
 *      and waits for a response.
 * Inputs:
 *      int8_t *pSrvip -- IP address of the firmware upgrade server string
 *      uint32_t srvport -- server port number to be used for firmware
 *                              upgrade string
 *      uint32_t srcPort -- adapter port number to be used for firmware
 *                              upgrade.
 *      int8_t *pSrcIP -- Retry is the number of times the node will repeat
 *                              the firmware upgrade attempt if failures
 *                              are encountered.
 * Outputs:
 *      HOST_APP_MSG_ID_E -- response type
 *---------------------------------------------------------------------------*/
HOST_APP_MSG_ID_E AtLibGs_FWUpgrade(
        int8_t *pSrvip,
        uint32_t srvport,
        uint32_t srcPort,
        int8_t *pSrcIP)
{
    HOST_APP_MSG_ID_E rxMsgId;

    /* Construct the AT command */
    sprintf(G_ATCmdBuf, "AT+FWUP=%s," _F32_ "," _F32_ ",%s\r\n", pSrvip, srvport, srcPort,
            pSrcIP);

    /* Send command to S2w App node */
    rxMsgId = AtLib_CommandSend();

    return rxMsgId;
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLibGs_BatteryChkStop
 *---------------------------------------------------------------------------*
 * Description:
 *      Stop the battery check.
 *      Sends the command:
 *          AT+BCHKSTOP
 *      and waits for a response.
 * Inputs:
 *      void
 * Outputs:
 *      HOST_APP_MSG_ID_E -- response type
 *---------------------------------------------------------------------------*/
HOST_APP_MSG_ID_E AtLibGs_BatteryChkStop(void)
{
    HOST_APP_MSG_ID_E rxMsgId;

    /* Construct the AT command */
    sprintf(G_ATCmdBuf, "AT+BCHKSTOP\r\n");

    /* Send command to S2w App node */
    rxMsgId = AtLib_CommandSend();

    return rxMsgId;

}

/*---------------------------------------------------------------------------*
 * Routine:  AtLibGs_MultiCast
 *---------------------------------------------------------------------------*
 * Description:
 *      Enable or disable multicast reception.
 *      Sends the command:
 *          AT+MCSTSET=<0|1>
 *      and waits for a response.
 * Inputs:
 *      uint8_t mode -- 0=disable multicast reception,
 *                      1=enable multicast reception
 * Outputs:
 *      HOST_APP_MSG_ID_E -- response type
 *---------------------------------------------------------------------------*/
HOST_APP_MSG_ID_E AtLibGs_MultiCast(uint8_t mode)
{
    HOST_APP_MSG_ID_E rxMsgId;

    /* Construct the AT command */
    sprintf(G_ATCmdBuf, "AT+MCSTSET=" _F8_ "\r\n", mode);

    /* Send command to S2w App node */
    rxMsgId = AtLib_CommandSend();

    return rxMsgId;
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLibGs_Version
 *---------------------------------------------------------------------------*
 * Description:
 *      Get the Version Info.
 *      Sends the command:
 *          AT+VER=?
 *      and waits for a response.
 * Inputs:
 *      uint8_t mode -- 0=disable multicast reception,
 *                      1=enable multicast reception
 * Outputs:
 *      HOST_APP_MSG_ID_E -- response type
 *---------------------------------------------------------------------------*/
HOST_APP_MSG_ID_E AtLibGs_Version(void)
{
    HOST_APP_MSG_ID_E rxMsgId;

    /* Construct the AT command */
    sprintf(G_ATCmdBuf, "AT+VER=?\r\n");

    /* Send command to S2w App node */
    rxMsgId = AtLib_CommandSend();

    return rxMsgId;
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLibGs_Mode
 *---------------------------------------------------------------------------*
 * Description:
 *      Set mode to Infrastructure or Ad-Hoc
 *      Sends the command:
 *          AT+WM=<0|1>
 *      and waits for a response.
 * Inputs:
 *      uint32_t mode -- 0=Infrastructure, 1=Ad Hoc
 * Outputs:
 *      HOST_APP_MSG_ID_E -- response type
 *---------------------------------------------------------------------------*/
HOST_APP_MSG_ID_E AtLibGs_Mode(uint32_t mode)
{
    HOST_APP_MSG_ID_E rxMsgId;

    /* Construct the AT command */
    sprintf(G_ATCmdBuf, "AT+WM=" _F32_ "\r\n", mode);

    /* Send command to S2w App node */
    rxMsgId = AtLib_CommandSend();

    return rxMsgId;
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLibGs_UdpServer_Start
 *---------------------------------------------------------------------------*
 * Description:
 *      Open UDP server on given port.
 *      Sends the command:
 *          AT+NSUDP=<Port>
 *      and waits for a response.
 *      The <Port> for the UDP Server to receive packets.
 *      Call AtLib_ParseUdpServerStartResponse() to parse the response
 *      into a connection id.
 * Inputs:
 *      int8_t *pUdpSrvPort -- Port on server string.
 * Outputs:
 *      HOST_APP_MSG_ID_E -- response type
 *---------------------------------------------------------------------------*/
HOST_APP_MSG_ID_E AtLibGs_UdpServer_Start(int8_t *pUdpSrvPort)
{
    HOST_APP_MSG_ID_E rxMsgId;

    /* Construct the AT command */
    sprintf(G_ATCmdBuf, "AT+NSUDP=%s\r\n", pUdpSrvPort);

    /* Send command to S2w App node */
    rxMsgId = AtLib_CommandSend();

    return rxMsgId;
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLibGs_TcpServer_Start
 *---------------------------------------------------------------------------*
 * Description:
 *      Set mode to Infrastructure or Ad-Hoc
 *      Sends the command:
 *          AT+NSTCP=<Port>
 *      and waits for a response.
 *      The <Port> for the TCP Server to receive packets.
 *      Call AtLib_ParseTcpServerStartResponse() to parse the response
 *      into a connection id.
 * Inputs:
 *      int8_t *pTcpSrvPort -- TCP port string
 * Outputs:
 *      HOST_APP_MSG_ID_E -- response type
 *---------------------------------------------------------------------------*/
HOST_APP_MSG_ID_E AtLibGs_TcpServer_Start(int8_t* pTcpSrvPort)
{
    HOST_APP_MSG_ID_E rxMsgId;

    /* Construct the AT command */
    sprintf(G_ATCmdBuf, "AT+NSTCP=%s\r\n", pTcpSrvPort);

    /* Send command to S2w App node */
    rxMsgId = AtLib_CommandSend();

    return rxMsgId;

}

/*---------------------------------------------------------------------------*
 * Routine:  AtLibGs_DNSLookup
 *---------------------------------------------------------------------------*
 * Description:
 *      Get the IP number from host name.
 *      Sends the command:
 *          AT+DNSLOOKUP=<name>
 *      and waits for a response.
 *      <name> is the URL of the address to lookup.
 *      Call AtLib_ParseDNSLookupResponse() to parse the response.
 * Inputs:
 *      int8_t *pUrl -- URL to parse.
 * Outputs:
 *      HOST_APP_MSG_ID_E -- response type
 *---------------------------------------------------------------------------*/
HOST_APP_MSG_ID_E AtLibGs_DNSLookup(int8_t* pUrl)
{
    HOST_APP_MSG_ID_E rxMsgId;

    /* Construct the AT command */
    sprintf(G_ATCmdBuf, "AT+DNSLOOKUP=%s\r\n", pUrl);

    /* Send command to S2w App node */
    rxMsgId = AtLib_CommandSend();

    return rxMsgId;
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLibGs_Close
 *---------------------------------------------------------------------------*
 * Description:
 *      Close a previously opened connection
 *      Sends the command:
 *          AT+NCLOSE=<cid>
 *      and waits for a response.
 *      <cid> is the connection id
 * Inputs:
 *      uint8_t cid -- Connection id
 * Outputs:
 *      HOST_APP_MSG_ID_E -- response type
 *---------------------------------------------------------------------------*/
HOST_APP_MSG_ID_E AtLibGs_Close(uint8_t cid)
{
    HOST_APP_MSG_ID_E rxMsgId;

    /* Construct the AT command */
    sprintf(G_ATCmdBuf, "AT+NCLOSE=%c\r\n", cid);

    /* Send command to S2w App node */
    rxMsgId = AtLib_CommandSend();

    return rxMsgId;
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLibGs_SetWRetryCount
 *---------------------------------------------------------------------------*
 * Description:
 *      Set the wireless retry count.
 *      Sends the command:
 *          AT+WRETRY=<n>
 *      and waits for a response.
 *      <n> is the number of retries (default is 5)
 * Inputs:
 *      uint8_t count -- Number of wireless retries
 * Outputs:
 *      HOST_APP_MSG_ID_E -- response type
 *---------------------------------------------------------------------------*/
HOST_APP_MSG_ID_E AtLibGs_SetWRetryCount(uint32_t count)
{
    HOST_APP_MSG_ID_E rxMsgId;

    /* Construct the AT command */
    sprintf(G_ATCmdBuf, "AT+WRETRY=" _F32_ "\r\n", count);

    /* Send command to S2w App node */
    rxMsgId = AtLib_CommandSend();

    return rxMsgId;

}

/*---------------------------------------------------------------------------*
 * Routine:  AtLibGs_GetErrCount
 *---------------------------------------------------------------------------*
 * Description:
 *      Get the error counts
 *      Sends the command:
 *          AT+ERRCOUNT=?
 *      and waits for a response.
 *      TODO: Parsing has to be done manually by looking at MRBuffer.
 * Inputs:
 *      void
 * Outputs:
 *      HOST_APP_MSG_ID_E -- response type
 *---------------------------------------------------------------------------*/
HOST_APP_MSG_ID_E AtLibGs_GetErrCount(void)
{
    HOST_APP_MSG_ID_E rxMsgId;

    /* Construct the AT command */
    sprintf(G_ATCmdBuf, "AT+ERRCOUNT=?\r\n");

    /* Send command to S2w App node */
    rxMsgId = AtLib_CommandSend();

    return rxMsgId;
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLibGs_EnableRadio
 *---------------------------------------------------------------------------*
 * Description:
 *      Enable or disable the radio.
 *      Sends the command:
 *          AT+WRXACTIVE=<mode>
 *      and waits for a response.
 *      <mode> is 0 when disabling the radio, 1 when enabled the radio.
 * Inputs:
 *      uint8_t mode -- 0 to disable the radio, 1 to enable the radio
 * Outputs:
 *      HOST_APP_MSG_ID_E -- response type
 *---------------------------------------------------------------------------*/
HOST_APP_MSG_ID_E AtLibGs_EnableRadio(uint8_t mode)
{
    HOST_APP_MSG_ID_E rxMsgId;

    /* Construct the AT command */
    sprintf(G_ATCmdBuf, "AT+WRXACTIVE=" _F8_ "\r\n", mode);

    /* Send command to S2w App node */
    rxMsgId = AtLib_CommandSend();

    return rxMsgId;
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLibGs_EnablePwSave
 *---------------------------------------------------------------------------*
 * Description:
 *      Enable or disable the 802.11 power save mode.
 *      Sends the command:
 *          AT+WRXPS=<mode>
 *      and waits for a response.
 *      <mode> is 0 to disable power save mode, 1 to enable power save.
 *          The default is enabled.
 * Inputs:
 *      uint8_t mode -- 0 to disabe power save mode, 1 to enable
 * Outputs:
 *      HOST_APP_MSG_ID_E -- response type
 *---------------------------------------------------------------------------*/
HOST_APP_MSG_ID_E AtLibGs_EnablePwSave(uint8_t mode)
{
    HOST_APP_MSG_ID_E rxMsgId;

    /* Construct the AT command */
    sprintf(G_ATCmdBuf, "AT+WRXPS=" _F8_ "\r\n", mode);

    /* Send command to S2w App node */
    rxMsgId = AtLib_CommandSend();

    return rxMsgId;
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLibGs_SetTime
 *---------------------------------------------------------------------------*
 * Description:
 *      Set the system time.
 *      Sends the command:
 *          AT+SETTIME=<date>,<time>
 *      and waits for a response.
 *      <date> is in the format "dd/mm/yyyy"
 *      <time> is in the format "HH:MM:SS"
 * Inputs:
 *      int8_t* pDate -- Date string in format "dd/mm/yyyy"
 *      int8_t* pTime -- Time string in format "HH:MM:SS"
 * Outputs:
 *      HOST_APP_MSG_ID_E -- response type
 *---------------------------------------------------------------------------*/
HOST_APP_MSG_ID_E AtLibGs_SetTime(int8_t* pDate, int8_t *pTime)
{
    HOST_APP_MSG_ID_E rxMsgId;

    /* Construct the AT command */
    sprintf(G_ATCmdBuf, "AT+SETTIME=%s,%s\r\n", pDate, pTime);

    /* Send command to S2w App node */
    rxMsgId = AtLib_CommandSend();

    return rxMsgId;
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLibGs_EnableExternalPA
 *---------------------------------------------------------------------------*
 * Description:
 *      This command enables or disables the external PA (patch antenna).
 *      Sends the command:
 *          AT+EXTPA=<mode>
 *      and waits for a response.
 *      <mode> is 1 to enable the external PA, or 0 to disable external PA.
 *      NOTE: If enabled, this command forces the adapter to standby and
 *          comes back immediately causing all configured parameters and
 *          network connections to be lost.
 * Inputs:
 *      void
 * Outputs:
 *      HOST_APP_MSG_ID_E -- response type
 *---------------------------------------------------------------------------*/
HOST_APP_MSG_ID_E AtLibGs_EnableExternalPA(uint8_t mode)
{
    HOST_APP_MSG_ID_E rxMsgId;

    /* Construct the AT command */
    sprintf(G_ATCmdBuf, "AT+EXTPA=" _F8_ "\r\n", mode);

    /* Send command to S2w App node */
    rxMsgId = AtLib_CommandSend();

    return rxMsgId;
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLibGs_SyncLossInterval
 *---------------------------------------------------------------------------*
 * Description:
 *      Configure the sync loss interval in TBTT interval.
 *      Sends the command:
 *          AT+WSYNCINTRL=<interval>
 *      and waits for a response.
 *      <interval> is the sync loss interval with a range of 1 to 65535.
 *          Default value is 30.
 * Inputs:
 *      void
 * Outputs:
 *      HOST_APP_MSG_ID_E -- response type
 *---------------------------------------------------------------------------*/
HOST_APP_MSG_ID_E AtLibGs_SyncLossInterval(uint16_t interval)
{
    HOST_APP_MSG_ID_E rxMsgId;

    /* Construct the AT command */
    sprintf(G_ATCmdBuf, "AT+WSYNCINTRL=" _F16_ "\r\n", interval);

    /* Send command to S2w App node */
    rxMsgId = AtLib_CommandSend();

    return rxMsgId;
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLibGs_PSPollInterval
 *---------------------------------------------------------------------------*
 * Description:
 *      Configure the Association Keep Alive Timer (PS) poll interval in
 *      seconds.
 *      Sends the command:
 *          AT+PSPOLLINTRL=<interval>
 *      and waits for a response.
 *      <interval> is the number of seconds to keep alive.  Default is 45.
 *          Range is 0 to 65535 seconds.  A value of 0 disables.
 * Inputs:
 *      uint16_t interval
 * Outputs:
 *      HOST_APP_MSG_ID_E -- response type
 *---------------------------------------------------------------------------*/
HOST_APP_MSG_ID_E AtLibGs_PSPollInterval(uint16_t interval)
{
    HOST_APP_MSG_ID_E rxMsgId;

    /* Construct the AT command */
    sprintf(G_ATCmdBuf, "AT+PSPOLLINTRL=" _F16_ "\r\n", interval);

    /* Send command to S2w App node */
    rxMsgId = AtLib_CommandSend();

    return rxMsgId;
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLibGs_SetTxPower
 *---------------------------------------------------------------------------*
 * Description:
 *      Set the transmit power.
 *      Sends the command:
 *          AT+WP=<power>
 *      and waits for a response.
 *      <power> is the transmit power.
 *      Transmit power can be 0 to 7 for internal PA GS101x with a default of
 *      0.  Transmit power can be 2 to 15 for external PA GS101x with a
 *      default power of 2.
 * Inputs:
 *      uint32_t power -- Power level (see above notes)
 * Outputs:
 *      HOST_APP_MSG_ID_E -- response type
 *---------------------------------------------------------------------------*/
HOST_APP_MSG_ID_E AtLibGs_SetTxPower(uint32_t power)
{
    HOST_APP_MSG_ID_E rxMsgId;

    /* Construct the AT command */
    sprintf(G_ATCmdBuf, "AT+WP=" _F32_ "\r\n", power);

    /* Send command to S2w App node */
    rxMsgId = AtLib_CommandSend();

    return rxMsgId;
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLibGs_SetDNSServerIP
 *---------------------------------------------------------------------------*
 * Description:
 *      Set static DNS IP address(es).
 *      Sends the command:
 *          AT+WSYNCINTRL=<dns1>[,<dns2>]
 *      and waits for a response.
 *      <dns1> is the primary DNS address in "##.##.##.##" format.
 *      <dns2> is the optional secondary DNS address in "##.##.##.##" format.
 * Inputs:
 *      int8_t *pDNS1 -- Primary DNS address string in "##.##.##.##" format.
 *      int8_t *pDNS2 -- Secondary DNS address string in "##.##.##.##" format.
 *          If NULL, no secondary DNS address is used.
 * Outputs:
 *      HOST_APP_MSG_ID_E -- response type
 *---------------------------------------------------------------------------*/
HOST_APP_MSG_ID_E AtLibGs_SetDNSServerIP(int8_t *pDNS1, int8_t *pDNS2)
{
    HOST_APP_MSG_ID_E rxMsgId;

    /* Construct the AT command */
    if (pDNS2 == NULL) {
        sprintf(G_ATCmdBuf, "AT+DNSSET=%s", pDNS1);
    } else {
        sprintf(G_ATCmdBuf, "AT+DNSSET=%s,%s", pDNS1, pDNS2);
    }

    /* Send command to S2w App node */
    rxMsgId = AtLib_CommandSend();

    return rxMsgId;

}

/*---------------------------------------------------------------------------*
 * Routine:  AtLibGs_EnableAutoConnect
 *---------------------------------------------------------------------------*
 * Description:
 *      Disable or enable auto connection.
 *      Sends the command:
 *          AT+ATC<mode>
 *      and waits for a response.
 *      NOTE: The resulting change only takes effect on the next reboot or
 *          an ATA command.
 * Inputs:
 *      uint8_t mode -- 0 to disable auto connect, 1 to enable auto connect.
 *          Default is disabled.
 * Outputs:
 *      HOST_APP_MSG_ID_E -- response type
 *---------------------------------------------------------------------------*/
HOST_APP_MSG_ID_E AtLibGs_EnableAutoConnect(uint8_t mode)
{
    HOST_APP_MSG_ID_E rxMsgId;

    /* Construct the AT command */
    sprintf(G_ATCmdBuf, "ATC" _F8_ "\r\n", mode);

    /* Send command to S2w App node */
    rxMsgId = AtLib_CommandSend();

    return rxMsgId;
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLibGs_SwitchFromAutoToCmd
 *---------------------------------------------------------------------------*
 * Description:
 *
 *      Sends the command:
 *          AT+WSYNCINTRL=<interval>
 *      and waits for a response.
 * Inputs:
 *      void
 * Outputs:
 *      HOST_APP_MSG_ID_E -- response type
 *---------------------------------------------------------------------------*/
void AtLibGs_SwitchFromAutoToCmd(void)
{
    uint32_t command_length;

    sprintf(G_ATCmdBuf, "+++");
    /* Get the command length */
    command_length = strlen((const char *)G_ATCmdBuf);

    App_Write((uint8_t *)&G_ATCmdBuf[0], command_length);

    MSTimerDelay(1000);

    /* Construct the AT command */
    sprintf(G_ATCmdBuf, "\r\n");
    command_length = strlen((const char *)G_ATCmdBuf);

    App_Write((uint8_t *)&G_ATCmdBuf[0], strlen(G_ATCmdBuf));
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLibGs_
 *---------------------------------------------------------------------------*
 * Description:
 *
 *      Sends the command:
 *          AT+WSYNCINTRL=<interval>
 *      and waits for a response.
 * Inputs:
 *      void
 * Outputs:
 *      HOST_APP_MSG_ID_E -- response type
 *---------------------------------------------------------------------------*/
HOST_APP_MSG_ID_E AtLibGs_StoreWAutoConn(int8_t * pSsid, uint8_t channel)
{
    HOST_APP_MSG_ID_E rxMsgId;

    /* Construct the AT command */
    sprintf(G_ATCmdBuf, "AT+WAUTO=0,%s,," _F8_ "\r\n", pSsid, channel);

    /* Send command to S2w App node */
    rxMsgId = AtLib_CommandSend();

    return rxMsgId;
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLibGs_StoreNAutoConn
 *---------------------------------------------------------------------------*
 * Description:
 *      Configure auto connection network parameters (for client connection).
 *      Sends the command:
 *          AT+NAUTO=0,0,<ip addr>,<port>
 *      and waits for a response.
 *      <ip addr> is the IP address of the remote server
 *      <port> is the port address of the remote server
 * Inputs:
 *      int8_t *pIpAddr -- IP address string in format "##.##.##.##"
 *      int16_t pRmtPort -- Port address of remote system
 * Outputs:
 *      HOST_APP_MSG_ID_E -- response type
 *---------------------------------------------------------------------------*/
HOST_APP_MSG_ID_E AtLibGs_StoreNAutoConn(int8_t* pIpAddr, int16_t pRmtPort)
{
    HOST_APP_MSG_ID_E rxMsgId;

    /* Construct the AT command */
    sprintf(G_ATCmdBuf, "AT+NAUTO=0,0,%s," _F16_ "\r\n", pIpAddr, pRmtPort);

    /* Send command to S2w App node */
    rxMsgId = AtLib_CommandSend();

    return rxMsgId;
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLibGs_StoreATS
 *---------------------------------------------------------------------------*
 * Description:
 *      Store network and configuration parameters.  See documentation
 *      for configuration parameters.
 *          0=Network Connection Timeout
 *              Maximum time to establish auto connection in 10 ms
 *              intervals.  Default is 1000 or 10 seconds.
 *          1=Auto Associate Timeout
 *              Maximum time to associate to desired wireless network in
 *              10 ms intervals.  Default is 1000 or 10 seconds.
 *          2=TCP Connection Timeout
 *              Maximum time to establish TCP client connection in
 *              10 ms intervals.  Default is 500 or 5 seconds.
 *          3=Association Retry Count
 *              Not currently supported
 *          4=Nagle Algorithm Wait Time
 *              Maximum time for serial data sent in Auto Connect Mode
 *              to be buffered in 10 ms intervals.  Default is 10 or 100 ms.
 *          5=Scan Time
 *              Maximum time for scanning in one radio chanenel in
 *              milliseconds.  Default is 20 or 20 ms.
 *      Sends the command:
 *          AT+ATS<param>=<time>
 *      and waits for a response.
 *      <param> is the numeric parameter from the above.
 *      <time> is the value to be used for the given <param> (see above).
 * Inputs:
 *      uint8_t param -- Timing parameter to set
 *      uint8_t value -- Timing value to use
 * Outputs:
 *      HOST_APP_MSG_ID_E -- response type
 *---------------------------------------------------------------------------*/
HOST_APP_MSG_ID_E AtLibGs_StoreATS(uint8_t param, uint8_t value)
{
    HOST_APP_MSG_ID_E rxMsgId;

    /* Construct the AT command */
    sprintf(G_ATCmdBuf, "ATS" _F8_ "=" _F8_ "\r\n", param, value);

    /* Send command to S2w App node */
    rxMsgId = AtLib_CommandSend();

    return rxMsgId;
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLibGs_BData
 *---------------------------------------------------------------------------*
 * Description:
 *      Enable or disable bulk data reception.
 *      Sends the command:
 *          AT+BDATA=<mode>
 *      and waits for a response.
 *      <mode> is 1 to enable, or 0 to disable.
 * Inputs:
 *      uint8_t mode -- 0 to disable bulk data reception, or 1 to enable.
 * Outputs:
 *      HOST_APP_MSG_ID_E -- response type
 *---------------------------------------------------------------------------*/
HOST_APP_MSG_ID_E AtLibGs_BData(uint8_t mode)
{
    HOST_APP_MSG_ID_E rxMsgId;

    /* Construct the AT command */
    sprintf(G_ATCmdBuf, "AT+BDATA=" _F8_ "\r\n", mode);

    /* Send command to S2w App node */
    rxMsgId = AtLib_CommandSend();

    return rxMsgId;
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLibGs_StartWPSPUSH
 *---------------------------------------------------------------------------*
 * Description:
 *      Associate to an access point (AP) using Wi-Fi Protected Setup (WPS)
 *      using the push button method.
 *      Sends the command:
 *          AT+WWPS=1
 *      and waits for a response.
 *      Use AtLib_ParseWPSPUSHResponse() immediately afterward to get the
 *      response.
 * Inputs:
 *      void
 * Outputs:
 *      HOST_APP_MSG_ID_E -- response type
 *---------------------------------------------------------------------------*/
HOST_APP_MSG_ID_E AtLibGs_StartWPSPUSH(void)
{
    HOST_APP_MSG_ID_E rxMsgId;

    /* Construct the AT command */
    sprintf(G_ATCmdBuf, "AT+WWPS=1");

    /* Send command to S2w App node */
    rxMsgId = AtLib_CommandSend();

    return rxMsgId;
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLibGs_StartWPSPIN
 *---------------------------------------------------------------------------*
 * Description:
 *      Associate to an access point (AP) using Wi-Fi Protected Setup (WPS)
 *          using the Personal Identification Number (PIN) method.
 *      Sends the command:
 *          AT+WWPS=2,<pin>
 *      and waits for a response.
 *      <pin> is a unique PIN.
 *      Use AtLib_ParseWPSPINResponse() to parse the response.
 * Inputs:
 *      int8_t* pin -- PIN string to pass in when doing WPS
 * Outputs:
 *      HOST_APP_MSG_ID_E -- response type
 *---------------------------------------------------------------------------*/
HOST_APP_MSG_ID_E AtLibGs_StartWPSPIN(int8_t* pin)
{
    HOST_APP_MSG_ID_E rxMsgId;

    /* Construct the AT command */
    sprintf(G_ATCmdBuf, "AT+WWPS=2,%s", pin);

    /* Send command to S2w App node */
    rxMsgId = AtLib_CommandSend();

    return rxMsgId;
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLib_ParseTcpClientCid
 *---------------------------------------------------------------------------*
 * Description:
 *      Parses the last line returned after connecting with TCPIP.
 *      If a CONNECT <id> is returned, returns <id>.  Otherwise, responds
 *      with HOST_APP_INVALID_CID.
 * Inputs:
 *      void
 * Outputs:
 *      uint8_t -- Returned connection id or HOST_APP_INVALID_CID.
 *---------------------------------------------------------------------------*/
uint8_t AtLib_ParseTcpClientCid(void)
{
    uint8_t cid;
    uint8_t *result = NULL;

    if ((result = (uint8_t *)strstr((const char *)MRBuffer, "CONNECT")) != NULL) {
        /* Succesfull connection done for TCP client */
        cid = result[HOST_APP_TCP_CLIENT_CID_OFFSET_BYTE];
    } else {
        /* Not able to extract the CID */
        cid = HOST_APP_INVALID_CID;
    }

    return cid;
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLib_ParseUdpClientCid
 *---------------------------------------------------------------------------*
 * Description:
 *      Parses the last line returned for a UDP Client connection.
 *      If a CONNECT <id> is returned, return <id>.  Otherwise, respond with
 *      HOST_APP_INVALID_CID.
 * Inputs:
 *      void
 * Outputs:
 *      uint8_t -- Returned connection id or HOST_APP_INVALID_CID.
 *---------------------------------------------------------------------------*/
uint8_t AtLib_ParseUdpClientCid(void)
{
    uint8_t cid;
    uint8_t *result = NULL;

    if ((result = (uint8_t *)strstr((const char *)MRBuffer, "CONNECT")) != NULL) {
        /* Succesfull connection done for UDP client */
        cid = result[HOST_APP_UDP_CLIENT_CID_OFFSET_BYTE];
    } else {
        /* Not able to extract the CID */
        cid = HOST_APP_INVALID_CID;
    }

    return cid;
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLib_ParseWlanConnStat
 *---------------------------------------------------------------------------*
 * Description:
 *      Parses the last line returned after doing a AtLibGs_WlanConnStat()
 *      command and determines if there is a wireless association.
 * Inputs:
 *      void
 * Outputs:
 *      uint8_t -- Returns true if associated, else false.
 *---------------------------------------------------------------------------*/
uint8_t AtLib_ParseWlanConnStat(void)
{
    char *pSubStr;

    /* Check whether response message contains the following */
    /* string "BSSID=00:00:00:00:00:00" */
    pSubStr = strstr((const char *)MRBuffer, "BSSID=00:00:00:00:00:00");

    if (pSubStr) {
        /* Not been associated */
        return false;
    } else {
        /* Already associated */
        return true;
    }
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLib_ParseGetMacResponse
 *---------------------------------------------------------------------------*
 * Description:
 *      Parses the last line returned after doing a AtLibGs_GetMAC()
 *      command.  The passed in MAC Id is compared to the returned MAC
 *      address.  If the returned MAC address is "00:00:00:00:00:00" or
 *      the MAC address does not match the passed in MAC address, false
 *      is returned.  If the MAC address returned matches the passed in
 *      MAC address, true is returned.
 * Inputs:
 *      int8_t *pRefNodeMacId -- MAC address to compare to
 * Outputs:
 *      uint8_t -- Returns true if matching MAC address and not 0's,
 *          else false.
 *---------------------------------------------------------------------------*/
uint8_t AtLib_ParseGetMacResponse(char *pMAC) //int8_t *pRefNodeMacId)
{
    char *pSubStr;
    char currNodeMac[20] = "00:00:00:00:00:00";

    /* Set the node MAC address, If the MAC is already set, */
    /* then skip the flash write operation */
    pSubStr = strstr((const char *)MRBuffer, ":");

    if (pSubStr) {
        /* Setup a copy to compare with */
        memcpy(currNodeMac, (pSubStr - 2), 17);
        currNodeMac[17] = '\0';
        
        pSubStr = &currNodeMac[0];
        while (*pSubStr != '\0')
        {
          if (*pSubStr != ':')
          {
               *pMAC = *pSubStr;
               pMAC++;
          }
          pSubStr++;
        }
          
        return true;

    } else {
        /* Failed to get MAC address information */
        return false;
    }
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLib_ParseNodeIpAddress
 *---------------------------------------------------------------------------*
 * Description:
 *      Parses the last line returned after doing a AtLibGs_WlanConnStat()
 *      command.  The passed in IP number is compared to the returned IP
 *      number (if any), and if it matches, 1 is returned.  Otherwise,
 *      returns 1.
 * Inputs:
 *      int8_t *pIpAddr -- Passed in IP address to compare
 * Outputs:
 *      uint8_t -- Returns 0 if invalid IP address, 1 if valid ip address and
 *          matches the passed in IP number.
 *---------------------------------------------------------------------------*/
uint8_t AtLib_ParseNodeIpAddress(int8_t *pIpAddr)
{
    char *pSubStr;

    pSubStr = strstr((const char *)MRBuffer, "IP addr=");
    if (pSubStr) {
        strcpy((char *)pIpAddr, strtok((pSubStr + 8), ": "));
        if (pIpAddr[0] == '0') {
            pIpAddr = NULL;

            /* Failed */
            return 0;
        }
    } else {
        /* Failed */
        return 0;
    }

    /* Success */
    return 1;

}

/*---------------------------------------------------------------------------*
 * Routine:  AtLib_ParseRssiResponse
 *---------------------------------------------------------------------------*
 * Description:
 *      Parses the last line returned after doing a AtLibGs_WlanConnStat()
 *      command.  The RSSI value is returned by reference.
 * Inputs:
 *      int16_t *pRSSI -- Returned passed in value
 * Outputs:
 *      uint8_t -- Returns 1 if RSSI value found and valid, else 0.
 *---------------------------------------------------------------------------*/
uint8_t AtLib_ParseRssiResponse(int16_t *pRssi)
{
    char *pSubStr;

    if ((pSubStr = strstr((const char *)MRBuffer, "-")) != NULL) {
        *pRssi = atoi((pSubStr));

        return 1;
    } else {
        return 0;
    }
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLib_ParseUdpServerStartResponse
 *---------------------------------------------------------------------------*
 * Description:
 *      Parses the last line returned after doing a AtLibGs_UdpServer_Start()
 *      command.  If connected, parses the UDP server connection id.
 *      Otherwise, no connection is returned.
 * Inputs:
 *      int16_t *pConnId -- Returned connection id (if any)
 * Outputs:
 *      uint8_t -- Returns 1 if connected, else 0.
 *---------------------------------------------------------------------------*/
uint8_t AtLib_ParseUdpServerStartResponse(uint8_t *pConnId)
{
    char * result = NULL;

    if ((result = strstr((const char *)MRBuffer, "CONNECT")) != NULL) {
        *pConnId = result[8];

        /* Success */
        return 1;
    } else if (((result = strstr((const char *)MRBuffer, "DISASSOCIATED"))
            !=NULL) || (strstr((const char *)MRBuffer, "SOCKET FAILURE")
            != NULL)) {
        /* Failed  */
        return 0;
    } else {
        /* Failed  */
        return 0;
    }
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLib_ParseUdpServerStartResponse
 *---------------------------------------------------------------------------*
 * Description:
 *      Parses the last line returned after doing a AtLibGs_TcpServer_Start()
 *      command.  If connected, parses the TCP server connection id.
 *      Otherwise, no connection is returned.
 * Inputs:
 *      int16_t *pConnId -- Returned connection id (if any)
 * Outputs:
 *      uint8_t -- Returns 1 if connected, else 0.
 *---------------------------------------------------------------------------*/
uint8_t AtLib_ParseTcpServerStartResponse(uint8_t *pConnId)
{
    char* pSubStr = NULL;

    if ((pSubStr = strstr((const char *)MRBuffer, "CONNECT")) != NULL) {
        *pConnId = pSubStr[8];
        return 1;/* Success */
    } else if (((strstr((const char *)MRBuffer, "DISASSOCIATED")) != NULL)
            || (strstr((const char *)MRBuffer, "SOCKET FAILURE") != NULL)) {
        return 0; /* Failed  */
    } else {
        return 0; /* Failed  */
    }
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLib_ParseDNSLookupResponse
 *---------------------------------------------------------------------------*
 * Description:
 *      Parses the last line returned after doing a AtLibGs_DNSLookup()
 *      command.  If connected, parses the TCP server connection id.
 *      Otherwise, no connection is returned.
 * Inputs:
 *      int16_t *pConnId -- Returned connection id (if any)
 * Outputs:
 *      uint8_t -- Returns 1 if connected, else 0.
 *---------------------------------------------------------------------------*/
uint8_t AtLib_ParseDNSLookupResponse(char *ipAddr)
{
    char *pSubStr = NULL;

    pSubStr = strstr((const char *)MRBuffer, "IP:");
    if (pSubStr) {
        strcpy(ipAddr, (pSubStr + 3));

        return 1; /* Success */
    } else {
        return 0; /* Failed  */
    }
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLib_ParseWPSPUSHResponse
 *---------------------------------------------------------------------------*
 * Description:
 *      Parses the last line returned after doing a AtLibGs_StartWPSPUSH()
 *      command.  If the WPS worked (and an SSID is reported), 1 is
 *      returned.  If fails, it returns 0.
 * Inputs:
 *      void
 * Outputs:
 *      uint8_t -- Returns 1 if WPS is successful, else 0.
 *---------------------------------------------------------------------------*/
uint8_t AtLib_ParseWPSPUSHResponse(void)
{
    if (strstr((const char *)MRBuffer, "SSID=") != NULL) {
        return 1; /* Success */
    } else {
        return 0; /* Failed  */
    }
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLib_ParseWPSPINResponse
 *---------------------------------------------------------------------------*
 * Description:
 *      Parses the last line returned after doing a AtLibGs_StartWPSPIN()
 *      command.  If the WPS worked (and an SSID is reported), 1 is
 *      returned.  If fails, it returns 0.
 * Inputs:
 *      void
 * Outputs:
 *      uint8_t -- Returns 1 if WPS is successful, else 0.
 *---------------------------------------------------------------------------*/
uint8_t AtLib_ParseWPSPINResponse(uint32_t pin)
{
    if (strstr((const char *)MRBuffer, "SSID=") != NULL) {
        return 1; /* Success */
    } else {
        return 0; /* Failed  */
    }
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLib_CommandSend
 *---------------------------------------------------------------------------*
 * Description:
 *      Sends an AT command to the module and waits for a response.  If
 *      data is returned, it is collected into MRBuffer.
 * Inputs:
 *      void
 * Outputs:
 *      HOST_APP_MSG_ID_E -- response type
 *---------------------------------------------------------------------------*/
HOST_APP_MSG_ID_E AtLib_CommandSend(void)
{
    HOST_APP_MSG_ID_E rxMsgId;

#ifdef HOST_APP_DEBUG_ENABLE
    ConsolePrintf(">%s\n", G_ATCmdBuf);
#endif    

    /* Now send the command to S2w App node */
    App_Write((uint8_t *)&G_ATCmdBuf[0], strlen((const char *)G_ATCmdBuf));

    /* Wait for the response while collecting data into the MRBuffer */
    rxMsgId = AtLib_ResponseHandle();

    return rxMsgId;
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLib_DataSend
 *---------------------------------------------------------------------------*
 * Description:
 *      Send data to the module.
 * Inputs:
 *      const uint8_t *pTxData -- Data to send
 *      uint32_t dataLen -- Length of data to send
 * Outputs:
 *      void
 *---------------------------------------------------------------------------*/
void AtLib_DataSend(const uint8_t *pTxData, uint32_t dataLen)
{
    App_Write(pTxData, dataLen);
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLib_SendTcpData
 *---------------------------------------------------------------------------*
 * Description:
 *      Send data to the given TCP connection.  The data is converted into
 *      the following byte format:
 *          <ESC><'S'><cid><N bytes><ESC><'E'>
 *      <ESC> is the escape character 0x1B
 *      <'S'> is the letter 'S'
 *      <cid> is the connection ID.
 *      <N bytes> is a number of bytes
 *      <'E'> is the letter 'E'
 * Inputs:
 *      uint8_t cid -- Connection ID
 *      const uint8_t *pTxData -- Data to send to the TCP connection
 *      uint32_t dataLen -- Length of data to send
 * Outputs:
 *      void
 *---------------------------------------------------------------------------*/
void AtLib_SendTcpData(uint8_t cid, const uint8_t *txBuf, uint32_t dataLen)
{
  if (HOST_APP_INVALID_CID != cid) {
        /* Construct the data start indication message */
        sprintf(G_ATCmdBuf, "%c%c%c", HOST_APP_ESC_CHAR, 'S', cid);

        /* Now send the data START indication message  to S2w node */
        App_Write((uint8_t *)&G_ATCmdBuf[0], 3);

        /* Now send the actual data */
        AtLib_DataSend(txBuf, dataLen);

        /* Construct the data start indication message */
        sprintf(G_ATCmdBuf, "%c%c", HOST_APP_ESC_CHAR, 'E');

        /* Now send the data END indication message  to S2w node */
        App_Write((uint8_t *)&G_ATCmdBuf[0], 2);
    }
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLib_SendUdpData
 *---------------------------------------------------------------------------*
 * Description:
 *      Send data to the given UDP connection.  The data is converted into
 *      the following byte format when UDP client:
 *          <ESC><'S'><cid><N bytes><ESC><'E'>
 *      When UDP server, transmit in the following format:
 *          <ESC><'U'><cid><ip><port><N bytes><ESC><'E'>
 *      <ESC> is the escape character 0x1B
 *      <'S'> is the letter 'S'
 *      <cid> is the connection ID.
 *      <N bytes> is a number of bytes
 *      <'E'> is the letter 'E'
 *      <'U'> is the letter 'U'
 *      <ip> is the ip number of the client
 *      <port> is the port number of the client
 * Inputs:
 *      uint8_t cid -- Connection ID
 *      const uint8_t *pTxData -- Data to send to the TCP connection
 *      uint32_t dataLen -- Length of data to send
 *      HOST_APP_CON_TYPE conType -- Type of connection
 *          (HOST_APP_CON_UDP_SERVER or HOST_APP_CON_UDP_CLIENT)
 *      uint8_t *pUdpClientIP -- String with client IP number
 *      uint16_t udpClientPort -- Port id of client
 * Outputs:
 *      void
 *---------------------------------------------------------------------------*/
void AtLib_SendUdpData(
        uint8_t cid,
        const uint8_t *txBuf,
        uint32_t dataLen,
        HOST_APP_CON_TYPE conType,
        const uint8_t *pUdpClientIP,
        uint16_t udpClientPort)
{
    if (HOST_APP_INVALID_CID != cid) {
        /* Construct the data start indication message */
        if (HOST_APP_CON_UDP_SERVER == conType) {
            /* <ESC> < U>  <cid> <ip address><:> <port numer><:> <data> <ESC> < E> */
            sprintf(G_ATCmdBuf, "%c%c%c%s:" _F16_ ":", HOST_APP_ESC_CHAR, 'U', cid,
                    pUdpClientIP, udpClientPort);
        } else {
            /* <ESC> < S>  <cid>  <data> <ESC> < E> */
            sprintf(G_ATCmdBuf, "%c%c%c", HOST_APP_ESC_CHAR, 'S', cid);
        }

        /* Now send the data START indication message  to S2w node */
        App_Write((uint8_t *)&G_ATCmdBuf[0], strlen(G_ATCmdBuf));

        /* Now send the actual data */
        AtLib_DataSend(txBuf, dataLen);

        /* Construct the data start indication message  */
        sprintf(G_ATCmdBuf, "%c%c", HOST_APP_ESC_CHAR, 'E');

        /* Now send the data END indication message  to S2w node */
        App_Write((uint8_t *)&G_ATCmdBuf[0], strlen(G_ATCmdBuf));
    }
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLib_BulkDataTransfer
 *---------------------------------------------------------------------------*
 * Description:
 *      Send bulk data to a current transfer.  Bulk data is transferred in
 *      the following format:
 *          <ESC><'Z'><cid><data length><N bytes><ESC><'E'>
 *      <ESC> is the escape character 0x1B
 *      <'Z'> is the letter 'Z'
 *      <cid> is the connection ID
 *      <data length> is 4 ASCII characters with the data length
 *      <N bytes> is a number of bytes, <= 1400 bytes
 *      <'E'> is the letter 'E'
 * Inputs:
 *      uint8_t cid -- Connection ID
 *      const uint8_t *pTxData -- Data to send to the TCP connection
 *      uint32_t dataLen -- Length of data to send
 *      HOST_APP_CON_TYPE conType -- Type of connection
 *          (HOST_APP_CON_UDP_SERVER or HOST_APP_CON_UDP_CLIENT)
 *      uint8_t *pUdpClientIP -- String with client IP number
 *      uint16_t udpClientPort -- Port id of client
 * Outputs:
 *      void
 *---------------------------------------------------------------------------*/
void AtLib_BulkDataTransfer(uint8_t cid, const uint8_t *pData, uint32_t dataLen)
{
    /*<Esc> <Z> <Cid> <Data Length xxxx 4 ascii char> <data> */
    int8_t digits[5];

    /* Construct the bulk data start indication message  */
    AtLib_ConvertNumberTo4DigitASCII(dataLen, digits);
    sprintf(&(G_ATCmdBuf[0]), "%c%c%c%s", HOST_APP_ESC_CHAR, 'Z', cid, digits);

    /* Now send the bulk data START indication message  to S2w node */
    App_Write((uint8_t *)&G_ATCmdBuf[0], strlen(G_ATCmdBuf));

    /* wait for GS1011 to process above command LeZ */
    MSTimerDelay(100);

    /* Now send the actual data */
    App_Write(pData, dataLen);
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLib_checkEOFMessage
 *---------------------------------------------------------------------------*
 * Description:
 *      This functions is used to check the completion of Commands
 *      This function will be called after receiving each line.
 * Inputs:
 *      const uint8_t *pBuffer -- Line of data to check
 * Outputs:
 *      HOST_APP_MSG_ID_E -- response type, HOST_APP_MSG_ID_NONE is returned
 *          if the passed line is not identified.
 *---------------------------------------------------------------------------*/
HOST_APP_MSG_ID_E AtLib_checkEOFMessage(const uint8_t *pBuffer)
{

    if ((strstr((const char *)pBuffer, "OK") != NULL)) {
        return HOST_APP_MSG_ID_OK;
    } else if ((strstr((const char *)pBuffer, "ERROR") != NULL)) {
        return HOST_APP_MSG_ID_ERROR;
    } else if ((strstr((const char *)pBuffer, "INVALID INPUT") != NULL)) {
        return HOST_APP_MSG_ID_INVALID_INPUT;
    } else if ((strstr((const char *)pBuffer, "DISASSOCIATED") != NULL)) {
        /* Reset the local flags */
        AtLib_ClearNodeAssociationFlag();
        AtLib_ClearAllCid();
        return HOST_APP_MSG_ID_DISASSOCIATION_EVENT;
    } else if ((strstr((const char *)pBuffer, "ERROR: IP CONFIG FAIL") != NULL)) {
        return HOST_APP_MSG_ID_ERROR_IP_CONFIG_FAIL;
    } else if (strstr((const char *)pBuffer, "ERROR: SOCKET FAILURE") != NULL) {
        /* Reset the local flags */
        AtLib_ClearAllCid();
        return HOST_APP_MSG_ID_ERROR_SOCKET_FAIL;
    } else if ((strstr((const char *)pBuffer, "APP Reset-APP SW Reset"))
            != NULL) {
        /* Reset the local flags */
        AtLib_ClearNodeAssociationFlag();
        AtLib_SetNodeResetFlag();
        AtLib_ClearAllCid();
        return HOST_APP_MSG_ID_APP_RESET;
    } else if (((uint8_t *)strstr((const char *)pBuffer, "DISCONNECT")) != NULL) {
        /* Reset the local flags */
        AtLib_ClearAllCid();
        return HOST_APP_MSG_ID_DISCONNECT;
    } else if ((strstr((const char *)pBuffer, "Disassociation Event")) != NULL) {
        /* reset the association flag */
        AtLib_ClearNodeAssociationFlag();
        AtLib_ClearAllCid();
        return HOST_APP_MSG_ID_DISASSOCIATION_EVENT;
    } else if ((strstr((const char *)pBuffer, "Out of StandBy-Alarm")) != NULL) {
        return HOST_APP_MSG_ID_OUT_OF_STBY_ALARM;
    } else if ((strstr((const char *)pBuffer, "Out of StandBy-Timer")) != NULL) {
        return HOST_APP_MSG_ID_OUT_OF_STBY_TIMER;
    } else if ((strstr((const char *)pBuffer, "UnExpected Warm Boot")) != NULL) {
        /* Reset the local flags */
        AtLib_ClearNodeAssociationFlag();
        AtLib_SetNodeResetFlag();
        AtLib_ClearAllCid();
        return HOST_APP_MSG_ID_UNEXPECTED_WARM_BOOT;
    } else if ((strstr((const char *)pBuffer, "Out of Deep Sleep")) != NULL) {
        return HOST_APP_MSG_ID_OUT_OF_DEEP_SLEEP;
    } else if ((strstr((const char *)pBuffer, "Serial2WiFi APP")) != NULL) {
        /* Reset the local flags */
        AtLib_ClearNodeAssociationFlag();
        AtLib_SetNodeResetFlag();
        AtLib_ClearAllCid();
        return HOST_APP_MSG_ID_WELCOME_MSG;
    } else if ((pBuffer[0] == 'A') && (pBuffer[1] == 'T')
            && (pBuffer[2] == '+')) {/* Handle the echoed back AT Command, if Echo is enabled.  "AT+" . */
        return HOST_APP_MSG_ID_NONE;
    }

    return HOST_APP_MSG_ID_NONE;
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLib_ReceiveDataHandle
 *---------------------------------------------------------------------------*
 * Description:
 *      Handle data coming in on a TCP or UDP connection.  Process all
 *      non-blocking data reads and return.
 * Inputs:
 *      void
 * Outputs:
 *      void
 *---------------------------------------------------------------------------*/
void AtLib_ReceiveDataHandle(void)
{
    uint8_t rxData;

    /* Read one byte at a time - Use non-blocking call */
    while (App_Read(&rxData, 1, 0)) {
        /* Process the received data */
        AtLib_ReceiveDataProcess(rxData);
    }
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLib_ReceiveDataProcess
 *---------------------------------------------------------------------------*
 * Description:
 *      Process individually received characters
 * Inputs:
 *      uint8_t rxData -- Character to process
 * Outputs:
 *      HOST_APP_MSG_ID_E -- response type
 *---------------------------------------------------------------------------*/
HOST_APP_MSG_ID_E AtLib_ReceiveDataProcess(uint8_t rxData)
{
    /* receive data handling state */
    static HOST_APP_RX_STATE_E receive_state = HOST_APP_RX_STATE_START;
    static uint32_t specialDataLen = 0;
    static uint8_t rxCurrentCid = 0;
    static uint8_t prevChar = 0;
    static uint32_t specialDataLenCharCount = 0;

    HOST_APP_MSG_ID_E rxMsgId = HOST_APP_MSG_ID_NONE;

#ifdef HOST_APP_DEBUG_ENABLE
    if ((isprint(rxData)) || (isspace(rxData)))
        ConsolePrintf("%c", rxData);
#endif  

    /* Process the received data */
    switch (receive_state) {
        case HOST_APP_RX_STATE_START:
            switch (rxData) {
                case HOST_APP_CR_CHAR:
                case HOST_APP_LF_CHAR:
                    /* CR and LF at the begining, just ignore it */
                    MRBufferIndex = 0;
                    break;

                case HOST_APP_ESC_CHAR:
                    /* ESCAPE sequence detected */
                    receive_state = HOST_APP_RX_STATE_ESCAPE_START;
                    MRBufferIndex = 0;
                    specialDataLen = 0;
                    rxCurrentCid = 0;
                    break;

                default:
                    /* Not start of ESC char, not start of any CR or NL */
                    MRBufferIndex = 0;
                    MRBuffer[MRBufferIndex] = rxData;
                    MRBufferIndex++;
                    receive_state = HOST_APP_RX_STATE_CMD_RESP;
                    break;
            }
            break;

        case HOST_APP_RX_STATE_CMD_RESP:
            if (HOST_APP_LF_CHAR == rxData) {
                /* LF detected - Messages from S2w node are terminated with LF/CR character */
                MRBuffer[MRBufferIndex] = rxData;

                // terminate string with NULL for strstr()
                MRBufferIndex++;
                MRBuffer[MRBufferIndex] = '\0';
                rxMsgId = AtLib_checkEOFMessage(MRBuffer);

                if (HOST_APP_MSG_ID_NONE != rxMsgId) {
                    /* command echo or end of response detected */
                    /* Now reset the  state machine */
                    receive_state = HOST_APP_RX_STATE_START;
                    MRBufferIndex = 0;
                }
            } else if (HOST_APP_ESC_CHAR == rxData) {
                /* Defensive check - This should not happen */
                receive_state = HOST_APP_RX_STATE_START;
            } else {
                MRBuffer[MRBufferIndex] = rxData;
                MRBufferIndex++;

                if (MRBufferIndex >= HOST_APP_RX_CMD_MAX_SIZE) {
                    /* Message buffer overflow. Something seriousely wrong. */
                    MRBufferIndex = 0;

                    /* Now reset the  state machine */
                    receive_state = HOST_APP_RX_STATE_START;
                }
            }
            break;

        case HOST_APP_RX_STATE_ESCAPE_START:
            if (HOST_APP_DATA_MODE_BULK_START_CHAR_H == rxData) {
                /* HTTP Bulk data handling start */
                /* <Esc>H<1 Byte - CID><4 bytes - Length of the data><data> */
                receive_state = HOST_APP_RX_STATE_HTTP_RESPONSE_DATA_HANDLE;
                specialDataLen = 0;
                specialDataLenCharCount = 0;
            } else if ((HOST_APP_DATA_MODE_BULK_START_CHAR_Z == rxData) ||
                       (HOST_APP_DATA_MODE_BULK_START_CHAR_K == rxData)){
                /* Bulk data handling start */
                /* <Esc>Z<Cid><Data Length xxxx 4 ascii char><data>   */
                receive_state = HOST_APP_RX_STATE_BULK_DATA_HANDLE;
                specialDataLen = 0;
                specialDataLenCharCount = 0;
            } else if (HOST_APP_DATA_MODE_NORMAL_START_CHAR_S == rxData) {
                /* Start of data */
                /* ESC S  cid  <----data --- > ESC E  */
                receive_state = HOST_APP_RX_STATE_DATA_HANDLE;
            } else if (HOST_APP_DATA_MODE_RAW_INDICATION_CHAR_COL == rxData) {
                /* Start of raw data  */
                /* ESC R : datalen : <----data --- >
                 Unlike other data format, there is no ESC E at the end .
                 So extract datalength to find out the incoming data size */
                receive_state = HOST_APP_RX_STATE_RAW_DATA_HANDLE;
                specialDataLen = 0;
            } else if (HOST_APP_DATA_MODE_ESC_OK_CHAR_O == rxData) {
                /* ESC command response OK */
                /* Note: No need to take any action. Its just an data reception */
                /* acknowledgement S2w node */
                receive_state = HOST_APP_RX_STATE_START;
            } else if (HOST_APP_DATA_MODE_ESC_FAIL_CHAR_F == rxData) {
                /* ESC command response FAILED */
                /* Note: Error reported from S2w node, you can use it */
                /* for debug purpose. */
                receive_state = HOST_APP_RX_STATE_START;
            } else {
                /* ESC sequence parse error !  */
                /* Reset the receive buffer */
                receive_state = HOST_APP_RX_STATE_START;
            }
            break;

        case HOST_APP_RX_STATE_DATA_HANDLE:
            rxCurrentCid = rxData;
            prevChar = 0x00;

            do {
                /* keep recieving data till you get ESC E */
                App_Read(&rxData, 1, 1);

                /* Read one byte at a time - blocking call */
                if (HOST_APP_DATA_MODE_NORMAL_END_CHAR_E == rxData) {
                    if (HOST_APP_ESC_CHAR == prevChar) {
                        /* End of data detected */
                        /* Reset the RX state machine */
                        receive_state = HOST_APP_RX_STATE_START;
                    } else {
                        AtLib_ProcessIncomingData(rxCurrentCid, rxData);
                        prevChar = rxData;
                    }
                } else {
                    AtLib_ProcessIncomingData(rxCurrentCid, rxData);
                    prevChar = rxData;
                }
            } while (receive_state != HOST_APP_RX_STATE_START);
            break;

        case HOST_APP_RX_STATE_HTTP_RESPONSE_DATA_HANDLE:
            /* HTTP response CID */
            rxCurrentCid = rxData;
            do {
                /* extracting  the rx data length*/
                /* Read one byte at a time */
                App_Read(&rxData, 1, 1);

                /* Read one byte at a time - blocking call */
                specialDataLen = (specialDataLen * 10) + ((rxData) - '0');
                specialDataLenCharCount++;
            } while (specialDataLenCharCount
                    < HOST_APP_HTTP_RESP_DATA_LEN_STRING_SIZE);

            /* Now read actual data */
            while (specialDataLen) {
                /* Read one byte at a time */
                App_Read(&rxData, 1, 1);

                /* Read one byte at a time - blocking call */
                specialDataLen--;
                AtLib_ProcessIncomingData(rxCurrentCid, rxData);
            }
            receive_state = HOST_APP_RX_STATE_START;
            break;

        case HOST_APP_RX_STATE_BULK_DATA_HANDLE:
            do {
                /* extracting  the rx data length*/
                /* Read one byte at a time - blocking call */
                App_Read(&rxData, 1, 1);

                specialDataLen = (specialDataLen * 10) + ((rxData) - '0');
                specialDataLenCharCount++;
            } while (specialDataLenCharCount
                    < HOST_APP_BULK_DATA_LEN_STRING_SIZE);
            /* Now read actual data */
            while (specialDataLen) {
                /* Read one byte at a time */
                specialDataLen--;
                AtLib_ProcessIncomingData(rxCurrentCid, rxData);
            };
            receive_state = HOST_APP_RX_STATE_START;
            break;

        case HOST_APP_RX_STATE_RAW_DATA_HANDLE:
            if (HOST_APP_DATA_MODE_RAW_INDICATION_CHAR_COL == rxData) {
                specialDataLenCharCount = 0;
            }

            /* Now reset the buffer and state machine */
            receive_state = HOST_APP_RX_STATE_START;

            do {
                /* extracting the rx data length*/
                /* Read one byte at a time */
                /* Read one byte at a time - blocking call */
                App_Read(&rxData, 1, 1);

                if (rxData != HOST_APP_DATA_MODE_RAW_INDICATION_CHAR_COL) {
                    specialDataLen = (specialDataLen * 10) + ((rxData) - '0');
                    specialDataLenCharCount++;
                }
            } while ((rxData != HOST_APP_DATA_MODE_RAW_INDICATION_CHAR_COL)
                    && (specialDataLenCharCount
                            < HOST_APP_BULK_DATA_LEN_STRING_SIZE));

            /* Now read actual data */
            while (specialDataLen) {
                /* Read one byte at a time - blocking call */
                App_Read(&rxData, 1, 1);
                specialDataLen--;
                AtLib_ProcessIncomingData(rxCurrentCid, rxData);
            }

            receive_state = HOST_APP_RX_STATE_START;
            break;

        default:
            /* This case will not be executed */
            break;
    }

    return rxMsgId;
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLib_ResponseHandle
 *---------------------------------------------------------------------------*
 * Description:
 *      Wait for a response after sending a command.  Keep parsing the
 *      data until a response is found.
 * Inputs:
 *      void
 * Outputs:
 *      HOST_APP_MSG_ID_E -- response type
 *---------------------------------------------------------------------------*/
HOST_APP_MSG_ID_E AtLib_ResponseHandle(void)
{
    HOST_APP_MSG_ID_E responseMsgId;
    uint8_t rxData;
    uint32_t timeout = MSTimerGet();
    uint8_t gotData = 0;

    /* Reset the receive buffer */
    AtLib_FlushRxBuffer();

    /* Reset the message ID */
    responseMsgId = HOST_APP_MSG_ID_NONE;

    /* Now process the response from S2w App node */
    while (HOST_APP_MSG_ID_NONE == responseMsgId) {
        gotData = 1;
        /* Read one byte at a time - non-blocking call, block here */
        timeout = MSTimerGet();
        while (!App_Read(&rxData, 1, 0)) {
            if (MSTimerDelta(timeout) >= ATLIB_RESPONSE_HANDLE_TIMEOUT) {
                gotData = 0;
                responseMsgId = HOST_APP_MSG_ID_RESPONSE_TIMEOUT;
                break;
            }
        }
        if (gotData) {
            /* Process the received data */
            responseMsgId = AtLib_ReceiveDataProcess(rxData);
            if (responseMsgId != HOST_APP_MSG_ID_NONE) {
                /* Message successfully received from S2w App node */
                break;
            }
        }
    }

    return responseMsgId;
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLib_ProcessRxChunk
 *---------------------------------------------------------------------------*
 * Description:
 *      Process a group of received bytes looking for a response message.
 * Inputs:
 *      const uint8_t *rxBuf -- Pointer to bytes
 *      uint32_t bufLen -- Number of bytes in receive buffer
 * Outputs:
 *      HOST_APP_MSG_ID_E -- response type
 *---------------------------------------------------------------------------*/
HOST_APP_MSG_ID_E AtLib_ProcessRxChunk(const uint8_t *rxBuf, uint32_t bufLen)
{
    HOST_APP_MSG_ID_E rxMsgId;

    rxMsgId = HOST_APP_MSG_ID_NONE;

    /* Parse the received data and check whether any valid message present in the chunk */
    while (bufLen) {
        /* Process the received data */
        rxMsgId = AtLib_ReceiveDataProcess(*rxBuf);
        if (rxMsgId != HOST_APP_MSG_ID_NONE) {
            /* Message received from S2w App node */
            break;
        }
        rxBuf++;
        bufLen--;
    }

    return rxMsgId;
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLib_FlushIncomingMessage
 *---------------------------------------------------------------------------*
 * Description:
 *      Read bytes until no more come in for 100 ms.
 * Inputs:
 *      void
 * Outputs:
 *      void
 *---------------------------------------------------------------------------*/
void AtLib_FlushIncomingMessage(void)
{
    /* This function will read all incoming data until nothing happens */
    /* for 100 ms */
    uint8_t rxData;
    uint32_t start;

    /* Read one byte at a time - non-blocking call */
    start = MSTimerGet();
    while (MSTimerDelta(start) < 100) {
        if (App_Read(&rxData, 1, 0)) {
            start = MSTimerGet();
#ifdef HOST_APP_DEBUG_ENABLE
            /* If required you can print the received characters on debug UART port. */
            ConsolePrintf("%c", rxData);
#endif
        }
    };
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLib_IsNodeResetDetected
 *---------------------------------------------------------------------------*
 * Description:
 *      Return flag telling if module has been reset.
 * Inputs:
 *      void
 * Outputs:
 *      uint8_t -- true if reset detected, else false
 *---------------------------------------------------------------------------*/
uint8_t AtLib_IsNodeResetDetected(void)
{
    return nodeResetFlag;
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLib_SetNodeResetFlag
 *---------------------------------------------------------------------------*
 * Description:
 *      Set the module reset flag.
 * Inputs:
 *      void
 * Outputs:
 *      void
 *---------------------------------------------------------------------------*/
void AtLib_SetNodeResetFlag(void)
{
    nodeResetFlag = true;
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLib_ClearNodeResetFlag
 *---------------------------------------------------------------------------*
 * Description:
 *      Clear the module reset flag.
 * Inputs:
 *      void
 * Outputs:
 *      void
 *---------------------------------------------------------------------------*/
void AtLib_ClearNodeResetFlag(void)
{
    nodeResetFlag = false;
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLib_IsNodeAssociated
 *---------------------------------------------------------------------------*
 * Description:
 *      Return flag that tells if module is associated with a network.
 * Inputs:
 *      void
 * Outputs:
 *      uint8_t -- true if associated with network, else false.
 *---------------------------------------------------------------------------*/
uint8_t AtLib_IsNodeAssociated(void)
{
    return nodeAssociationFlag;
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLib_SetNodeAssociationFlag
 *---------------------------------------------------------------------------*
 * Description:
 *      Set the flag for associated to network.
 * Inputs:
 *      void
 * Outputs:
 *      void
 *---------------------------------------------------------------------------*/
void AtLib_SetNodeAssociationFlag(void)
{
    nodeAssociationFlag = true;
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLib_ClearNodeAssociationFlag
 *---------------------------------------------------------------------------*
 * Description:
 *      Clear the flag for associated to network.
 * Inputs:
 *      void
 * Outputs:
 *      void
 *---------------------------------------------------------------------------*/
void AtLib_ClearNodeAssociationFlag(void)
{
    nodeAssociationFlag = false;
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLib_SaveTcpCid
 *---------------------------------------------------------------------------*
 * Description:
 *      Save the current client TCP connection id.
 * Inputs:
 *      uint8_t cid -- Current client TCP connection id.
 * Outputs:
 *      void
 *---------------------------------------------------------------------------*/
void AtLib_SaveTcpCid(uint8_t cid)
{
    tcpClientCid = cid;
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLib_SaveUdpCid
 *---------------------------------------------------------------------------*
 * Description:
 *      Save the current client UDP connection id.
 * Inputs:
 *      uint8_t cid -- Current client UDP connection id.
 * Outputs:
 *      void
 *---------------------------------------------------------------------------*/
void AtLib_SaveUdpCid(uint8_t cid)
{
    udpClientCid = cid;
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLib_ClearAllCid
 *---------------------------------------------------------------------------*
 * Description:
 *      Remove all saved TCP and UDP client ids.
 * Inputs:
 *      void
 * Outputs:
 *      void
 *---------------------------------------------------------------------------*/
void AtLib_ClearAllCid(void)
{
    tcpClientCid = HOST_APP_INVALID_CID;
    udpClientCid = HOST_APP_INVALID_CID;
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLib_GetTcpCid
 *---------------------------------------------------------------------------*
 * Description:
 *      Returned the previously saved TCP connection id.
 * Inputs:
 *      void
 * Outputs:
 *      uint8_t -- TCP connection id.
 *---------------------------------------------------------------------------*/
uint8_t AtLib_GetTcpCid(void)
{
    return tcpClientCid;
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLib_GetUdpCid
 *---------------------------------------------------------------------------*
 * Description:
 *      Returned the previously saved UDP connection id.
 * Inputs:
 *      void
 * Outputs:
 *      uint8_t -- TCP connection id.
 *---------------------------------------------------------------------------*/
uint8_t AtLib_GetUdpCid(void)
{
    return udpClientCid;
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLib_strcasecmp
 *---------------------------------------------------------------------------*
 * Description:
 *      Compared two strings with case insensitivity.
 * Inputs:
 *      const char *s1 -- First string to compare
 *      const char *s2 -- Second string to compare
 * Outputs:
 *      int32_t -- result of comparison.  0 if matches.  <0 if s1 before s2.
 *          >0 if s1 after s2.
 *---------------------------------------------------------------------------*/
int32_t AtLib_strcasecmp(const char *s1, const char *s2)
{
    const uint8_t *us1 = (const uint8_t *)s1, *us2 = (const uint8_t *)s2;

    while (tolower(*us1) == tolower(*us2)) {
        if (*us1++ == '\0')
            return (0);
        us2++;
    }

    return (tolower(*us1) - tolower(*us2));
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLib_ConvertNumberTo4DigitASCII
 *---------------------------------------------------------------------------*
 * Description:
 *      Equivalent of sprintf("%04d"), number to convert a number to
 *      four characters.
 * Inputs:
 *      uint32_t myNum -- Number to convert to text
 *      int8_t *pStr -- Place to store characters
 * Outputs:
 *      int32_t -- result of comparison.  0 if matches.  <0 if s1 before s2.
 *          >0 if s1 after s2.
 *---------------------------------------------------------------------------*/
void AtLib_ConvertNumberTo4DigitASCII(uint32_t myNum, int8_t *pStr)
{
    uint8_t digit1;
    uint8_t digit2;
    uint8_t digit3;
    uint8_t digit4;

    digit1 = myNum / 1000;
    digit2 = (myNum % 1000) / 100;
    digit3 = ((myNum % 1000) % 100) / 10;
    digit4 = ((myNum % 1000) % 100) % 10;

    sprintf((char *)pStr, _F8_ _F8_ _F8_ _F8_, digit1, digit2, digit3, digit4);
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLib_ProcessIncomingData
 *---------------------------------------------------------------------------*
 * Description:
 *      Process a bytes coming from the given connection.
 * Inputs:
 *      uint8_t cid -- connection id
 *      uint8_t rxData -- Data to process
 * Outputs:
 *      void
 *---------------------------------------------------------------------------*/
void AtLib_ProcessIncomingData(uint8_t cid, uint8_t rxData)
{
    App_ProcessIncomingData(cid, rxData);
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLib_FlushRxBuffer
 *---------------------------------------------------------------------------*
 * Description:
 *      Flush by clearing the receiving buffer (MRBuffer).
 * Inputs:
 *      void
 * Outputs:
 *      void
 *---------------------------------------------------------------------------*/
void AtLib_FlushRxBuffer(void)
{
    /* Reset the response receive buffer */
    MRBufferIndex = 0;
    memset(MRBuffer, '\0', HOST_APP_RX_CMD_MAX_SIZE);
}

/*---------------------------------------------------------------------------*
 * Routine:  AtLib_Init
 *---------------------------------------------------------------------------*
 * Description:
 *      Prepare the AtLib.
 * Inputs:
 *      void
 * Outputs:
 *      void
 *---------------------------------------------------------------------------*/
void AtLib_Init(void)
{
    /* Reset the command receive buffer */
    AtLib_FlushRxBuffer();

    /* Reset the flags */
    nodeAssociationFlag = false;
    nodeResetFlag = false;
    tcpClientCid = HOST_APP_INVALID_CID;
    udpClientCid = HOST_APP_INVALID_CID;
}

/*-------------------------------------------------------------------------*
 * End of File:  AtCmdLib.c
 *-------------------------------------------------------------------------*/
