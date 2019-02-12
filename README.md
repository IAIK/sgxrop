# SGX-ROP: Practical Enclave Malware with Intel SGX

This repository contains the implementations of the paper "Practical Enclave Malware with Intel SGX". 
The repository consists of three parts: `tap_claw`, `demo`, and `egghunter`. 

## TAP + CLAW

Contains the Intel TSX-based primitives to check whether a page is mapped and writable without using syscalls. 

## Demo

Uses TAP + CLAW inside a (malicious) SGX enclave to break ASLR of the host application, create a ROP payload and mount a simple PoC attack (i.e., create a file in the current directory). 

## Egg Hunter

Shows how to use TAP as egg hunter for classical exploits. 


## License

All code is licensed under the MIT license.
