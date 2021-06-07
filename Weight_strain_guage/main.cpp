#include "mbed.h"

#include "USBSerial.h"

USBSerial  usb(0x1f00, 0x2012, 0x0001, false);

#define DT (int)(LPC_GPIO->W0[19]&1)

inline int Convert(int inp) {
    constexpr int Conversion_Const = (1<<24);
    return inp^Conversion_Const - Conversion_Const;
}

enum NextConversion {
    A_Gain128 = 1,
    B_Gain64 = 2,
    A_Gain32 = 3,
};


constexpr int SckReadMax = 24;
int GetValue(enum NextConversion nc) {
    uint32_t read = 0;
    int& read_s = *(int*)(&read);

    LPC_GPIO->W0[17] = 0;
    while (DT) { wait_us(1); }
    wait_us(1);
    read = 0;

    const int imax = SckReadMax + (int)nc;
    for (int i = 0; i < imax; i++) {
        LPC_GPIO->W0[17] = 1;
        LPC_GPIO->W0[7] = 1;
        wait_us(3);
        if (i < SckReadMax) read = ((read << 1) | DT);
        LPC_GPIO->W0[17] = 0;
        LPC_GPIO->W0[7] = 0;
        wait_us(3);
    }
    return Convert(read_s);
}

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
    
    usb.printf("-------------------------------------\r\n");
    usb.printf("SYSTEM START\r\n");
    usb.printf("-------------------------------------\r\n");

    int read_s = 0;
    for(;;){
        read_s = GetValue(NextConversion::A_Gain128);
        usb.printf("%d \r\n", read_s);
        wait_ms(100);
    }
    return 0;
}
