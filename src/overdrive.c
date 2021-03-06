//Copyright (C) 2020, Andy Silk (@silkyandrew97)
//MIT License
//Project Home: https://github.com/silkyandrew97/raspberry_ripple

#include <stdlib.h>
#include <math.h>
#include "overdrive.h"

#define THRESHOLD 0.3333333f

static inline float db2lin(float db){
    return powf(10.0f, 0.05f * db);
}

static inline void peak_calcs(jack_default_audio_sample_t *in, overdrive_parameters *drive, interface_parameters *inter, float prev_peak, float *local_store){
    //Calculate peak from current period
    float local_peak = 0.0f;
    float abs;
    uint32_t i;
    for (i = 0; i < inter->nframes; i++){
        abs = fabsf(in[i]);
        if (abs > local_peak){
            local_peak = abs;
        }
    }
    //Assign period peak to window_store
    drive->window_store[drive->buffer_count] = local_peak;
    //If current period peak is larger than what is stored in the window
    if (local_peak > drive->peak){
        //Assign new peak value
        drive->peak = local_peak;
        //Reset peak counter
        drive->peak_count = 0;
        //Peak Smoothing
        float prev_sample;
        for (i = 0; i < inter->nframes; i++){
            abs = fabsf(in[i]);
            if (i != 0){
                prev_sample = local_store[i-1];
            }
            else{
                prev_sample = prev_peak;
            }
            if (abs > prev_sample){
                local_store[i] = abs;
            }
            else{
                local_store[i] = prev_sample;
            }
        }
    }
    //If largest peak lost from window
    else if (drive->peak_count == drive->peak_window){
        //Calucate new window peak
        drive->peak = local_peak;
        uint32_t high = drive->buffer_count;
        for (i = drive->buffer_count + 1; i < drive->peak_window; i++){
            if (drive->window_store[i] >= drive->peak){
                drive->peak = drive->window_store[i];
                high = i;
            }
        }
        for (i = 0; i < drive->buffer_count; i++){
            if (drive->window_store[i] >= drive->peak){
                drive->peak = drive->window_store[i];
                high = i;
            }
        }
        //Reassign peak counter
        drive->peak_count = drive->buffer_count - high;
        if (high > drive->buffer_count){
            drive->peak_count += drive->peak_window;
        }
        //Peak Smoothing
        if (drive->peak < prev_peak){
            float linspace = (prev_peak - drive->peak) / inter->nframes;
            for (i = 0; i < inter->nframes; i++){
                local_store[i] = prev_peak - (((float)i+1.0f) * linspace);
            }
        }
    }
}

static inline int effect(jack_default_audio_sample_t *in, jack_default_audio_sample_t *out, overdrive_parameters *drive, interface_parameters *inter, float prev_peak, float *local_store){
    float norm, abs;
    uint32_t i;
    for (i = 0; i<inter->nframes; i++){
        //Apply Drive Coefficient
        if (drive->peak == prev_peak){
            local_store[i] = drive->peak * drive->drive_coeff;
        }
        else{
            local_store[i] *= drive->drive_coeff;
        }
        //Normalise
        norm = in[i] / local_store[i];
        abs = fabsf(norm);
        //Anomaly Detection
        if ((abs == 0.0f) || (isnan(abs)) || (isinf(abs))){
            out[i] = 0.0f;
        }
        //If No Anomalies Detected
        else{
            //Apply Static Characteristic
            if (abs <= THRESHOLD){
                out[i] = 2.0f * in[i];
            }
            else if ((abs > THRESHOLD) && (abs <= (2.0f * THRESHOLD))){
                if (norm > 0.0f){
                    out[i] = local_store[i] * (3.0f - powf((2.0f - norm * 3.0f), 2.0f)) / 3.0f;
                }
                else{
                    out[i] = local_store[i] * (-(3.0f - powf((2.0f - (abs * 3.0f)), 2.0f)) / 3.0f);
                }
            }
            else if (abs > 2.0f * THRESHOLD){
                if (norm > 0.0f){
                    out[i] = local_store[i];
                }
                else{
                    out[i] = -local_store[i];
                }
            }
            else{
                out[i] = in[i];
                fprintf(stderr, "[ERROR] in Overdrive Static Characteristic\n");
                return 1;
            }
            //Drive Coefficent Normalisation
            out[i] /= drive->norm_factor;
            //Apply Gain
            out[i] *= drive->gain;
        }
    }
    return 0;
}

void overdrive_default(overdrive_parameters *drive){
    //Set Default Parameters
    drive->drive = 0.5f;
    drive->window_t = 0.5f;
    drive->gain_db = 0.0f;
    drive->chain = 0;
    drive->buffer_count = 0;
    drive->peak_count = 0;
    drive->peak = 0.0f;
}

int overdrive_init(overdrive_parameters *drive, interface_parameters *inter){
    //Parameter Initialisation
    drive->gain = db2lin(drive->gain_db);
    drive->drive_coeff = 1.0f + (2.0f * powf((1.0f - drive->drive), 2.5f));
    drive->inv_drive_coeff = 1.0f / drive->drive_coeff;
    if (drive->inv_drive_coeff < (2 * THRESHOLD)){
        drive->norm_factor = drive->drive_coeff * (3.0f - powf((2.0f - drive->inv_drive_coeff * 3.0f), 2.0f)) / 3.0f;
    }
    else{
        drive->norm_factor = drive->drive_coeff;
    }
    //Sliding Window Calculations
    //Calculate sliding window size rounded up to nearest block multiple
    uint32_t window_n = (uint32_t)(floorf(drive->window_t * (float)inter->fs));
    uint32_t remainder = window_n % inter->nframes;
    uint32_t window;
    if(remainder != 0){
        window = window_n + inter->nframes - remainder;
    }
    else{
        window = window_n;
    }
    //Allocate memory needed to store each block's peak value
    drive->peak_window = window/inter->nframes;
    drive->window_store = (float*)malloc((float)drive->peak_window * sizeof(float));
    if (drive->window_store == NULL){
        fprintf(stderr, "[ERROR] in drive->window_store memory allocation\n");
        return 1;
    }
    //Initialise
    uint32_t i;
    for (i = 0; i < drive->peak_window; i++) {
        drive->window_store[i] = 0.0f;
    }
    return 0;
}

int overdrive(jack_default_audio_sample_t *in, jack_default_audio_sample_t *out, overdrive_parameters *drive, interface_parameters *inter){
    float prev_peak = drive->peak;
    float local_store[inter->nframes];
    float *ls = local_store;
    //Peak Calculations
    peak_calcs(in, drive, inter, prev_peak, ls);
    //Effect and Gain
    if (effect(in, out, drive, inter, prev_peak, ls)){
        fprintf(stderr,"[ERROR] in overdrive effect\n");
        exit(1);
    }
    return 0;
}
