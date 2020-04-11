//Copyright (C) 2020, Andy Silk (@silkyandrew97)
//MIT License
//Project Home: https://github.com/silkyandrew97/raspberry_ripple

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <jack/jack.h>
#include <math.h>
#include "compressor.h"
#include "overdrive.h"
#include "interface.h"
#include "test.h"

jack_port_t *input_port;
jack_port_t *output_port_1;
jack_port_t *output_port_2;
jack_port_t *output_port_3;
jack_port_t *output_port_4;
jack_port_t *output_port_5;
jack_port_t *output_port_6;
jack_client_t *client;

interface_parameters *inter;
overdrive_parameters *drive;
compressor_parameters *comp;
test_timer timer = {
    {0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f},
    {0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f}
};

static inline void print_about(){
    printf("\n"
           "Raspberry Ripple - A Programmable Bass Guitar Effects Pedal\n"
           "(c) Copyright 2020, Andy Silk (@silkyandrew97)\n"
           "MIT License\n"
           "Project Home: https://github.com/...\n"
           "\n");
}

static inline void print_help(){
    printf("\n"
           "Usage:\n"
           "  test_overdrive [Additional Arguments]\n"
           "\n"
           "Additional Arguments (s, u and f denote string, unsigned integer and float values respectively:\n"
           "\n"
           "  Interface Parameters:\n"
           "    [--soundcard s]     Device Name\n"
           "                        Default is hw:0\n"
           "    [--nperiods u]      Periods per Buffer - Must be at least 1\n"
           "                        Default is 3 - Recommended for USB Audio Interface\n"
           "    [--nframes u]       Frames per Period - Must be at least 1\n"
           "                        Default is 64 - Soundcards vary in compatibility\n"
           "    [--fs u]            Sampling Rate (Hz) - Must be at least 44100\n"
           "                        Default is 48000 - Soundcards vary in compatibility\n"
           "\n"
           "  Overdrive Parameters:\n"
           "    [--drive f]         Overdrive Level - Automatically set in this test\n"
           "    [--window f]        Window Size (s) - Must be at most 59\n"
           "                        Default is 0.5f\n"
           "    [--drive_gain f]    Overdrive Gain (dB)\n"
           "                        Default is 0.0f\n"
           "\n");
}

static inline void INThandler(int sig){
    signal(sig, SIG_IGN);
    //Clean
    overdrive_free(drive);
    //Close Client
    printf("\n");
    jack_client_close (client);
    usleep(10000);
    printf("Raspberry Ripple Ended\n");
}

