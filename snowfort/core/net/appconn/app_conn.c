/*
 * app_conn.c
 *
 *  Created on: Aug 20, 2013
 *      Author: yzliao
 */
/*
 * \file
 * 		Application callback connector
 * \author
 * 		Yizheng Liao <yzliao@stanford.edu>
 */


#include <stdlib.h>
#include <string.h>
#include "net/mac/tdmardc.h"
#include "app_conn.h"

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

/* Local Variable */
// Local variable used to hold application callback pointer
static const struct app_callbacks* app_callback_API;

/*---------------------------------------------------------------------------*/
void app_conn_open(const struct app_callbacks *u)
{
	app_callback_API = u;
}

/*---------------------------------------------------------------------------*/
void app_conn_close(void)
{
	app_callback_API = NULL;
}

/*---------------------------------------------------------------------------*/
void app_conn_input(void)
{
	app_callback_API->recv();
}

/*---------------------------------------------------------------------------*/
void app_conn_send(const void * ptr, const uint16_t data_len)
{

  //add guard for tdma_rdc_buffer access
  while(tdma_rdc_buf_in_using_flg);

  // lock tdma_rdc_buffer and preventing access from other functions.
  tdma_rdc_buf_in_using_flg = 1;

  if((data_len+tdma_rdc_buf_ptr) <= MAX_PKT_PAYLOAD_SIZE)
  {
    memcpy(tdma_rdc_buffer+tdma_rdc_buf_ptr,ptr,data_len*sizeof(uint8_t));
    tdma_rdc_buf_ptr = tdma_rdc_buf_ptr + data_len;
  }
  else
  {
   
    uint8_t temp_len = MAX_PKT_PAYLOAD_SIZE-tdma_rdc_buf_ptr;
    memcpy(tdma_rdc_buffer+tdma_rdc_buf_ptr,ptr,temp_len*sizeof(uint8_t));
    tdma_rdc_buf_full_flg = 1;
    tdma_rdc_buf_ptr = 0;
    memcpy(tdma_rdc_buffer+tdma_rdc_buf_ptr,ptr+temp_len,(data_len-temp_len)*sizeof(uint8_t));
    tdma_rdc_buf_ptr = tdma_rdc_buf_ptr + data_len - temp_len;



  }

  if(tdma_rdc_buf_full_flg == 1)
    tdma_rdc_buf_send_ptr = tdma_rdc_buf_ptr;

  // release tdma_rdc_buffer
  tdma_rdc_buf_in_using_flg = 0;
}

/*---------------------------------------------------------------------------*/
void tdma_rdc_buf_clear(void)
{
  //add guard for tdma_rdc_buffer access
  while(tdma_rdc_buf_in_using_flg);

  // lock tdma_rdc_buffer and preventing access from other functions.
  tdma_rdc_buf_in_using_flg = 1;

  memset(tdma_rdc_buffer,0,MAX_PKT_PAYLOAD_SIZE);
  tdma_rdc_buf_ptr = 0;
  tdma_rdc_buf_send_ptr = 0;
  tdma_rdc_buf_full_flg = 0;

  // release tdma_rdc_buffer
  tdma_rdc_buf_in_using_flg = 0;

}