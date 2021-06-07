#include "mbed.h"

#include "USBSerial.h"

USBSerial  usb(0x1f00, 0x2012, 0x0001, false);

#define DT (int)(LPC_GPIO->W0[19]&1)

constexpr unsigned int StoreLength = 50;
int StorePointerA = 0;
int StorePointerB = 0;
int StoreCounterA = 0;
int StoreCounterB = 0;
int GetValue_StoreA[StoreLength];
int GetValue_StoreB[StoreLength];
constexpr int SckReadMax = 24;

inline void InitialConfig() {

    // 出力設定
    LPC_IOCON->PIO0_7 = 0;
    LPC_GPIO->DIR[0] |= (1 << 7); // OUTPUT

    // 出力設定
    LPC_IOCON->PIO0_17 = 0;
    LPC_GPIO->DIR[0] |= (1 << 17); // OUTPUT

    // 入力設定
    LPC_IOCON->PIO0_19 = 0;
    LPC_IOCON->PIO0_19 |= (1 << 5); // HYS
    LPC_GPIO->DIR[0] &= ~(1 << 19); // INPUT

    // 配列をゼロで初期化
    memset(GetValue_StoreA, 0, sizeof(GetValue_StoreA));
    memset(GetValue_StoreB, 0, sizeof(GetValue_StoreB));
}

void PrintGetValResults() {
    int sum_a = 0;
    int sum_b = 0;
    if (StoreCounterA > StoreLength && StoreCounterB > StoreLength) {
        for (int i = 0; i < StoreLength; i++) {
            sum_a += GetValue_StoreA[i];
            sum_b += GetValue_StoreB[i];
        }
        sum_a /= StoreLength;
        sum_b /= StoreLength;

        usb.printf("Results A = %08d / B = %08d\r\n", sum_a, sum_b);
    }

    StoreCounterA = 0;
    StoreCounterB = 0;
}


enum NextConversion {
    NotSet = 0,
    A_Gain128 = 1,
    B_Gain32 = 2,
    A_Gain64 = 3,
};


inline int Convert(int inp) {
    constexpr int Conversion_Const = (1 << 24);
    return inp ^ Conversion_Const - Conversion_Const;
}

int GetValue(enum NextConversion nc) {
    static enum NextConversion NextCon = NextConversion::NotSet;

    uint32_t read = 0;
    int& read_s = *(int*)(&read);

    LPC_GPIO->W0[17] = 0;
    while (DT) { wait_us(1); }
    wait_us(1);
    read = 0;

    __disable_irq();
    const int imax = SckReadMax + (int)nc;
    for (int i = 0; i < imax; i++) {
        // 波形はオシロスコープで確認していないが読み出しはできていると思われる。
        LPC_GPIO->W0[17] = 1;
        LPC_GPIO->W0[7] = 1;
        wait_us(3);
        if (i < SckReadMax) read = ((read << 1) | DT);
        LPC_GPIO->W0[17] = 0;
        LPC_GPIO->W0[7] = 0;
        wait_us(3);
    }

    switch (NextCon) {
        case A_Gain128:
        case A_Gain64:
            GetValue_StoreA[++StorePointerA];
            StorePointerA %= StoreLength;
            StoreCounterA ++;
            break;
        case B_Gain32:
            GetValue_StoreB[++StorePointerA];
            StorePointerB %= StoreLength;
            StoreCounterB++;
            break;
        default:
            // case NotSet:
            // NOP
            break;
    }
    __enable_irq();
    NextCon = nc;
    return Convert(read_s);
}

int main(void)
{
    InitialConfig();

    int wait_count = 0;
    while(!usb.connected()){
        wait_ms(100);
        wait_count++;
        if (wait_count > 20) break; // 2秒待って、USB接続がなければ次に進む
    };

    usb.printf("-------------------------------------\r\n");
    usb.printf("SYSTEM START\r\n");
    usb.printf("-------------------------------------\r\n");

    Ticker t;
    t.attach(&PrintGetValResults, 1.0f);
    
    for(;;){
        GetValue(NextConversion::B_Gain32);
        GetValue(NextConversion::A_Gain128);
    }
    return 0;
}
