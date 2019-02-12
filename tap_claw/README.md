# TAP and CLAW

Small test application for TAP and CLAW, two primitives which can check whether a virtual address is mapped (TAP) and writable (CLAW) without using syscalls. 

## Requirements

TAP and CLAW were successfully tested on an i7-6700K and an i7-8650U on Ubuntu 18.04.1. 

### Hardware
* CPU supporting Intel TSX

### Software
* gcc
* make

## Build

To build the test application, simply run

    make
 
## Run

Run

    ./tap_claw
 
to see TAP and CLAW in action.

### Sample Output

A sample output could look like this

```
[INFO ] Address 0x7ffe15e6d1c4 (stack):           mapped        writable
[INFO ] Address 0x55e99bef8014 (global):          mapped        writable
[INFO ] Address 0x7f1d7d1cfe80 (printf):          mapped        readonly
[INFO ] Address 0x55e99bcf7891 (main):            mapped        readonly
[INFO ] Address (nil) (NULL):                     not mapped    
[INFO ] Address 0xffffffffffffffff (kernel):      not mapped    
./tap_claw  0.00s user 0.00s system 83% cpu 0.003 total
```
