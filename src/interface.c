//Copyright (C) 2020, Andy Silk (@silkyandrew97)
//MIT License
//Project Home: https://github.com/silkyandrew97/raspberry_ripple

#include <stdlib.h>
#include <string.h>
#include "interface.h"

int interface_default(interface_parameters *inter){
    //Set Default Parameters
    inter->soundcard = (char*)malloc((inter->sclen + 1) * sizeof(char));
    if (inter->soundcard == NULL){
        fprintf(stderr, "[ERROR] in inter->soundcard memory allocation\n");
        return 1;
    }
    sprintf(inter->soundcard, "hw:0");
    inter->nperiods = 3;
    inter->nframes = 64;
    inter->fs = 48000;
    inter->sclen = 4;
    inter->plen = 1;
    inter->flen = 2;
    inter->fslen = 5;
    return 0;
}

void interface_init(interface_parameters *inter){
    //Input Interface Parameters
    char params[(30 + inter->sclen + inter->plen + inter->flen + inter->fslen)];
    sprintf(params, "jackd -d alsa -d %s -n %d -p %d -r %d", inter->soundcard, inter->nperiods,  inter->nframes, inter->fs);
    char bash[(67 + strlen(params))];
    sprintf(bash, "#!/bin/bash\nkillall jackd\nsleep 1\n%s &\nsleep 1\nexit 1\n", params);
    //Run Bash Script
    system(bash);
}
