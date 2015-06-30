/*
 * Copyright (c) 2008, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 * $Id: shell-sky.c,v 1.17 2010/10/12 11:34:08 adamdunkels Exp $
 */

/**
 * \file
 *         Tmote Sky-specific Contiki shell commands
 * \author
 *         Adam Dunkels <adam@sics.se>
 */
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "contiki.h"
#include "shell-sky.h"

#include "dev/watchdog.h"

#include "net/rime.h"
#include "net/netstack.h"
#include "net/mac/tdmardc.h"
#include "dev/cc2420.h"
#include "dev/leds.h"
#include "dev/sht11.h"
#include "dev/light-sensor.h"
#include "dev/battery-sensor.h"
#include "dev/sht11-sensor.h"
#include "net/rime/timesynch.h"
#include "remote-shell.h"

#include "node-id.h"


/*---------------------------------------------------------------------------*/
PROCESS(shell_nodeid_process, "nodeid");
SHELL_COMMAND(nodeid_command,
	      "nodeid",
	      "nodeid: set node ID",
	      &shell_nodeid_process);
/*
PROCESS(shell_sense_process, "sense");
SHELL_COMMAND(sense_command,
	      "sense",
	      "sense: print out sensor data",
	      &shell_sense_process);
PROCESS(shell_senseconv_process, "senseconv");
SHELL_COMMAND(senseconv_command,
	      "senseconv",
	      "senseconv: convert 'sense' data to human readable format",
	      &shell_senseconv_process);
*/
PROCESS(shell_txpower_process, "txpower");
SHELL_COMMAND(txpower_command,
	      "txpower",
	      "txpower <power>: change CC2420 transmission power (0 - 31)",
	      &shell_txpower_process);
PROCESS(shell_rfchannel_process, "rfchannel");
SHELL_COMMAND(rfchannel_command,
	      "rfchannel",
	      "rfchannel <channel>: change CC2420 radio channel (11 - 26)",
	      &shell_rfchannel_process);
PROCESS(shell_sendcmd_process, "sendcmd");
SHELL_COMMAND(sendcmd_command,
         "sendcmd",
         "sendcmd <command>: send command to remote node",
         &shell_sendcmd_process);
PROCESS(shell_timeslot_process, "timeslot");
SHELL_COMMAND(timeslot_command,
         "timeslot",
         "timeslot <command>: set time slot for TDMA",
         &shell_timeslot_process);
PROCESS(shell_sendp2pcmd_process, "sendp2pcmd");
SHELL_COMMAND(sendp2pcmd_command, 
	      "sendp2pcmd", 
	      "sendp2pcmd <command>: send command to specific remote nodes", 
	      &shell_sendp2pcmd_process);
/*---------------------------------------------------------------------------*/
#define MAX(a, b) ((a) > (b)? (a): (b))
#define MIN(a, b) ((a) < (b)? (a): (b))

#if 0
struct spectrum {
  int channel[16];
};
#define NUM_SAMPLES 4
static struct spectrum rssi_samples[NUM_SAMPLES];
static int
do_rssi(void)
{
  static int sample;
  int channel;
  
  NETSTACK_MAC.off(0);

  cc2420_on();
  for(channel = 11; channel <= 26; ++channel) {
    cc2420_set_channel(channel);
    rssi_samples[sample].channel[channel - 11] = cc2420_rssi() + 53;
  }
  
  NETSTACK_MAC.on();
  
  sample = (sample + 1) % NUM_SAMPLES;

  {
    int channel, tot;
    tot = 0;
    for(channel = 0; channel < 16; ++channel) {
      int max = -256;
      int i;
      for(i = 0; i < NUM_SAMPLES; ++i) {
	max = MAX(max, rssi_samples[i].channel[channel]);
      }
      tot += max / 20;
    }
    return tot;
  }
}

/*---------------------------------------------------------------------------*/

