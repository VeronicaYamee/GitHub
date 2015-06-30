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
PROCESS(null_app_process, "Hello world Process");
AUTOSTART_PROCESSES(&null_app_process);


/*---------------------------------------------------------------*/
PROCESS_THREAD(null_app_process, ev, data)
{
	PROCESS_BEGIN();
	printf("Hello world Started\n");

	// turn off radio
	NETSTACK_RDC.off(0);
	NETSTACK_RADIO.off();

	// print this node's information
	printf("This node information:\n");
	printf("node id: %d\n",SN_ID);
	printf("TDMA slot number: %d\n",sf_tdma_get_slot_num());
	printf("Channel: %d\n", cc2420_get_channel());

	
	PROCESS_END();
}

