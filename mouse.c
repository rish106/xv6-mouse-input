#include "types.h"
#include "defs.h"
#include "x86.h"
#include "mouse.h"
#include "traps.h"

#define SEND_BUSY       0x02
#define RECV_BUSY       0x01
#define MSDEFAULT       0xF6
#define MSACTIVATE      0xF4
#define MSCPU           0

// Wait until the mouse controller is ready for us to send a packet
void 
mousewait_send(void) 
{
    while (inb(MSSTATP) & SEND_BUSY);
    return;
}

// Wait until the mouse controller has data for us to receive
void 
mousewait_recv(void) 
{
    while (!(inb(MSSTATP) & RECV_BUSY));
    return;
}

// Send a one-byte command to the mouse controller, and wait for it
// to be properly acknowledged
void 
mousecmd(uchar cmd) 
{
    mousewait_send();

    outb(MSSTATP, PS2MS);
    mousewait_send();
    outb(MSDATAP, cmd);

    mousewait_recv();

    while (inb(MSDATAP) != MSACK);

    return;
}

// To activate the mouse when the system boots up
void
mouseinit(void)
{

    mousewait_send();

    outb(MSSTATP, MSEN);

    mousewait_send();
    outb(MSSTATP, 0x20);

    mousewait_recv();
    uchar status_byte = inb(MSDATAP);
    status_byte = status_byte | 0x02;

    mousewait_send();
    outb(MSSTATP, 0x60);

    mousewait_send();
    outb(MSDATAP, status_byte);

    mousecmd(MSDEFAULT);
    mousecmd(MSACTIVATE);

    ioapicenable(IRQ_MOUSE, MSCPU);

    return;

}

// To handle an interrupt raised by the mouse controller 
void
mouseintr(void)
{

    mousewait_recv();
    uint packet = 0;

    for (uint i = 0; i < 3; i++) {
        uint curr = inb(MSDATAP);
        uint offset = 8 * i;
        packet = packet | (curr << offset);
    }

    if (packet & 0b0001) {
        cprintf("LEFT\n");
    } else if (packet & 0b0010) {
        cprintf("RIGHT\n");
    } else if (packet & 0b0100) {
        cprintf("MID\n");
    }

    return;

}