struct sense_msg {
  uint16_t len;
  uint16_t clock;
  uint16_t timesynch_time;
  uint16_t light1;
  uint16_t light2;
  uint16_t temp;
  uint16_t humidity;
  uint16_t rssi;
  uint16_t voltage;
};
#endif
/*---------------------------------------------------------------------------*/
#define REMOTE_CMD_NUM 6
char *remote_cmd_list[REMOTE_CMD_NUM]; //list of available remote command
uint8_t
shell_sky_is_remote_cmd(const char * str)
{
  char * strptr = (char *)str;
  char command[20] = {'\0'};
  short counter = 0;
  short command_len;
  short i;

  while((isalpha(*strptr)  == 1|| *strptr == ' ') && counter < 20)
  {
    if(isalpha(*strptr))
    {
      command[counter] = (*strptr);
      ++strptr;
      ++counter;
    }
    else if(*strptr == ' ')
    {
      ++strptr;
    }
  }
  command[counter]='\0';
  command_len = strlen(command);

  for(i = 0; i < REMOTE_CMD_NUM; i++)
  {
    if(strncmp(command,remote_cmd_list[i],command_len) == 0)
      return 1;
  }

  return 0;

}
/*---------------------------------------------------------------------------*/
#if 0
PROCESS_THREAD(shell_sense_process, ev, data)
{
  struct sense_msg msg;
  PROCESS_BEGIN();

  SENSORS_ACTIVATE(light_sensor);
  SENSORS_ACTIVATE(battery_sensor);
  SENSORS_ACTIVATE(sht11_sensor);

  msg.len = 7;
  msg.clock = clock_time();
#if TIMESYNCH_CONF_ENABLED
  msg.timesynch_time = timesynch_time();
#else /* TIMESYNCH_CONF_ENABLED */
  msg.timesynch_time = 0;
#endif /* TIMESYNCH_CONF_ENABLED */
  msg.light1 = light_sensor.value(LIGHT_SENSOR_PHOTOSYNTHETIC);
  msg.light2 = light_sensor.value(LIGHT_SENSOR_TOTAL_SOLAR);
  msg.temp = sht11_sensor.value(SHT11_SENSOR_TEMP);
  msg.humidity = sht11_sensor.value(SHT11_SENSOR_HUMIDITY);
  msg.rssi = do_rssi();
  msg.voltage = battery_sensor.value(0);

  msg.rssi = do_rssi();

  SENSORS_DEACTIVATE(light_sensor);
  SENSORS_DEACTIVATE(battery_sensor);
  SENSORS_DEACTIVATE(sht11_sensor);

  shell_output(&sense_command, &msg, sizeof(msg), "", 0);
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_senseconv_process, ev, data)
{
  struct shell_input *input;
  struct sense_msg *msg;
  PROCESS_BEGIN();
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == shell_event_input);
    input = data;

    if(input->len1 + input->len2 == 0) {
      PROCESS_EXIT();
    }
    msg = (struct sense_msg *)input->data1;

    if(msg != NULL) {
      char buf[40];
      snprintf(buf, sizeof(buf),
	       "%d", 10 * msg->light1 / 7);
      shell_output_str(&senseconv_command, "Light 1 ", buf);
      snprintf(buf, sizeof(buf),
	       "%d", 46 * msg->light2 / 10);
      shell_output_str(&senseconv_command, "Light 2 ", buf);
      snprintf(buf, sizeof(buf),
	       "%d.%d", (msg->temp / 10 - 396) / 10,
	       (msg->temp / 10 - 396) % 10);
      shell_output_str(&senseconv_command, "Temperature ", buf);
      snprintf(buf, sizeof(buf),
	       "%d", (int)(-4L + 405L * msg->humidity / 10000L));
      shell_output_str(&senseconv_command, "Relative humidity ", buf);
      snprintf(buf, sizeof(buf),
	       "%d", msg->rssi);
      shell_output_str(&senseconv_command, "RSSI ", buf);
      snprintf(buf, sizeof(buf), /* 819 = 4096 / 5 */
	       "%d.%d", (msg->voltage / 819), (10 * msg->voltage / 819) % 10);
      shell_output_str(&senseconv_command, "Voltage ", buf);
    }
  }
  PROCESS_END();
}
#endif
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_txpower_process, ev, data)
{
  struct {
    uint16_t len;
    uint16_t txpower;
  } msg;
  const char *newptr;
  char buf[20];
  PROCESS_BEGIN();

  msg.txpower = shell_strtolong(data, &newptr);
  
  /* If no transmission power was given on the command line, we print
     out the current txpower. */
  
  if(newptr == data) {
    msg.txpower = cc2420_get_txpower();
    //printf("TX Power %d\n",msg.txpower);
  } else {
    if (msg.txpower < 0 || msg.txpower > 31)
    {
      printf("Invalid TX power, see help\n");
    }
    cc2420_set_txpower(msg.txpower);
    printf("New TX Power %d (expected) %d (HW)\n",msg.txpower,cc2420_get_txpower());
  }

  snprintf(buf, sizeof(buf), "%d", msg.txpower);
  shell_output_str(&txpower_command, "Txpower: ", buf);

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_rfchannel_process, ev, data)
{
  struct {
    uint16_t len;
    uint16_t channel;
  } msg;
  const char *newptr;
  char buf[20];
  PROCESS_BEGIN();

  msg.channel = shell_strtolong(data, &newptr);
  
  /* If no channel was given on the command line, we print out the
     current channel. */
  if(newptr == data) {
    msg.channel = cc2420_get_channel();
    //printf("Current channel: %d\n",cc2420_get_channel());
  } else {
    if (msg.channel < 11 || msg.channel > 26)
    {
      printf("Invalid channel, see help\n");
      msg.channel = cc2420_get_channel();
    }
    else
    {
      cc2420_set_channel(msg.channel); 
    }
    
  }

  //msg.len = 1;
  snprintf(buf, sizeof(buf), "%d", msg.channel);
  shell_output_str(&rfchannel_command, "Current Channel: ", buf);


  //shell_output(&rfchannel_command, &msg, sizeof(msg), "", 0);

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_nodeid_process, ev, data)
{
  uint16_t nodeid;
  char buf[20];
  const char *newptr;
  PROCESS_BEGIN();

  nodeid = shell_strtolong(data, &newptr);
  
  /* If no node ID was given on the command line, we print out the
     current channel. Else we burn the new node ID. */
  if(newptr == data) {
    nodeid = node_id;
  } else {
    nodeid = shell_strtolong(data, &newptr);
    watchdog_stop();
    leds_on(LEDS_RED);
    node_id_burn(nodeid);
    leds_on(LEDS_BLUE);
    node_id_restore();
    leds_off(LEDS_RED + LEDS_BLUE);
    watchdog_start();
    sf_tdma_set_mac_addr();
  }

  snprintf(buf, sizeof(buf), "%d", nodeid);
  shell_output_str(&nodeid_command, "Node ID: ", buf);

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_timeslot_process, ev, data)
{

  uint16_t time_slot;
  char buf[20];
  const char *newptr;

  PROCESS_BEGIN();

  time_slot = shell_strtolong(data,&newptr);

  /* if no slot number was given on the command line, we print out
   * the current time slot. Else change the time slot to new one.
   */
  if (newptr == data)
  {
    time_slot = sf_tdma_get_slot_num();
  }
  else
  {
    time_slot = shell_strtolong(data,&newptr);
    NETSTACK_RDC.off(0);
    sf_tdma_set_slot_num(time_slot);
    NETSTACK_RDC.on();
  }

  snprintf(buf, sizeof(buf), "%d", time_slot);
  shell_output_str(&timeslot_command, "Current time slot: ", buf);

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_sendcmd_process, ev, data)
{

  PROCESS_BEGIN();

  //verify if the command is a valid remote command

  if(shell_sky_is_remote_cmd(data) != 0)
  {
    printf("Send COMMAND \"%s\" to remote node\n",(char *)data);
    remote_shell_send(data,strlen(data));
  }
  else
  {
    printf("\"%s\" is not a validate remote shell command\n",(char *)data);
  }

  PROCESS_END();
}

