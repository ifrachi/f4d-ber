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
 *         Header file for the Base Edge Router (BER) module.
 * \author
 *         Idan Ifrach <idan.ifrach@mail.huji.ac.il>
 * \brief
 *         This file contains the implementation of the Base Edge Router (BER), which acts as the RPL root node,
 *         TSCH coordinator, and UDP server. It is a critical component in a wireless sensor network (WSN)
 *         designed for plant phenotyping and environmental monitoring.
 * @{
 */
 #ifndef BER_H_
 #define BER_H_

 #include "contiki.h"

 /**
  * \brief Initialize the Base Edge Router.
  */
 void ber_init(void);

 /**
  * \brief Handle periodic BER operations.
  */
 void ber_process(void);

 #endif /* BER_H_ */

 /** @} */

#ifndef BER_H
#define BER_H

#define IMF4D_BASE_SENSORS_COUNT 19

#define IMF4D_ADVANCED_SENSORS_COUNT 2

#define OLD_SP_KEYS_SIZE 11

#define ASCII_ART \
" ███████████ █████ █████  ██████████  \n"\
"░░███░░░░░░█░░███ ░░███  ░░███░░░░███ \n"\
" ░███   █ ░  ░███  ░███ █ ░███   ░░███\n"\
" ░███████    ░███████████ ░███    ░███\n"\
" ░███░░░█    ░░░░░░░███░█ ░███    ░███\n"\
" ░███  ░           ░███░  ░███    ███ \n"\
" █████             █████  ██████████  \n"\
"░░░░░             ░░░░░  ░░░░░░░░░░   \n"





const char* quotes[] = {
    "If debugging is the process of removing software bugs, then programming must be the process of putting them in.",
    "In order to understand recursion, one must first understand recursion.",
    "There are only two hard things in computer science: cache invalidation and naming things.",
    "Programming today is a race between software engineers striving to build bigger and better idiot-proof programs, and the Universe trying to produce bigger and better idiots. So far, the Universe is winning.",
    "Code never lies, comments sometimes do.",
    "Quality is not an act, it is a habit.",
    "The strength of JavaScript is that you can do anything. The weakness is that you will.",
    "A good programmer looks both ways before crossing a one-way street.",
    "It’s harder to read code than to write it.",
    "The most important property of a program is whether it accomplishes the intention of its user.",
    "Programs must be written for people to read, and only incidentally for machines to execute.",
    "Programming is the art of doing one thing at a time.",
    "The best code is no code at all.",
    "Java is to JavaScript what Car is to Carpet.",
    "Real programmers can write assembly code in any language.",
    "It’s not about how fast you can code, but how well you can think."
};

static const char *old_sensor_keys[OLD_SP_KEYS_SIZE] = {
    "light",
    "battery_t",
    "battery",
    "bmp_press",
    "bmp_temp",
    "hdc_temp",
    "hdc_humidity",
    "tmp107_amb",
    "tmp107_obj",
    "packet_number",
    "rssi"
};

// Array for raw sensor keys
static const char *raw_sensor_keys[IMF4D_BASE_SENSORS_COUNT] = {
    "bmp_390_u18_pressure_raw",
    "bmp_390_u18_temperature_raw",
    "bmp_390_u19_pressure_raw",
    "bmp_390_u19_temperature_raw",
    "hdc_2010_u13_temperature_raw",
    "hdc_2010_u13_humidity_raw",
    "hdc_2010_u16_temperature_raw",
    "hdc_2010_u16_humidity_raw",
    "hdc_2010_u17_temperature_raw",
    "hdc_2010_u17_humidity_raw",
    "opt_3001_u1_light_intensity_raw",
    "opt_3001_u2_light_intensity_raw",
    "opt_3001_u3_light_intensity_raw",
    "opt_3001_u4_light_intensity_raw",
    "opt_3001_u5_light_intensity_raw",
    "batmon_temperature_raw",
    "batmon_battery_voltage_raw",
    "package_number",
    "rssi"

};


static const char *raw_adv_sensor_keys[IMF4D_ADVANCED_SENSORS_COUNT] = {
    "co2_ppm_raw",
     "air_velocity_raw"


};

// Array for decimal sensor keys
static const char *decimal_sensor_keys[IMF4D_BASE_SENSORS_COUNT] = {
    "bmp_390_u18_pressure",
    "bmp_390_u18_temperature",
    "bmp_390_u19_pressure",
    "bmp_390_u19_temperature",
    "hdc_2010_u13_temperature",
    "hdc_2010_u13_humidity",
    "hdc_2010_u16_temperature",
    "hdc_2010_u16_humidity",
    "hdc_2010_u17_temperature",
    "hdc_2010_u17_humidity",
    "opt_3001_u1_light_intensity",
    "opt_3001_u2_light_intensity",
    "opt_3001_u3_light_intensity",
    "opt_3001_u4_light_intensity",
    "opt_3001_u5_light_intensity",
    "batmon_temperature",
    "batmon_battery_voltage",
    "package_number",
    "rssi"
};


static const char *decimal_adv_sensor_keys[IMF4D_ADVANCED_SENSORS_COUNT] = {
 "co2_ppm",
    "air_velocity"

};

    // Flags for sensors that should be displayed as floating-point
static const bool *is_float[OLD_SP_KEYS_SIZE] = {
        true,  // "light"
        false, // "battery_t"
        false, // "battery"
        true,  // "bmp_press"
        true,  // "bmp_temp"
        true,  // "hdc_temp"
        true,  // "hdc_humidity"
        true,  // "tmp107_amb"
        true,  // "tmp107_obj"
        false, // "packet_number"
        false  // "rssi"
    };



#endif // BER_H