static inline int get_args(int argc, char *argv[]){
    float validf;
    int validi;
    char err;
    int i = 1;
    while (i < argc){
        //Find Additional Arguments
        if (i == (argc - 1)){
            printf("[USER-ERROR] Not enough input arguments, please refer to usage guide below\n");
            print_help();
            exit(1);
        }
        //Interface Parameters
        else if (strcmp(argv[i], "--soundcard") == 0){
            inter->sclen = (uint32_t)strlen(argv[i+1]);
            inter->soundcard = (char*)realloc(inter->soundcard, (inter->sclen + 1) * sizeof(char));
            if (inter->soundcard == NULL){
                fprintf(stderr, "[ERROR] in inter->soundcard memory allocation\n");
                return 1;
            }
            sprintf(inter->soundcard, "%s", argv[i+1]);
            i+=2;
        }
        else if (strcmp(argv[i], "--nperiods") == 0){
            if (sscanf(argv[i+1], "%d %c", &validi, &err) != 1){
                printf("[USER-ERROR] Invalid value '%s' for '%s', please refer to usage guide below\n", argv[i+1], argv[i]);
                print_help();
                exit(1);
            }
            else if(atoi(argv[i+1])<1){
                printf("[USER-ERROR] Invalid value '%s' for '%s', please refer to usage guide below\n", argv[i+1], argv[i]);
                print_help();
                exit(1);
            }
            else{
                inter->plen = (uint32_t)strlen(argv[i+1]);
                inter->nperiods = atoi(argv[i+1]);
                i+=2;
            }
        }
        else if (strcmp(argv[i], "--nframes") == 0){
            if (sscanf(argv[i+1], "%d %c", &validi, &err) != 1){
                printf("[USER-ERROR] Invalid value '%s' for '%s', please refer to usage guide below\n", argv[i+1], argv[i]);
                print_help();
                exit(1);
            }
            else if(atoi(argv[i+1])<1){
                printf("[USER-ERROR] Invalid value '%s' for '%s', please refer to usage guide below\n", argv[i+1], argv[i]);
                print_help();
                exit(1);
            }
            else{
                inter->flen = (uint32_t)strlen(argv[i+1]);
                inter->nframes = atoi(argv[i+1]);
                i+=2;
            }
        }
        else if (strcmp(argv[i], "--fs") == 0){
            if (sscanf(argv[i+1], "%d %c", &validi, &err) != 1){
                printf("[USER-ERROR] Invalid value '%s' for '%s', please refer to usage guide below\n", argv[i+1], argv[i]);
                print_help();
                exit(1);
            }
            else if(atoi(argv[i+1])<44100){
                printf("[USER-ERROR] Invalid value '%s' for '%s', please refer to usage guide below\n", argv[i+1], argv[i]);
                print_help();
                exit(1);
            }
            else{
                inter->fslen = (uint32_t)strlen(argv[i+1]);
                inter->fs = atoi(argv[i+1]);
                i+=2;
            }
        }
        //Overdrive Parameters
        else if (strcmp(argv[i], "--drive") == 0){
            if (sscanf(argv[i+1], "%f %c", &validf, &err) != 1){
                printf("[USER-ERROR] Invalid value '%s' for '%s', please refer to usage guide below\n", argv[i+1], argv[i]);
                print_help();
                exit(1);
            }
            else if((atof(argv[i+1])<0.0f) || (atof(argv[i+1])>1.0f)){
                printf("[USER-ERROR] Invalid value '%s' for '%s', please refer to usage guide below\n", argv[i+1], argv[i]);
                print_help();
                exit(1);
            }
            else{
                drive->drive = atof(argv[i+1]);
                i+=2;
            }
        }
        else if (strcmp(argv[i], "--window") == 0){
            if (sscanf(argv[i+1], "%f %c", &validf, &err) != 1){
                printf("[USER-ERROR] Invalid value '%s' for '%s', please refer to usage guide below\n", argv[i+1], argv[i]);
                print_help();
                exit(1);
            }
            else if(atof(argv[i+1])>59.0f){
                printf("[USER-ERROR] Invalid value '%s' for '%s', please refer to usage guide below\n", argv[i+1], argv[i]);
                print_help();
                exit(1);
            }
            else{
                drive->window_t = atof(argv[i+1]);
                i+=2;
            }
        }
        else if (strcmp(argv[i], "--drive_gain") == 0){
            if (sscanf(argv[i+1], "%f %c", &validf, &err) != 1){
                printf("[USER-ERROR] Invalid value '%s' for '%s', please refer to usage guide below\n", argv[i+1], argv[i]);
                print_help();
                exit(1);
            }
            else{
                drive->gain_db = atof(argv[i+1]);
                i+=2;
            }
        }
        else{
            printf("[USER-ERROR] Invalid argument '%s', please refer to usage guide below\n", argv[i]);
            print_help();
            exit(1);
        }
    }
    return 0;
}

static inline void effects_chain(jack_default_audio_sample_t *in, jack_default_audio_sample_t *out, compressor_parameters *comp, overdrive_parameters *drive, interface_parameters *inter){
    if (comp->chain == 1){
        if (compressor(in, out, comp, inter)){
            fprintf(stderr,"[ERROR] in compressor effect\n");
            exit(1);
        }
        if (drive->chain == 2){
            overdrive(in, out, drive, inter);
        }
    }
    else if (drive->chain == 1){
        overdrive(in, out, drive, inter);
        if (comp->chain == 2){
            if (compressor(out, out, comp, inter)){
                fprintf(stderr,"[ERROR] in compressor effect\n");
                exit(1);
            }
        }
    }
    else{
        memcpy(out, in, (sizeof(jack_default_audio_sample_t) * inter->nframes));
        fprintf(stderr, "[ERROR] in main process\n");
        exit(1);
    }
}

static inline void timer_calcs(float *t, clock_t begin, clock_t end){
    t[0] = (float)(end - begin) / CLOCKS_PER_SEC;
    if(t[0]>t[1]){
     t[1] = t[0];
    }
}

