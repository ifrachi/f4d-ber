/*
 * Copyright (c) 2025, Professor Menachem Moshelion,
 * Field4D Research Framework, The Hebrew University of Jerusalem, Israel
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
 * ARE DISCLAIMED. IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */
/**
 * \file
 *         Base Edge Router (BER) module implementation.
 * \author
 *         Idan Ifrach <idan.ifrach@mail.huji.ac.il>
 *
 * \ingroup simplelink launchpad-cc2650
 *
 * \brief
 *         This file contains the implementation of the Base Edge Router (BER), which acts as the RPL root node,
 *         TSCH coordinator, and UDP server. It is a critical component in a wireless sensor network (WSN)
 *         designed for plant phenotyping and environmental monitoring.
 * @{
 */


#include "contiki.h"
#include "ber.h"
#include "net/routing/routing.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include "net/mac/tsch/tsch.h"
#include "sys/node-id.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "sys/ctimer.h"
#include "sys/etimer.h"
#include "sys/log.h"
#include "sys/node-id.h"
#include "sys/rtimer.h"
#include "sys/log.h"
#include <stdio.h>
#include "net/ipv6/uip.h"

#define LOG_MODULE "F4D"
#define LOG_LEVEL LOG_LEVEL_INFO
#define UDP_SERVER_PORT 1234
#define UDP_CLIENT_PORT 4321
#define WITH_SERVER_REPLY  0
#define BYTES_PER_INT 4
#define ACK_MSG "ACK"
#define IMF4D_BASE_SENSORS_COUNT 19
#define IMF4D_ADVANCED_SENSORS_COUNT 3
#define BYTES_PER_UINT32 4
#define ENERGEST_PAYLOAD_BUFFER_SIZE 8  /* Size of the uint32_t array */
char ipv6_decoded[64];

static struct simple_udp_connection udp_conn;
static int air_velocity, air_velocity_int, air_velocity_frac;
uint32_t energest_values_uint32[ENERGEST_PAYLOAD_BUFFER_SIZE];

