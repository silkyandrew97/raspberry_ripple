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

jack_port_t *input_port;
jack_port_t *output_port;
jack_client_t *client;

interface_parameters *inter;
overdrive_parameters *drive;
compressor_parameters *comp;

static inline void print_about(){
    printf("\n"
           "Raspberry Ripple - A Programmable Bass Guitar Effects Pedal\n"
           "(c) Copyright 2020, Andy Silk (@silkyandrew97)\n"
           "MIT License\n"
           "Project Home: https://github.com/silkyandrew97/raspberry_ripple\n");
}

static inline void print_help(){
    printf("\n"
           "Usage:\n"
           "  raspberry_ripple <effect_1> <effect_2> [Additional Arguments]\n"
           "\n"
           "Where:\n"
           "  effect_1              First effect in chain\n"
           "                        Default is compressor\n"
           "  effect_2              Second effect in chain\n"
           "                        Default is no second effect\n"
           "\n"
           "  e.g. raspberry_ripple compressor overdrive\n"
           "\n"
           "Additional Arguments (s, d and f denote string, integer and float values respectively:\n"
           "\n"
           "  Interface Parameters:\n"
           "    [--soundcard s]     Device Name\n"
           "                        Default is hw:0\n"
           "    [--nperiods d]      Periods per Buffer - Must be at least 1\n"
           "                        Default is 3 - Recommended for USB Audio Interface\n"
           "    [--nframes d]       Frames per Period - Must be at least 1\n"
           "                        Default is 64 - Soundcards vary in compatibility\n"
           "    [--fs d]            Sample Rate (Hz) - Must be at least 44100\n"
           "                        Default is 48000 - Soundcards vary in compatibility\n"
           "\n"
           "  Compressor Parameters:\n"
           "    [--ratio f]         Compression Ratio - Must be more than 20\n"
           "                        Default is 50.0f\n"
           "    [--knee_width f]    Transition Area in Compression Characteristic (dB)\n"
           "                        - Must be at least 0\n"
           "                        Default is 10.0f\n"
           "    [--threshold f]     Start of Compressor Characteristic (dB)\n"
           "                        - Usually taken as the minimum signal level\n"
           "                        Default is -60.0f\n"
           "    [--attack f]        Attack Time (s) - Must be at least 0\n"
           "                        Default is 0.002f\n"
           "    [--release f]       Release Time (s) - Must be at least 0.025\n"
           "                        Default is 0.3f\n"
           "    [--compression f]   Dynamic Range Compression (dB) - Must be at least 0\n"
           "                        Default is 6.0f\n"
           "    [--comp_gain f]     Compressor Gain (dB)\n"
           "                        Default is 0.0f\n"
           "\n"
           "  Overdrive Parameters:\n"
           "    [--drive f]         Overdrive Level - Must be in the range 0 to 1 (low to high)\n"
           "                        Default is 0.5f\n"
           "    [--window f]        Window Size (s) - Must be at most 59\n"
           "                        Default is 0.5f\n"
           "    [--drive_gain f]    Overdrive Gain (dB)\n"
           "                        Default is 0.0f\n"
           "\n");
}

