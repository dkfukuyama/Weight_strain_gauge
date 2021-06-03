#include "mbed.h"

#include "USBSerial.h"

USBSerial  usb(0x1f00, 0x2012, 0x0001, false);

#define DT (int)(LPC_GPIO->W0[19]&1)

int main(void)
{

    LPC_IOCON->PIO0_7 = 0;
    LPC_GPIO->DIR[0] |= (1<<7); // OUTPUT

    LPC_IOCON->PIO0_17 = 0;
    LPC_GPIO->DIR[0] |= (1<<17); // OUTPUT

    LPC_IOCON->PIO0_19 = 0;
    LPC_IOCON->PIO0_19 |= (1<<5); // HYS
    LPC_GPIO->DIR[0] &= ~(1 << 19); // INPUT


    while(!usb.connected());
    
    wait_ms(1000);

    usb.printf("-------------------------------------\r\n");
    usb.printf("SYSTEM START\r\n");
    usb.printf("-------------------------------------\r\n");

    uint32_t read = 0;
    int& read_s = *(int*)(&read);  
    for(;;){
        LPC_GPIO->W0[17] = 0;
        while(DT){ wait_us(1); }
        wait_us(1);
        read = 0;

        for(int i=0; i<25; i++){
            LPC_GPIO->W0[17] = 1;
            LPC_GPIO->W0[7] = 1;
            wait_us(3);
            read = ((read << 1) | DT);
            LPC_GPIO->W0[17] = 0;
            LPC_GPIO->W0[7] = 0;
            wait_us(3);
        }
        //read_s ^= (1<<24);
        //usb.printf("0X_%010X \r\n", read);
        usb.printf("%d \r\n", read_s);
        wait_ms(100);
    }
    return 0;
}
