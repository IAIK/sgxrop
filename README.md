# SGX-ROP: Practical Enclave Malware with Intel SGX

This repository contains the implementations of the DIMVA 2019 paper 

* [Practical Enclave Malware with Intel SGX](https://misc0110.net/files/sgxrop.pdf) by Schwarz, Weiser, and Gruss

The repository consists of three parts: `tap_claw`, `demo`, and `egghunter`. 

## TAP + CLAW

Contains the Intel TSX-based primitives to check whether a page is mapped and writable without using syscalls. 

## Demo

Uses TAP + CLAW inside a (malicious) SGX enclave to break ASLR of the host application, create a ROP payload and mount a simple PoC attack (i.e., create a file in the current directory). 

## Egg Hunter

Shows how to use TAP as egg hunter for classical exploits. 


## Note on Broken Microcode

Intel released a document [Performance Monitoring Impact of Intel Transactional Synchronization Extension Memory](https://cdrdv2.intel.com/v1/dl/getContent/604224) describing that certain microcode updates disable the usage of TSX within SGX. This inadvertently also breaks SGX ROP in the current form. 
In case you have such a microcode update, you have the following possibilities:

#### Disable the Microcode Update
For demo/testing purposes, it is the easiest to simply disable the microcode update. This can be done using the boot parameter `disable_ucode_ldr`. 

#### Replace TSX with DataBounce or EchoLoad
In case you cannot disable the microcode update, or TSX is disabled, e.g., due to security reasons, you can replace TSX with a different primitive. Possible alternatives based on transient-execution attacks that have been shown to works are *DataBounce* as described in 
 * [Store-to-Leak Forwarding: Leaking Data on Meltdown-resistant CPUs](https://misc0110.net/files/stl-forwarding.pdf) by Schwarz, Canella, Giner, and Gruss
 
and *EchoLoad*, which also works on CPUs where *DataBounce* is mitigated, as described in
 * [KASLR: Break It, Fix It, Repeat](https://misc0110.net/files/kaslrbfr.pdf) by Canella, Schwarz, Haubenwallner, Schwarzl, and Gruss.

## License

All code is licensed under the MIT license.
