//Copyright (C) 2020, Andy Silk (@silkyandrew97)
//MIT License
//Project Home: https://github.com/silkyandrew97/raspberry_ripple

#include <stdlib.h>
#include <math.h>
#include "compressor.h"

static inline float db2lin(float db){
    return powf(10.0f, 0.05f * db);
}

static inline float lin2db(float lin){
    return 20.0f * log10f(lin);
}

void compressor_default(compressor_parameters *comp){
    comp->ratio = 50.0f;
    comp->knee_width = 10.0f;
    comp->threshold = -60.0f;
    comp->attack_t = 0.002f;
    comp->release_t = 0.3f;
    comp->compression_db = 6.0f;
    comp->gain_db = 0.0f;
    comp->gs[0] = 0.0f;
    comp->gs[1] = 0.0f;
    comp->chain = 0;
}

void compressor_init(compressor_parameters *comp, interface_parameters *inter){
    //Parameter Initialisation
    comp->comps = db2lin(comp->compression_db) - 1.0f;
    comp->gain = db2lin(comp->gain_db);
    if (comp->attack_t == 0.0f){
        comp->att = 0.0f;
    }
    else{
        comp->att = expf(-log10f(9.0f)/((float)inter->fs * comp->attack_t));
    }
    if (comp->release_t == 0.0f){
        comp->rel = 0.0f;
    }
    else{
        comp->rel = expf(-log10f(9.0f)/((float)inter->fs * comp->release_t));
    }
}

int compressor(jack_default_audio_sample_t *in, jack_default_audio_sample_t *out, compressor_parameters *comp, interface_parameters *inter){
    float abs, db, sc, gc, lin;
    for (uint32_t i = 0; i < inter->nframes; i++){
        //Apply Anomaly  Detection
        abs = fabsf(in[i]);
        if ((abs == 0.0f) || (isnan(abs)) || (isinf(abs))){
            out[i] = 0.0f;
            gc = 0.0f;
            //Maintain gs continuity when anomaly detected
            if (gc <= comp->gs[0]){
                comp->gs[1] = (comp->att * comp->gs[0]);
            }
            else if (gc > comp->gs[0]){
                comp->gs[1] = (comp->rel * comp->gs[0]);
            }
            else{
                comp->gs[1] = (comp->att * comp->gs[0]);
                fprintf(stderr, "[ERROR] in Anomaly Gain Smoothing\n");
                return 1;
            }
        }
        //If No Anomalies Detected
        else{
            //Convert Input Signal to dB
            db = lin2db(abs);
            //Gain Computer
            if (db < (comp->threshold - 0.5f * comp->knee_width)){
                sc = db;
            }
            else if ((db >= (comp->threshold - 0.5f * comp->knee_width)) &&
                     (db < (comp->threshold + 0.5f * comp->knee_width))){
                sc = db + (((1.0f/comp->ratio) - 1.0f) *
                     powf((db - comp->threshold + 0.5f * comp->knee_width), 2.0f)) /
                     (2.0f * comp->knee_width);
            }
            else if (db >= (comp->threshold + 0.5f * comp->knee_width)){
                sc = comp->threshold + (db - comp->threshold) / comp->ratio;
            }
            else{
                sc = db;
                fprintf(stderr, "[ERROR] in Gain Computer\n");
                return 1;
            }
            gc = sc - db;
            //Gain Smoothing
            if (gc <= comp->gs[0]){
                comp->gs[1] = (comp->att * comp->gs[0]) + (1.0f - comp->att) * gc;
            }
            else if (gc > comp->gs[0]){
                comp->gs[1] = (comp->rel * comp->gs[0]) + (1.0f - comp->rel) * gc;
            }
            else{
                comp->gs[1] = (comp->att * comp->gs[0]) + (1.0f - comp->att) * gc;
                fprintf(stderr, "[ERROR] in Gain Smoothing\n");
                return 1;
            }
            //Apply Linear Gain and Parallelisation
            lin = db2lin(comp->gs[1]);
            out[i] = (comp->comps * in[i] * lin) + in[i];
            //Apply Gain
            out[i] *= comp->gain;
        }
        comp->gs[0] = comp->gs[1];
    }
    return 0;
}