static inline void INThandler(int sig){
    signal(sig, SIG_IGN);
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
        //Find "compressor"
        if (strcmp(argv[i], "compressor") == 0){
            if (comp->chain == 0){
                if (drive->chain == 0){
                    comp->chain = 1;
                }
                else{
                    comp->chain = 2;
                }
            }
            else{
                printf("[USER-ERROR] %s can only be set once in chain, please refer to usage guide below\n", argv[i]);
                print_help();
                exit(1);
            }
            i++;
        }
        //Find "overdrive"
        else if (strcmp(argv[i], "overdrive") == 0){
            if (drive->chain == 0){
                if (comp->chain == 0){
                    drive->chain = 1;
                }
                else{
                    drive->chain = 2;
                }
            }
            else{
                printf("[USER-ERROR] %s can only be set once in chain, please refer to usage guide below\n", argv[i]);
                print_help();
                exit(1);
            }
            i++;
        }
        //Find Additional Arguments
        else if (i == (argc - 1)){
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
                inter->fs = (uint32_t)strlen(argv[i+1]);
                inter->fslen = atoi(argv[i+1]);
                i+=2;
            }
        }
        //Compressor Parameters
        else if (strcmp(argv[i], "--ratio") == 0){
            if (sscanf(argv[i+1], "%f %c", &validf, &err) != 1){
                printf("[USER-ERROR] Invalid value '%s' for '%s', please refer to usage guide below\n", argv[i+1], argv[i]);
                print_help();
                exit(1);
            }
            else if(atof(argv[i+1])<=20.0f){
                printf("[USER-ERROR] Invalid value '%s' for '%s', please refer to usage guide below\n", argv[i+1], argv[i]);
                print_help();
                exit(1);
            }
            else{
                comp->ratio = atof(argv[i+1]);
                i+=2;
            }
        }
        else if (strcmp(argv[i], "--knee_width") == 0){
            if (sscanf(argv[i+1], "%f %c", &validf, &err) != 1){
                printf("[USER-ERROR] Invalid value '%s' for '%s', please refer to usage guide below\n", argv[i+1], argv[i]);
                print_help();
                exit(1);
            }
            else if(atof(argv[i+1])<0.0f){
                printf("[USER-ERROR] Invalid value '%s' for '%s', please refer to usage guide below\n", argv[i+1], argv[i]);
                print_help();
                exit(1);
            }
            else{
                comp->knee_width = atof(argv[i+1]);
                i+=2;
            }
        }
        else if (strcmp(argv[i], "--threshold") == 0){
            if (sscanf(argv[i+1], "%f %c", &validf, &err) != 1){
                printf("[USER-ERROR] Invalid value '%s' for '%s', please refer to usage guide below\n", argv[i+1], argv[i]);
                print_help();
                exit(1);
            }
            else{
                comp->threshold = atof(argv[i+1]);
                i+=2;
            }
        }
        else if (strcmp(argv[i], "--attack") == 0){
            if (sscanf(argv[i+1], "%f %c", &validf, &err) != 1){
                printf("[USER-ERROR] Invalid value '%s' for '%s', please refer to usage guide below\n", argv[i+1], argv[i]);
                print_help();
                exit(1);
            }
            else if(atof(argv[i+1])<0.0f){
                printf("[USER-ERROR] Invalid value '%s' for '%s', please refer to usage guide below\n", argv[i+1], argv[i]);
                print_help();
                exit(1);
            }
            else{
                comp->attack_t = atof(argv[i+1]);
                i+=2;
            }
        }
        else if (strcmp(argv[i], "--release") == 0){
            if (sscanf(argv[i+1], "%f %c", &validf, &err) != 1){
                printf("[USER-ERROR] Invalid value '%s' for '%s', please refer to usage guide below\n", argv[i+1], argv[i]);
                print_help();
                exit(1);
            }
            else if(atof(argv[i+1])<0.025f){
                printf("[USER-ERROR] Invalid value '%s' for '%s', please refer to usage guide below\n", argv[i+1], argv[i]);
                print_help();
                exit(1);
            }
            else{
                comp->release_t = atof(argv[i+1]);
                i+=2;
            }
        }
        else if (strcmp(argv[i], "--compression") == 0){
            if (sscanf(argv[i+1], "%f %c", &validf, &err) != 1){
                printf("[USER-ERROR] Invalid value '%s' for '%s', please refer to usage guide below\n", argv[i+1], argv[i]);
                print_help();
                exit(1);
            }
            else if(atof(argv[i+1])<0.0f){
                printf("[USER-ERROR] Invalid value '%s' for '%s', please refer to usage guide below\n", argv[i+1], argv[i]);
                print_help();
                exit(1);
            }
            else{
                comp->compression_db = atof(argv[i+1]);
                i+=2;
            }
        }
        else if (strcmp(argv[i], "--comp_gain") == 0){
            if (sscanf(argv[i+1], "%f %c", &validf, &err) != 1){
                printf("[USER-ERROR] Invalid value '%s' for '%s', please refer to usage guide below\n", argv[i+1], argv[i]);
                print_help();
                exit(1);
            }
            else{
                comp->gain_db = atof(argv[i+1]);
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
    //Default Chain Order
    if ((comp->chain == 0) && (drive->chain == 0)){
        comp->chain = 1;
    }
    return 0;
}

//Process Callback Function - Executed on each block at the correct time
int process (jack_nframes_t nframes, void *arg){
    //Initialise pointers in and out to the memory area associated with each
    jack_default_audio_sample_t *in, *out;
    in = jack_port_get_buffer (input_port, nframes);
    out = jack_port_get_buffer (output_port, nframes);
    //Effect Chain
    if (comp->chain == 1){
        if (compressor(in, out, comp, inter)){
            fprintf(stderr,"[ERROR] in compressor effect\n");
            exit(1);
        }
        if (drive->chain == 2){
            overdrive(out, out, drive, inter);
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
    //Peak Count
    drive->peak_count++;
    //Buffer Count
    drive->buffer_count++;
    //Once window is filled, start overwriting
    if (drive->buffer_count == drive->peak_window){
        drive->buffer_count = 0;
    }
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
    
    //JACK Initialisation
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
    //Create two ports
    input_port = jack_port_register (client, "input",
                     JACK_DEFAULT_AUDIO_TYPE,
                     JackPortIsInput, 0);
    output_port = jack_port_register (client, "output",
                      JACK_DEFAULT_AUDIO_TYPE,
                      JackPortIsOutput, 0);
    if ((input_port == NULL) || (output_port == NULL)){
        fprintf(stderr, "[JACK-ERROR] Cannot register JACK ports\n");
        exit (1);
    }
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
    printf("\n"
           "Raspberry Ripple Started... Press CTRL-C to exit\n");
    
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
    if (jack_connect (client, jack_port_name (output_port), ports[0])) {
        fprintf (stderr, "[JACK-WARNING] Cannot connect output port - it has to be done manually\n");
    }
    free (ports);
    //Run until stopped by user
    sleep (-1);
    exit (0);
}
