//Copyright (C) 2020, Andy Silk (@silkyandrew97)
//MIT License
//Project Home: https://github.com/silkyandrew97/raspberry_ripple

#ifndef __JACK__
#define __JACK__

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

typedef struct{
    //User Parameters
    char *soundcard;    //Device name
    uint32_t nperiods;  //Periods per Buffer - Must be at least 1
    uint32_t nframes;   //Frames per Period - Must be of the form 2^n
    uint32_t fs;        //Sampling Rate (Hz) - Usually 44100 or 48000, depending on soundcard
    //Algorithmic Parameters
    uint32_t sclen, plen, flen, fslen;
} interface_parameters;

//Set JACK Defaults
int interface_default(interface_parameters *inter);

//Initialise Interface parameters
void interface_init(interface_parameters *inter);

#endif
