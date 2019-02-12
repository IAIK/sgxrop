# TSX Egg Hunter

A small (16 byte) fault-resistant egg hunter using Intel TSX. 
Given a starting address, and a 32-bit value, the egg hunter searches the address space for this value and returns the address where it found the value. 
The egg hunter does not crash on inaccessible memory and is stealthy, as it does not require any syscalls. 

## Requirements

The egg hunter was successfully tested on an i7-6700K and an i7-8650U on Ubuntu 18.04.1. 

### Hardware
* CPU supporting Intel TSX

### Software
* gcc
* make

## Build

To build the egg hunter test, simply run

    make
 
## Run

Run

    ./hunt
 
to see the egg hunter in action.

### Sample Output

A sample output could look like this

```
[INFO ] Hiding egg @ 0x7f70445eb03f
[INFO ] Starting egg hunter @ 0x7f7043706000 (14 MB before egg)
[INFO ] Found egg @ 0x7f70445eb03f
[INFO ] Egg hunt was successful!
./hunt  0.63s user 0.01s system 99% cpu 0.642 total
```