/* Function to translate abbreviated JSON keys to original JSON keys */
void
translate_json_keys(const char *input, char *output, const char *ipv6_decoded)
{
  const char *ptr = input;
  char *out_ptr = output;

  /* Initialize the buffer with the JSON_START marker and opening brace */
  strcpy(out_ptr, "\nJSON_START\n");
  out_ptr += strlen(out_ptr);

  /* Loop through input string */
  while(*ptr != '\0') {
    if(*ptr == '\"') {
      ptr++;       /* Move past the quote */
      switch(*ptr) {
      case 'a':
        strcpy(out_ptr, "\n    \"light\"");
        break;
      case 'b':
        strcpy(out_ptr, "    \"battery\"");
        break;
      case 'c':
        strcpy(out_ptr, "    \"bmp_press\"");
        break;
      case 'd':
        strcpy(out_ptr, "    \"bmp_temp\"");
        break;
      case 'e':
        strcpy(out_ptr, "    \"hdc_temp\"");
        break;
      case 'f':
        strcpy(out_ptr, "    \"hdc_humidity\"");
        break;
      case 'g':
        strcpy(out_ptr, "    \"packet_number\"");

        break;
      default:
        out_ptr[0] = '\0';             /* If the key is not recognized */
        break;
      }
      if(out_ptr[0] != '\0') {
        out_ptr += strlen(out_ptr);
        while(*ptr != ':') {
          ptr++;           /* Move past the abbreviated key */
        }
      }
    }
    *out_ptr++ = *ptr++;     /* Copy the rest of the characters */
    if(*ptr == ',' || *ptr == '}') {
      /* Add the necessary comma or closing brace and new line */
      if(*ptr == ',') {
        *out_ptr++ = ',';
        *out_ptr++ = '\n';
      } else if(*ptr == '}') {
        ptr++;         /* Move past the closing brace */
        break;         /* End the loop as we've reached the end of the main JSON object */
      }
      ptr++;
    }
  }

  /* Add hardcoded values for battery_t, tmp107_amb, tmp107_obj, and rssi */
  strcpy(out_ptr, ",\n    \"battery_t\":99,\n");
  out_ptr += strlen(out_ptr);
  strcpy(out_ptr, "    \"tmp107_amb\":99.999,\n");
  out_ptr += strlen(out_ptr);
  strcpy(out_ptr, "    \"tmp107_obj\":99.999,\n");
  out_ptr += strlen(out_ptr);
  strcpy(out_ptr, "    \"rssi\":-99,\n");
  out_ptr += strlen(out_ptr);

  /* Add the ipv6 key and value last */
  strcpy(out_ptr, "    \"ipv6\":\"");
  out_ptr += strlen(out_ptr);
  strcpy(out_ptr, ipv6_decoded);
  out_ptr += strlen(out_ptr);
  strcpy(out_ptr, "\"\n");
  out_ptr += strlen(out_ptr);

  /* Add JSON_END marker */
  strcpy(out_ptr, "}\nJSON_END\n");
  out_ptr += strlen(out_ptr);

  /* Null-terminate the output string */
  *out_ptr = '\0';
}
void
ip_beautify(const uip_ipaddr_t *addr)
{
  char ipv6[64];
  char str[64];
  memset(ipv6, '\0', sizeof ipv6);
  memset(str, '\0', sizeof str);
  uint16_t a;
  uint16_t r = 0;
  int i, f;
  for(i = 0, f = 0; i < sizeof(uip_ipaddr_t); i += 2) {
    a = (addr->u8[i] << 8) + addr->u8[i + 1];
    if(r != a && a != 0) {
      r = a;
      sprintf(str, "%x", a);
      strcat(ipv6, str);
    }
    if(a == 0 && f >= 0) {
      if(f++ == 0) {
      }
    } else {
      if(f > 0) {
        f = -1;
      } else if(i > 0) {
      }
    }
  }
  strcpy(ipv6_decoded, ipv6);
  memset(ipv6, '\0', sizeof ipv6);
  memset(str, '\0', sizeof str);
}
/*-------------------------------------------------*/
void
print_energest_info(const char *label, uint32_t ticks, uint32_t total_ticks)
{
  float permil_float = (ticks * 1000.0f) / total_ticks;
  uint32_t permil = (uint32_t)(permil_float * 100); /* Multiply by 100 to keep two decimal places */
  printf("[INFO: Energest] %s : %10u/%10u (%u.%02u permil)\n", label, ticks, total_ticks, permil / 100, permil % 100);
}
/*-------------------------------------------------*/
void
unpack_data_and_print(uint32_t *energest_values, const char *ipv6_decoded)
{
  printf("\n\rENERGEST_START\n");
  printf("\n[INFO: Energest] --- Period summary #%u (%u seconds) [\"%s\"]\n",
         energest_values[7], energest_values[6],
         ipv6_decoded);   /* Removed strlen usage as it's not needed for "%s" */
  printf("[INFO: Energest] Total time  :    %u\n", energest_values[5]);

  print_energest_info("CPU", energest_values[0], energest_values[5]);
  print_energest_info("LPM", energest_values[1], energest_values[5]);
  print_energest_info("Deep LPM", energest_values[2], energest_values[5]);
  print_energest_info("Radio Tx", energest_values[3], energest_values[5]);
  print_energest_info("Radio Rx", energest_values[4], energest_values[5]);
  printf("\n\rENERGEST_END\n");

  /* JSON output */
  printf("\n\rENERGEST_JSON_START\n");
  printf("{\n");
  printf("  \"ipv6\": \"%s\",\n", ipv6_decoded);
  printf("  \"cpu\": %u,\n", energest_values[0]);
  printf("  \"lpm\": %u,\n", energest_values[1]);
  printf("  \"deep_lpm\": %u,\n", energest_values[2]);
  printf("  \"radio_tx\": %u,\n", energest_values[3]);
  printf("  \"radio_rx\": %u,\n", energest_values[4]);
  printf("  \"total_time\": %u,\n", energest_values[5]);
  printf("  \"period_count\": %u,\n", energest_values[7]);
  printf("  \"period_time\": %u\n", energest_values[6]);
  printf("}\n");
  printf("\rENERGEST_JSON_END\n");
}
/*-------------------------------------------------*/
void
print_raw_payload_as_json(const int *payload, int size)
{
  if(size <= 0 || size > IMF4D_BASE_SENSORS_COUNT) {
    printf("Invalid payload size.\n");
    return;
  }

  printf("{\n");
  for(int i = 0; i < size; ++i) {
    printf("    \"%s\": %d", raw_sensor_keys[i], payload[i]);
    if(i < size - 1) {
      printf(",\n");
    } else {
      printf("\n");
    }
  }
  printf("}\n");
}
/*-------------------------------------------------*/

