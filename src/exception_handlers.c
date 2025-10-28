#include <genesis.h>

// Exception state variables required by custom boot code
u32 registerState[16];
u32 ext1State;
u32 addrState;
u32 ext2State;
u32 srState;
u32 pcState;

// Exception callback functions - these can be empty for basic functionality
void busErrorCB() {
    // Handle bus error - for now just halt
    while(1);
}

void addressErrorCB() {
    // Handle address error - for now just halt
    while(1);
}

void illegalInstCB() {
    // Handle illegal instruction - for now just halt
    while(1);
}

void zeroDivideCB() {
    // Handle zero divide - for now just halt
    while(1);
}

void chkInstCB() {
    // Handle CHK instruction - for now just halt
    while(1);
}

void trapvInstCB() {
    // Handle TRAPV instruction - for now just halt
    while(1);
}

void privilegeViolationCB() {
    // Handle privilege violation - for now just halt
    while(1);
}

void traceCB() {
    // Handle trace - for now just halt
    while(1);
}

void line1x1xCB() {
    // Handle line 1010/1111 emulation - for now just halt
    while(1);
}

void errorExceptionCB() {
    // Handle error exception - for now just halt
    while(1);
}

// Note: intCB, eintCB, and vintCB are already defined by SGDK
// The custom boot code will use the default SGDK interrupt handlers