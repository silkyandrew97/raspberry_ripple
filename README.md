# Raspberry Ripple - A Programmable Bass Guitar Effects Pedal
Raspberry Ripple is an investigation into the development of a low-cost, high-quality, digital multi-effects pedal, designed to be programmed and reprogrammed by the amateur software engineer. Its aims are to increase availability and affordability by providing a reproducible digital audio effects (DAFX) test environment for the bass guitar, along with two working examples of real-time effects to show its effectiveness.

This implementation integrates JACK Audio Connection Kit with a Raspberry Pi 4 Model B in the C programming language, and is functionally usable with any commercial USB audio interface.
## Directory Structure
- include (shared header files)
- matlab (offline source code)
- raspberry_ripple.xcodeproj (Xcode project)
- res/test_recordings (test recordings - see below)
- src (real-time source code)
- test (real-time test files)
- tools (JACK and QjackCtl installation scripts)
- usr/bin (built program store)
## Copyright
Copyright (C) 2020, Andy Silk (@silkyandrew97)
## Installation
JACK Audio Connection Kit must be installed, and certain configurations made, for raspberry_ripple to run on the Pi 4. These are automatically implemented by running the following script from root:
```
./tools/jack_install.sh
```
NOTE: JACK will not work with more than one installed instance - ensure all other versions are removed before running script.

To build the main raspberry_ripple program, and associated test programs, run the following script from root:
```
make
```
To aid visualisation of JACK connections, QjakckCtl can be simply installed by running the following scipt from root:
```
./tools/qjackctl_install.sh
```
## Running Raspberry Ripple
The main program can be run as root using the following command:
```
./usr/bin/raspberry_ripple
```
According to the following convention:
```
Usage:
  raspberry_ripple <effect_1> <effect_2> [Additional Arguments]

Where:
  effect_1              First effect in chain
                        Default is compressor
  effect_2              Second effect in chain
                        Default is no second effect

  e.g. raspberry_ripple compressor overdrive

Additional Arguments (s, d and f denote string, integer and float values respectively:

  Interface Parameters:
    [--soundcard s]     Device Name
                        Default is hw:0
    [--nperiods d]      Periods per Buffer - Must be at least 1
                        Default is 3 - Recommended for USB Audio Interface
    [--nframes d]       Frames per Period - Must be at least 1
                        Default is 64 - Soundcards vary in compatibility
    [--fs d]            Sample Rate (Hz) - Must be at least 44100
                        Default is 48000 - Soundcards vary in compatibility

 Compressor Parameters:
    [--ratio f]         Compression Ratio - Must be more than 20
                        Default is 50.0f
    [--knee_width f]    Transition Area in Compression Characteristic (dB)
                        - Must be at least 0
                        Default is 10.0f
    [--threshold f]     Start of Compressor Characteristic (dB)
                        - Usually taken as the minimum signal level
                        Default is -60.0f
    [--attack f]        Attack Time (s) - Must be at least 0
                        Default is 0.002f
    [--release f]       Release Time (s) - Must be at least 0.025
                        Default is 0.3f
    [--compression f]   Dynamic Range Compression (dB) - Must be at least 0
                        Default is 6.0f
    [--comp_gain f]     Compressor Gain (dB)
                        Default is 0.0f 

  Overdrive Parameters:
    [--drive f]         Overdrive Level - Must be in the range 0 to 1 (low to high)
                        Default is 0.5f
    [--window f]        Window Size (s) - Must be at most 59
                        Default is 0.5f
    [--drive_gain f]    Overdrive Gain (dB)
                        Default is 0.0f
```
## Running Tests
Three end-to-end tests are included to show the example effects in isolation and together. They are run with the following command:
```
./usr/bin/test_<program>
```
According to the following convention:
```
Usage:
  test_<program> [Additional Arguments]

Where <program> can be:
  <compressor>          6 outputs at varying compression, along with the unaltered signal
                        Compression values are 0.0dB, 3.0dB, 6.0dB, 9.0dB, 12.0dB and 15.0dB
  <overdrive>           6 outputs at varying drive, along with the unaltered signal
                        Drive values are 0.0, 0.2, 0.4, 0.6, 0.8 and 1.0
  <together>            8 outputs at varying chain order, compression and drive values, along with the unaltered signal
                        Chain order is both separately and in either order, at 6.0dB and 0.5 and then 12.0dB and 1.0
  
Additional Arguments are the same as for main program, except when argument is subject of the test.
  
   e.g. test_together --soundcard hW:Alpha --threshold -50 --window 1.0
````
Real-time recordings obtained from these tests can be found in /res/test_recordings with the following structure:
| 1st Number | Test          | 2nd Number | Example                  | 3rd Number | Parameter |
| ---------- | ------------- | ---------- | ------------------------ | ---------- | --------- |
| 1          | test_together | 1          | Impulse and Sustained E  | 0          | Unaltered |
|            |               | 2          | Dancing In The Moonlight | 1          | Compressor (6.0dB) Only |
|            |               | 3          | Longview                 | 2          | Overdrive (0.5) Only |
|            |               | 4          | We Are Family            | 3          | Compressor (6.0dB) -> Overdrive (0.5) |
|            |               | 5          | Hit                      | 4          | Overdrive (0.5) -> Compressor (6.0dB) |
|            |               | 6          | Noise                    | 5          | Unaltered |
|            |               |            |                          | 6          | Compressor (12.0dB) Only |
|            |               |            |                          | 7          | Overdrive (1.0) Only |
|            |               |            |                          | 8          | Compressor (12.0dB) -> Overdrive (1.0) |
|            |               |            |                          | 9          | Overdrive (1.0) -> Compressor (12.0dB) |
| 2          | test_compressor | 1          | Impulse and Sustained E  | 0          | Unaltered |
|            |               | 2          | Dancing In The Moonlight | 1          | 0.0dB |
|            |               | 3          | Longview                 | 2          | 3.0dB |
|            |               | 4          | We Are Family            | 3          | 6.0dB |
|            |               | 5          | Hit                      | 4          | 9.0dB |
|            |               | 6          | Noise                    | 5          | 12.0dB |
|            |               |            |                          | 6          | 15.0dB |
| 3          | test_overdrive | 1          | Impulse and Sustained E  | 0          | Unaltered |
|            |               | 2          | Dancing In The Moonlight | 1          | 0.0 |
|            |               | 3          | Longview                 | 2          | 0.2 |
|            |               | 4          | We Are Family            | 3          | 0.4 |
|            |               | 5          | Hit                      | 4          | 0.6 |
|            |               | 6          | Noise                    | 5          | 0.8 |
|            |               |            |                          | 6          | 1.0 |

## Licensing
The MIT License applies to this software - please refer to the LICENSE file in the root directory for details.

This software makes extensive use of JACK Audio Connection Kit, though it has not been endorsed by any of the authors. Therefore, thanks are given to the developers for their contribution to the open-source community.
### JACK
Copyright (C) 2001-2019 by Paul Davis, Stephane Letz, Jack O'Quinn, Torben Hohn, Filipe Coelho and others.

JACK is free software; you can redistribute it and/or modify it under the terms of the GNU GPL and LGPL licenses as published by the Free Software Foundation, http://www.gnu.org. The JACK server uses the GPL, as noted in the source file headers. However, the JACK library is licensed under the LGPL, allowing proprietary programs to link with it and use JACK services. You should have received a copy of these Licenses along with the program; if not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