//Process Callback Function - Executed on each block at the correct time
int process (jack_nframes_t nframes, void *arg){
    //Timer Declarations
    clock_t begin, end, overall_begin, overall_end;
    //Overall Timer Start
    overall_begin = clock();
    //Initialise pointers in and out to the memory area associated with each
    jack_default_audio_sample_t *in, *out_1, *out_2, *out_3, *out_4, *out_5, *out_6;
    in = jack_port_get_buffer (input_port, nframes);
    out_1 = jack_port_get_buffer (output_port_1, nframes);
    out_2 = jack_port_get_buffer (output_port_2, nframes);
    out_3 = jack_port_get_buffer (output_port_3, nframes);
    out_4 = jack_port_get_buffer (output_port_4, nframes);
    out_5 = jack_port_get_buffer (output_port_5, nframes);
    out_6 = jack_port_get_buffer (output_port_6, nframes);
    
    //Global Params
    comp->chain = 0;
    drive->chain = 1;
    
    //out_1 Timer Start
    begin = clock();
    //Params - Overdrive = 0.0
    drive->drive = 0.0f;
    //Adjust Dependants
    drive->drive_coeff = 1.0f + (2.0f * powf((1.0f - drive->drive), 2.5f));
    drive->inv_drive_coeff = 1.0f / drive->drive_coeff;
    if (drive->inv_drive_coeff < 2 * THRESHOLD){
        drive->norm_factor = drive->drive_coeff * (3.0f - powf((2.0f - drive->inv_drive_coeff * 3.0f), 2.0f)) / 3.0f;
    }
    else{
        drive->norm_factor = drive->drive_coeff;
    }
    //Effect
    effects_chain(in, out_1, comp, drive, inter);
    //out_1 Timer End
    end = clock();
    //out_1 Timer Calculations
    timer_calcs(timer.t1, begin, end);
    
    //out_2 Timer Start
    begin = clock();
    //Params - Overdrive = 0.2
    drive->drive = 0.2f;
    //Adjust Coefficients
    drive->drive_coeff = 1.0f + (2.0f * powf((1.0f - drive->drive), 2.5f));
    drive->inv_drive_coeff = 1.0f / drive->drive_coeff;
    if (drive->inv_drive_coeff < 2 * THRESHOLD){
        drive->norm_factor = drive->drive_coeff * (3.0f - powf((2.0f - drive->inv_drive_coeff * 3.0f), 2.0f)) / 3.0f;
    }
    else{
        drive->norm_factor = drive->drive_coeff;
    }
    //Effect
    effects_chain(in, out_2, comp, drive, inter);
    //out_2 Timer End
    end = clock();
    //out_2 Timer Calculations
    timer_calcs(timer.t2, begin, end);
    
    //out_3 Timer Start
    begin = clock();
    //Params - Overdrive = 0.4
    drive->drive = 0.4f;
    //Adjust Coefficients
    drive->drive_coeff = 1.0f + (2.0f * powf((1.0f - drive->drive), 2.5f));
    drive->inv_drive_coeff = 1.0f / drive->drive_coeff;
    if (drive->inv_drive_coeff < 2 * THRESHOLD){
        drive->norm_factor = drive->drive_coeff * (3.0f - powf((2.0f - drive->inv_drive_coeff * 3.0f), 2.0f)) / 3.0f;
    }
    else{
        drive->norm_factor = drive->drive_coeff;
    }
    //Effect
    effects_chain(in, out_3, comp, drive, inter);
    //out_3 Timer End
    end = clock();
    //out_3 Timer Calculations
    timer_calcs(timer.t3, begin, end);
    
    //out_4 Timer Start
    begin = clock();
   //Params - Overdrive = 0.6
    drive->drive = 0.6f;
    //Adjust Coefficients
    drive->drive_coeff = 1.0f + (2.0f * powf((1.0f - drive->drive), 2.5f));
    drive->inv_drive_coeff = 1.0f / drive->drive_coeff;
    if (drive->inv_drive_coeff < 2 * THRESHOLD){
        drive->norm_factor = drive->drive_coeff * (3.0f - powf((2.0f - drive->inv_drive_coeff * 3.0f), 2.0f)) / 3.0f;
    }
    else{
        drive->norm_factor = drive->drive_coeff;
    }
    //Effect
    effects_chain(in, out_4, comp, drive, inter);
    //out_4 Timer End
    end = clock();
    //out_4 Timer Calculations
    timer_calcs(timer.t4, begin, end);
    
    //out_5 Timer Start
    begin = clock();
    //Params - Overdrive = 0.8
    drive->drive = 0.8f;
    //Adjust Coefficients
    drive->drive_coeff = 1.0f + (2.0f * powf((1.0f - drive->drive), 2.5f));
    drive->inv_drive_coeff = 1.0f / drive->drive_coeff;
    if (drive->inv_drive_coeff < 2 * THRESHOLD){
        drive->norm_factor = drive->drive_coeff * (3.0f - powf((2.0f - drive->inv_drive_coeff * 3.0f), 2.0f)) / 3.0f;
    }
    else{
        drive->norm_factor = drive->drive_coeff;
    }
    //Effect
    effects_chain(in, out_5, comp, drive, inter);
    //out_5 Timer End
    end = clock();
    //out_5 Timer Calculations
    timer_calcs(timer.t5, begin, end);
    
    //out_6 Timer Start
    begin = clock();
    //Params - Overdrive = 1.0
    drive->drive = 1.0f;
    //Adjust Coefficients
    drive->drive_coeff = 1.0f + (2.0f * powf((1.0f - drive->drive), 2.5f));
    drive->inv_drive_coeff = 1.0f / drive->drive_coeff;
    if (drive->inv_drive_coeff < 2 * THRESHOLD){
        drive->norm_factor = drive->drive_coeff * (3.0f - powf((2.0f - drive->inv_drive_coeff * 3.0f), 2.0f)) / 3.0f;
    }
    else{
        drive->norm_factor = drive->drive_coeff;
    }
    //Effect
    effects_chain(in, out_6, comp, drive, inter);
    //out_6 Timer End
    end = clock();
    //out_6 Timer Calculations
    timer_calcs(timer.t6, begin, end);
    
    //Overall Timer End
    overall_end = clock();
    //Overall Timer Calculations
    timer_calcs(timer.overall, overall_begin, overall_end);
    return 0;
}