void
print_payload_as_json(const int *payload, int size, const char *ipv6_decoded)
{
  printf("\nJSON_START\n{\n");
  printf("    \"ipv6\":\"%s\",\n", ipv6_decoded);

  for(int i = 0; i < size; i++) {
    if(strcmp(decimal_sensor_keys[i], "bmp_390_u18_pressure") == 0 ||
       strcmp(decimal_sensor_keys[i], "bmp_390_u19_pressure") == 0) {
      /* Apply transformation for bmp_390_u18_pressure and bmp_390_u19_pressure */
      int integer_part = payload[i] / 10000;
      int decimal_part = abs(payload[i] % 10000);
      printf("    \"%s\":%d.%04d", decimal_sensor_keys[i], integer_part, decimal_part);
    } else if(strcmp(decimal_sensor_keys[i], "batmon_battery_voltage") == 0 ||
              strcmp(decimal_sensor_keys[i], "batmon_temperature") == 0 ||
              strcmp(decimal_sensor_keys[i], "package_number") == 0 ||
              strcmp(decimal_sensor_keys[i], "rssi") == 0) {
      /* Print these values as integers without decimal transformation */
      printf("    \"%s\":%d", decimal_sensor_keys[i], payload[i]);
    } else if(strcmp(decimal_sensor_keys[i], "air_velocity") == 0) {
      /* Special handling for air_velocity */
      int integer_part = payload[i] / 100000;
      int decimal_part = abs(payload[i] % 100000);
      printf("    \"%s\":%d.%05d", decimal_sensor_keys[i], integer_part, decimal_part);
    } else {
      /* For other sensors, apply the decimal transformation */
      int integer_part = payload[i] / 100;
      int decimal_part = abs(payload[i] % 100);
      printf("    \"%s\":%d.%02d", decimal_sensor_keys[i], integer_part, decimal_part);
    }
    if(i < size - 1) {
      printf(",\n");
    } else {
      printf("\n");
    }
  }
  printf("}\nJSON_END\n");
}
/*-------------------------------------------------*/
// int
// bytes_to_int(const uint8_t *bytes)
// {
//   return (int)bytes[0] | ((int)bytes[1] << 8) | ((int)bytes[2] << 16) | ((int)bytes[3] << 24);
// }
int bytes_to_int(const uint8_t *bytes) {
  return (int)(bytes[0] | (bytes[1] << 8) | (bytes[2] << 16) | (bytes[3] << 24));
}
/*-------------------------------------------------*/

