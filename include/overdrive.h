//Copyright (C) 2020, Andy Silk (@silkyandrew97)
//MIT License
//Project Home: https://github.com/silkyandrew97/raspberry_ripple

#ifndef __OVERDRIVE__
#define __OVERDRIVE__

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <jack/jack.h>
#include "interface.h"

typedef struct{
    //User Parameters
    float drive;        //Overdrive Level - Must be in the range 0 to 1 (low to high)
    float window_t;     //Window Size (s) - Must be at most 59
    float gain_db;      //Gain (dB)
    uint32_t chain;     //Position in effects line chain
    //Algorithmic Parameters
    uint32_t buffer_count, peak_count, peak_window;
    float *window_store, gain, peak, high, drive_coeff, inv_drive_coeff, norm_factor;
} overdrive_parameters;

//Set Default Parameters
void overdrive_default(overdrive_parameters *drive);

//Initialise Overdrive Parameters
int overdrive_init(overdrive_parameters *drive, interface_parameters *inter);

//Overdrive Effect
int overdrive(jack_default_audio_sample_t *in, jack_default_audio_sample_t *out, overdrive_parameters *drive, interface_parameters *inter);

#endif
