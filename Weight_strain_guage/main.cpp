#include "mbed.h"

#include "USBSerial.h"

USBSerial  usb(0x1f00, 0x2012, 0x0001, false);

int main(void)
{

    LPC_IOCON->PIO0_7 = 0;
    LPC_GPIO->DIR[0] |= (1<<7);

    while(!usb.connected());
    
    wait_ms(1000);

    usb.printf("-------------------------------------\r\n");
    usb.printf("SYSTEM START\r\n");
    usb.printf("-------------------------------------\r\n");

    for(;;){
        LPC_GPIO->W0[7] = 0;
        wait_ms(300);
        LPC_GPIO->W0[7] = 1;
        wait_ms(100);
    }
    return 0;
}
