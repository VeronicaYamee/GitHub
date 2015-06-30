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
#include "dev/i2c.h"
#include "dev/mpu-6050.h"

#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#define I2C_SENSOR
#define SAMPLING_FREQ 2
#define SAMPLES_PER_FRAME (SAMPLING_FREQ/FRAMES_PER_SEC_INT)



/*---------------------------------------------------------------*/
PROCESS(null_app_process, "Sensor No Comm Process");
AUTOSTART_PROCESSES(&null_app_process);


/*---------------------------------------------------------------*/
PROCESS_THREAD(null_app_process, ev, data)
{
	PROCESS_BEGIN();
	printf("Sensor No Comm Started\n");

	// turn off radio
	NETSTACK_RDC.off(0);
	NETSTACK_RADIO.off();


	// connect with MPU
	static uint8_t MPU_status = 0;
	static mpu_data_union samples;
	static struct etimer rxtimer;
	int i = 0;

	if (node_id != 0)
	{
		MPU_status = 0;
		for(i = 0; i < 100 & (~MPU_status);i++)
		{
			MPU_status = mpu_enable();
		}

		if (MPU_status == 0)
			printf("MPU could not be enabled.\n");

		MPU_status = 0;
		for(i = 0; i < 100 & (~MPU_status);i++)
		{
			MPU_status = mpu_wakeup();
		}

		if (MPU_status == 0)
		{
			printf("MPU could not be awakened.\n");
		}
		etimer_set(&rxtimer, (unsigned long)(CLOCK_SECOND/SAMPLING_FREQ));
	}

	// sampling
	if (node_id != 0)
	{
		while(1)
		{
			PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&rxtimer));
			etimer_reset(&rxtimer);

			MPU_status = mpu_sample_all(&samples);

			if (MPU_status != 0)
			{
				printf("%d,%d,%d,%d\n",samples.data.accel_x,samples.data.accel_y,samples.data.accel_z,samples.data.temperature);
			}
			else
			{
				printf("Cannot sample data\n");
			}
		}
	}

	
	PROCESS_END();
}

