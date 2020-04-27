## Test Recordings Structure
For ease of navigation, the test recording structure from the root README has been included in this directory also:

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
