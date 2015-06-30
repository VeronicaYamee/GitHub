#include <stdio.h>
#include <string.h>
#include "contiki.h"
#include "net/packetbuf.h"
#include "net/netstack.h"
#include "net/mac/tdmardc.h" // for flags to sync with tdma 
#include "sys/etimer.h"
#include "appconn/app_conn.h"
#include "node-id.h"

#include "app_util.h"

#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif


/*---------------------------------------------------------------*/
PROCESS(null_app_process, "Get information Process");
AUTOSTART_PROCESSES(&null_app_process);

/*---------------------------------------------------------------*/
//APP Callback function
static void app_recv(void)
{
	PROCESS_CONTEXT_BEGIN(&null_app_process);


#ifdef SF_MOTE_TYPE_SENSOR
	uint8_t payload_len = packetbuf_datalen();
	int i = 0;
	//printf("%d, %d\n",payload_len,packetbuf_attr(PACKETBUF_ATTR_PACKET_ID));
	//printf("%s\n",(char *)packetbuf_dataptr());
	char msg[100];
	strncpy(msg,(char *)packetbuf_dataptr(),payload_len-1);
	//printf("%c%c%c\n",msg[0],msg[1],msg[2]);
	printf("%s",msg);
	printf("\n");

#endif /* SF_MOTE_TYPE_SENSOR */

	PROCESS_CONTEXT_END(&null_app_process);
}
static const struct app_callbacks nullApp_callback = {app_recv};


/*---------------------------------------------------------------*/
PROCESS_THREAD(null_app_process, ev, data)
{
	PROCESS_BEGIN();
	printf("Get information Started\n");

	// open app conn
	app_conn_open(&nullApp_callback);



	
	PROCESS_END();
}

