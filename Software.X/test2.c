#pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator: High-speed crystal/resonator on RA6/OSC2/CLKOUT and RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = ON       // Power-up Timer Enable bit (PWRT enabled)
#pragma config MCLRE = ON      // RE3/MCLR pin function select bit (RE3/MCLR pin function is MCLR)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

#include <xc.h>
#include <stdint.h>
#define _XTAL_FREQ 20000000

#define buzzer PORTCbits.RC2
#define power PORTBbits.RB1
#define level PORTD
#define max_notes 65


unsigned char timerH, timerL;
unsigned char switchRitm = 1;

const uint16_t notes[max_notes]=
{
    247,294,330,330,330,370,392,392,392,440,370,370,330,294,294,330,330,247,294,
    330,330,330,370,392,392,392,440,370,370,330,294,330,330,247,294,330,330,330,
    392,440,440,440,494,523,523,494,440,494,330,330,330,370,392,392,440,494,
    330,330,330,392,370,370,330,294,330
};

unsigned char duration[max_notes]=
{
    1,1,2,2,1,1,2,2,1,1,2,2,1,1,1,1,2,1,1,2,2,1,1,2,2,1,1,2,2,1,1,2,2,1,1,2,2,1,
    1,2,2,1,1,2,2,1,1,1,1,2,1,1,2,2,2,1,
    1,2,1,1,2,2,1,1,4
};

void __interrupt() isr(void){
    if(INTF){
        switchRitm++;
        if(switchRitm > 3)
            switchRitm = 1;
        INTF = 0;
    }
    if(TMR1IF){
        TMR1H = timerH;
        TMR1L = timerL;
        buzzer = ~buzzer;
        TMR1IF = 0;
    }
}

void Delay_Ms(unsigned int s){
    unsigned int j;
    for(j = 0; j < s; j++)__delay_ms(1);
}

void soundPlay (unsigned int freq, unsigned int duration){
    float period;
    period = 400000.0/freq;
    period = 65536 - period;
    timerH = (char)(period / 256);
    timerL = (char)(period - 256 * timerH);
    TMR1H = timerH;
    TMR1L = timerL;
    T1CONbits.TMR1ON = 1;
    Delay_Ms(duration);
    T1CONbits.TMR1ON = 0;
}

void main(void){
    unsigned char i;
    
    ANSEL = ANSELH = 0;
    TRISCbits.TRISC2 = 0;
    buzzer = 0;
    TRISBbits.TRISB0 = 1; // buton ritm
    TRISBbits.TRISB1 = 1; // buton on/off
    PORTB = 0x00;
    TRISD = 0x00;
    level = 0x00;
    
    //initializare TIMER 1 
    T1CONbits.TMR1ON = 0;
    T1CONbits.TMR1CS = 0;  // clock intern (Fosc/4)
    T1CONbits.T1CKPS1 = 1; 
    T1CONbits.T1CKPS0 = 0;
    //valoarea prescaler 1:4
    TMR1IE = 1; 
    TMR1IF = 0;
    //Activare intreruperi globale si periferice
    GIE = 1;    
    PEIE = 1;   
    //Initilizare intrerupere externa pe pin-ul RB0/INT
    INTE = 1;
    INTF = 0;
    
    OPTION_REGbits.nRBPU = 0; //actiz PORTB pull-ups pentru stari individuale ale pinilor
    WPUB = 0x01;    //weak pull-up activ pentru RB0
    OPTION_REGbits.INTEDG = 0;  //intreruperea e generata pentru front-ul descrescator al pin-ului RB0 (tranzitia High -> Low)
      
    while(1){
        if(power){
            for(i = 0;i < max_notes; i++){
                if(power == 0)
                    break;
            switch (switchRitm){
                case 2:
                    level = 0x03;
                    break;
                case 3:
                    level = 0x07;
                    break;
                default:
                    level = 0x01;
            }
            soundPlay(notes[i], switchRitm*100*duration[i]);
            __delay_ms(50);
            }
            __delay_ms(3000);
        }
        else{
            PORTD = 0x00;
            switchRitm = 1;
        }
    }
}