static void
bytes_to_uint32(const uint8_t *bytes, uint32_t *num)
{
  *num = ((uint32_t)bytes[0] << 24) |
    ((uint32_t)bytes[1] << 16) |
    ((uint32_t)bytes[2] << 8) |
    (uint32_t)bytes[3];
}
/*-------------------------------------------------*/
static void
parse_energest_data(const uint8_t *data, uint32_t *uint32_array, uint16_t datalen)
{
  if(datalen != ENERGEST_PAYLOAD_BUFFER_SIZE * BYTES_PER_UINT32) {
    printf("Data length mismatch\n");
    return;
  }
  for(int i = 0; i < ENERGEST_PAYLOAD_BUFFER_SIZE; i++) {
    bytes_to_uint32(data + i * BYTES_PER_UINT32, &uint32_array[i]);
  }
}
/*-------------------------------------------------*/
void
convertBytesToPayload(const uint8_t *bytes_payload, int *payload, int payload_size)
{
  int byte_index = 0;
  for(int i = 0; i < payload_size; ++i) {
    payload[i] = bytes_to_int(&bytes_payload[byte_index]);
    byte_index += BYTES_PER_INT;
  }
}
/*-------------------------------------------------*/
PROCESS(udp_server_process, "UDP server");
AUTOSTART_PROCESSES(&udp_server_process);
/*-------------------------------------------------*/
static void
udp_rx_callback(struct simple_udp_connection *c,
                const uip_ipaddr_t *sender_addr,
                uint16_t sender_port,
                const uip_ipaddr_t *receiver_addr,
                uint16_t receiver_port,
                const uint8_t *data,
                uint16_t datalen)
{
  ip_beautify(sender_addr);

  LOG_INFO("############################################\n");
  LOG_INFO("Received %u bytes, From %s \n", datalen, ipv6_decoded);
  LOG_INFO("############################################\n");

  int payload_size = (datalen > 0) ? (datalen / BYTES_PER_INT) : 1;

  int payload[payload_size];

  char buff[512];   /* Buffer for formatted string */

  convertBytesToPayload(data, payload, payload_size);

  switch(datalen) {
    case 2:
        // Handle PING case
        sprintf(buff, "\n\rPING received from: %.*s \n\r", strlen(ipv6_decoded), ipv6_decoded);
        printf("%s\n", buff);
        break;

    case 12:
        // Handle Air Velocity and CO2 case
        air_velocity = bytes_to_int(&data[4]);          // Convert bytes to int
        air_velocity_int = air_velocity / 100;          // Integer part of air_velocity
        air_velocity_frac = air_velocity % 100;         // Fractional part of air_velocity

        // Prepare the JSON string
        sprintf(buff,
                "\nJSON_START\n{\n"
                "    \"ipv6\":\"%.*s\",\n"
                "    \"co2_ppm\":%d,\n"
                "    \"air_velocity\":%d.%02d,\n"
                "    \"package_number\":%d\n"
                "}\nJSON_END\n",
                (int)strlen(ipv6_decoded), ipv6_decoded,
                bytes_to_int(&data[0]),
                air_velocity_int, air_velocity_frac,
                bytes_to_int(&data[8])
        );

        printf("%s\n", buff);
        break;

    case 68:
        memset(buff, 0, sizeof(buff));
        sprintf(buff,
                "\nJSON_START\n{\n"
                "    \"ipv6\":\"%.*s\",\n"
                "    \"light\":%d.%02d,\n"
                "    \"battery_t\":%d,\n"
                "    \"battery\":%d,\n"
                "    \"bmp_press\":%d.%02d,\n"
                "    \"bmp_temp\":%d.%02d,\n"
                "    \"hdc_temp\":%d.%02d,\n"
                "    \"hdc_humidity\":%d.%02d,\n"
                "    \"package_number\":%d\n"
                "}\nJSON_END\n",
                strlen(ipv6_decoded), ipv6_decoded,
                bytes_to_int(&data[0]), bytes_to_int(&data[4]),    // light
                bytes_to_int(&data[8]),                            // battery_t
                bytes_to_int(&data[12]),                           // battery
                bytes_to_int(&data[16]), bytes_to_int(&data[20]),  // bmp_press
                bytes_to_int(&data[24]), bytes_to_int(&data[28]),  // bmp_temp
                bytes_to_int(&data[32]), bytes_to_int(&data[36]),  // hdc_temp
                bytes_to_int(&data[40]), bytes_to_int(&data[44]),  // hdc_humidity
                bytes_to_int(&data[48])                            // package_number
        );
        printf("%s\n", buff);
        break;


    case 76:
        // Handle the payload of size 76 (Placeholder)
        print_payload_as_json(payload, payload_size, ipv6_decoded);
        break;

    default:
        // Handle unexpected data lengths
        LOG_INFO("Unexpected data length received.\n");
        break;
}

#if WITH_SERVER_REPLY
  /* Send ACK response for successfully processed payload */
  LOG_INFO("Sending response.\n");
  uint8_t ack_msg[4] = { 1, 0, 0, 1 };
  simple_udp_sendto(&udp_conn, ack_msg, sizeof(ack_msg), sender_addr);
#endif /* WITH_SERVER_REPLY */
}
PROCESS_THREAD(udp_server_process, ev, data) {

  PROCESS_BEGIN();
  NETSTACK_ROUTING.root_start();
  NETSTACK_MAC.on();
  tsch_set_coordinator(1);

  /* Initialize UDP connection */
  simple_udp_register(&udp_conn, UDP_SERVER_PORT, NULL,
                      UDP_CLIENT_PORT, udp_rx_callback);

  LOG_INFO("\n\n\r%s\n", ASCII_ART);

  /* Generate a random index and print a random quote */
  int n_quotes = sizeof(quotes) / sizeof(quotes[0]);
  int random_index = random_rand() % n_quotes;

  LOG_INFO("> Random Quote: \"%s\".\n", quotes[random_index]);
  LOG_INFO("> Version Number: 2.0.2.\n");
  LOG_INFO("> Initialization Completed Successfully.\n");

  PROCESS_END();
}
/*
 * ============================================================================
 * Copyright (c) 2024-2026, Menachem Moshelion,
 * The Hebrew University of Jerusalem, Israel
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions, and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions, and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 * * Neither the name of The Hebrew University of Jerusalem nor the names of
 *   its contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ============================================================================
 */
/*---------------------------------------------------------------------------*/

/** @} */