PROCESS_THREAD(shell_sendp2pcmd_process, ev, data)
{

  PROCESS_BEGIN();

  char isValid = 1;
  
  // skip past the node values.
  char node_nums[60] = {'\0'};
  int index = 0;
  char * cmd = data;
  while((isalpha(cmd[index]) == 0) && (cmd[index] != '\0')) {
    if((isdigit(cmd[index])==0) && (cmd[index] != ' ')){
      isValid = 0;
      printf("Invalid node id.\n");
      break;
    }
    node_nums[index] = cmd[index];
    index++;
  } 

  //verify command is valid.
  if((shell_sky_is_remote_cmd(cmd) != 0) && (isValid != 0))
  {
    printf("Send COMMAND \"%s\" to remote nodes %s\n",(char *)cmd, (char*)node_nums);
    remote_shell_send(data,strlen(data));
  }
  else
  {
    printf("\"%s\" is not a validate remote shell command\n",(char *)data);
  }

  PROCESS_END();
}

/*---------------------------------------------------------------------------*/
void
shell_sky_init(void)
{
  shell_register_command(&txpower_command);
  shell_register_command(&rfchannel_command);
//  shell_register_command(&sense_command);
//  shell_register_command(&senseconv_command);
  shell_register_command(&nodeid_command);
  shell_register_command(&sendcmd_command);
  shell_register_command(&sendp2pcmd_command);
  shell_register_command(&timeslot_command);

  //remote command list
  remote_cmd_list[0] = "reboot";
  remote_cmd_list[1] = "txpower";
  remote_cmd_list[2] = "blink";
  remote_cmd_list[3] = "rfchannel";
  remote_cmd_list[4] = "nodeid";
  remote_cmd_list[5] = "timeslot";

}
/*---------------------------------------------------------------------------*/
