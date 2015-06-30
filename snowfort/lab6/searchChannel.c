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
PROCESS(null_app_process, "Search Channel Process");
AUTOSTART_PROCESSES(&null_app_process);

/*---------------------------------------------------------------*/
//APP Callback function
static void app_recv(void)
{
	PROCESS_CONTEXT_BEGIN(&null_app_process);


#ifdef SF_MOTE_TYPE_SENSOR
	uint8_t payload_len = packetbuf_datalen();
	uint8_t* payload_ptr = (uint8_t *)packetbuf_dataptr();

	uint8_t i = 0;

	for(i = 0; i < payload_len-2; i++)
	{
		printf("%d,",payload_ptr[i]);
	}
	putchar('\n');

	

#endif /* SF_MOTE_TYPE_SENSOR */

	PROCESS_CONTEXT_END(&null_app_process);
}
static const struct app_callbacks nullApp_callback = {app_recv};


/*---------------------------------------------------------------*/
PROCESS_THREAD(null_app_process, ev, data)
{
	PROCESS_BEGIN();
	printf("Search Channel Started\n");

	// start shell
	serial_shell_init();
	shell_sky_init();
	shell_blink_init();
	shell_reboot_init();

	// open app conn
	app_conn_open(&nullApp_callback);

	//samples
	uint8_t samples[10] = {0,1,2,3,4,5,6,7,8,9};



	static struct etimer sn_timer;

	if(node_id != 0)
		etimer_set(&sn_timer,(unsigned long)CLOCK_SECOND>>1);

	while(1)
	{
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&sn_timer));
		etimer_reset(&sn_timer);

		app_conn_send(samples,sizeof(uint8_t)*10);
	}


	
	PROCESS_END();
}