//Shut Down Callback - if client is disconnected
void jack_shutdown (void *arg){
    exit (1);
}

int main (int argc, char *argv[]){
    //Handler to catch CTRL-C
    signal(SIGINT, INThandler);
    //Parameter Memory Allocation
    inter = malloc(sizeof(interface_parameters));
    if (inter == NULL){
        fprintf(stderr, "[ERROR] in interface_parameters memory allocation\n");
        exit(1);
    }
    comp = malloc(sizeof(compressor_parameters));
    if (comp == NULL){
        fprintf(stderr, "[ERROR] in compressor_parameters memory allocation\n");
        exit(1);
    }
    drive = malloc(sizeof(overdrive_parameters));
    if (drive == NULL){
        fprintf(stderr, "[ERROR] in overdrive_parameters memory allocation\n");
        exit(1);
    }
    //Parameter Defaults
    if(interface_default(inter)){
        fprintf(stderr,"[ERROR] in initialising interface defaults\n");
        exit(1);
    }
    compressor_default(comp);
    overdrive_default(drive);
    //Get Parameter Arguments
    if(get_args(argc, argv)){
        fprintf(stderr,"[ERROR] in getting parameter arguments\n");
        exit(1);
    }
    
    //Parameter Initialisation
    printf("\n"
    "/-----INTERFACE CONFIGURATION-----/\n"
    "\n");
    interface_init(inter);
    compressor_init(comp, inter);
    if(overdrive_init(drive, inter)){
        fprintf(stderr,"[ERROR] in overdrive parameter initialisation\n");
        exit(1);
    }
    
    
    /*...JACK Initialisation...*/
    const char **ports;
    const char *client_name = "raspberry_ripple";
    const char *server_name = NULL;
    jack_options_t options = JackNullOption;
    jack_status_t status;
    //Open a client connection to the JACK server
    client = jack_client_open (client_name, options, &status, server_name);
    if (client == NULL){
        fprintf (stderr, "[JACK-ERROR] jack_client_open() failed, status = 0x%2.0x\n", status);
        if (status & JackServerFailed){
            fprintf (stderr, "[JACK-ERROR] Unable to connect to JACK server\n");
        }
        exit (1);
    }
    if (status & JackServerStarted){
        fprintf (stderr, "[JACK-INFO] JACK server started\n");
    }
    if (status & JackNameNotUnique){
        client_name = jack_get_client_name(client);
        fprintf (stderr, "[JACK-INFO] Unique name `%s' assigned\n", client_name);
    }
    //Call process callback whenever there is work to be done
    jack_set_process_callback (client, process, 0);
    //Call shutdown callback when disconnected
    jack_on_shutdown (client, jack_shutdown, 0);
    //Create 1 input and 8 output ports
    input_port = jack_port_register (client, "input",
                     JACK_DEFAULT_AUDIO_TYPE,
                     JackPortIsInput, 0);
    output_port_1 = jack_port_register (client, "output_1",
                      JACK_DEFAULT_AUDIO_TYPE,
                      JackPortIsOutput, 0);
    output_port_2 = jack_port_register (client, "output_2",
                      JACK_DEFAULT_AUDIO_TYPE,
                      JackPortIsOutput, 0);
    output_port_3 = jack_port_register (client, "output_3",
                      JACK_DEFAULT_AUDIO_TYPE,
                      JackPortIsOutput, 0);
    output_port_4 = jack_port_register (client, "output_4",
                      JACK_DEFAULT_AUDIO_TYPE,
                      JackPortIsOutput, 0);
    output_port_5 = jack_port_register (client, "output_5",
                      JACK_DEFAULT_AUDIO_TYPE,
                      JackPortIsOutput, 0);
    output_port_6 = jack_port_register (client, "output_6",
                      JACK_DEFAULT_AUDIO_TYPE,
                      JackPortIsOutput, 0);

    if ((input_port == NULL) || (output_port_1 == NULL) ||
        (output_port_2 == NULL) || (output_port_3 == NULL) ||
        (output_port_4 == NULL) || (output_port_5 == NULL) ||
        (output_port_6 == NULL)) {
        fprintf(stderr, "no more JACK ports available\n");
        exit (1);
    }
    //Activate Client - Process will now start running
    if (jack_activate (client)){
        fprintf (stderr, "[JACK-ERROR] Cannot activate client");
        exit (1);
    }
    //Connect Ports
    ports = jack_get_ports (client, NULL, NULL, JackPortIsPhysical|JackPortIsOutput);
    if (ports == NULL){
        fprintf(stderr, "[JACK-ERROR] No physical capture ports\n");
        exit (1);
    }
    if (jack_connect (client, ports[0], jack_port_name (input_port))) {
        fprintf (stderr, "[JACK-WARNING] Cannot connect input port - it has to be done manually\n");
    }
    free (ports);
    ports = jack_get_ports (client, NULL, NULL, JackPortIsPhysical|JackPortIsInput);
    if (ports == NULL) {
        fprintf(stderr, "[JACK-ERROR] No physical playback ports\n");
        exit (1);
    }
    if (jack_connect (client, jack_port_name (output_port_1), ports[0])) {
        fprintf (stderr, "[JACK-WARNING] Cannot connect output port - it has to be done manually\n");
    }
    free (ports);
    
    //Run Raspberry Ripple
    printf("\n"
    "/-----RASPBERRY RIPPLE-----/\n");
    //Display Raspberry Ripple Info
    print_about();
    
    //Parameter Warnings
    if (inter->nframes < 64){
        printf("[USER-WARNING] Check current block length (%u frames) is compatible with USB Audio Interface\n", inter->nframes);
    }
    if (inter->nperiods != 3){
        printf("[USER-WARNING] 3 periods recommended for USB Audio Interface (currently set at %u)\n", inter->nperiods);
    }
    if (!((inter->fs == 48000) || (inter->fs == 44100))){
        printf("[USER-WARNING] Check current sampling rate (%uHz) is compatible with USB Audio Interface\n", inter->fs);
    }
    float latency = 1000.0f * (float)inter->nframes * (float)inter->nperiods / (float)inter->fs;
    if (latency > 6.0f){
        printf("[USER-WARNING] Latency (%.2fms) is more than 'just noticeable difference' (6ms)\n"
               "               - Possible audible lag in real-time\n", latency);
    }
    
    //Run until stopped by user
    printf("\n"
           "Raspberry Ripple Started... Press CTRL-C to exit\n");
    sleep (-1);
    exit (0);
}
