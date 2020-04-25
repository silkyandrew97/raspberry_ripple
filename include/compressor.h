//Copyright (C) 2020, Andy Silk (@silkyandrew97)
//MIT License
//Project Home: https://github.com/silkyandrew97/raspberry_ripple

#ifndef __COMPRESSOR__
#define __COMPRESSOR__

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <jack/jack.h>
#include "interface.h"

typedef struct{
    //User Parameters
    float ratio;            //Compression Ratio - Must be more than 20
    float knee_width;       //Transition Area in Compression Characteristic (dB)
                            //- Must be at least 0
    float threshold;        //Start of Compressor Characteristic (dB)
                            //- Usually taken as the minimum signal level
    float attack_t;         //Attack Time (s) - Must be greater than 0
    float release_t;        //Release Time (s) - Must be greater than 0.025
    float compression_db;   //Dynamic Range Compression (dB) - Must be at least 0
    float gain_db;          //Gain (dB)
    uint32_t chain;         //Position in effects line chain
    //Algorithmic Parameters
    float gain, comps, att, rel, gs[2];
} compressor_parameters;

//Set Compressor Defaults
void compressor_default(compressor_parameters *comp);

//Initialise Compressor Parameters
void compressor_init(compressor_parameters *comp, interface_parameters *inter);

//Compressor Effect
int compressor(jack_default_audio_sample_t *in, jack_default_audio_sample_t *out, compressor_parameters *comp, interface_parameters *inter);

#endif
