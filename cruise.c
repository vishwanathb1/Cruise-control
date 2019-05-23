#include <avr/io.h>
#include<util/delay.h>
#include<avr/interrupt.h>

#define SET_BIT(PORT, PIN) PORT |= (1<<PIN)
#define CLR_BIT(PORT, PIN) PORT &= ~(1<<PIN)

//Function prototype
void initADC();
uint16_t readADC(uint8_t ch);

//Global variables
volatile int Engine_switch = 0;
volatile int Seatbelt_switch = 0;
volatile int Cruise_switch = 0;
volatile int Brake_pedal = 0;

int main(void)
{
    CLR_BIT(DDRD, 2);
    CLR_BIT(DDRD, 3);
    CLR_BIT(DDRB, 0);
    SET_BIT(PORTB, 0);
    SET_BIT(PORTD, 2);
    SET_BIT(PORTD, 3);
    CLR_BIT(DDRC, PC2);  //Potentiometer to handle vehicle speed
    SET_BIT(DDRB, PB1);  //Oscillator to display vehicle speed
    SET_BIT(EICRA, ISC00);
    SET_BIT(EICRA, ISC10);
    SET_BIT(EIMSK, INT0);
    SET_BIT(EIMSK, INT1);

    //Engine Switch
    CLR_BIT(DDRB, 0);
    SET_BIT(PORTB, 0);
    //Seatbelt Switch
    CLR_BIT(DDRB, 2);
    SET_BIT(PORTB, 2);

    SET_BIT(DDRB, 5);   //Led for cruise on indication
    initADC();

    sei();

    TCCR1A|=((1<<COM1A1)|(1<<WGM11)|(1<<WGM10));
    TCCR1B|=((1<<WGM12)|(1<<CS01)|(1<<CS00));
    TCNT1=0x00;

    uint16_t Speed = 0;

    while(1)
    {
        if ((!(PINB&(1<<PB0))) && (!(PINB&(1<<PB2))))
        {

            if(Cruise_switch == 1 && Brake_pedal == 0 )
            {
                Speed = readADC(4);
                _delay_ms(200);
                if (Speed <= 40 && Speed => 200)
                {
                    CLR_BIT(PORTB, 5);
                    OCR1A=Speed;
                    _delay_ms(100);
                }
                else
                {
                    SET_BIT(PORTB, 5);
                    OCR1A=128;
                    _delay_ms(100);
                }
            }
            else
            {
                CLR_BIT(PORTB, 5);
                OCR1A=readADC(4);
                _delay_ms(100);
            }
        }
        else
        {
            CLR_BIT(PORTB, 5);
            OCR1A = 0;
            _delay_ms(200);
        }
    }
    return 0;
}

//Cruise control switch
ISR(INT0_vect)
{
    if(!(PIND&(1<<PD2)))
       Cruise_switch = 1;
    else
    Cruise_switch = 0;
}
//Brake pedal switch
ISR(INT1_vect)
{
    if(!(PIND&(1<<PD3)))
      Brake_pedal = 1;
    else
    Brake_pedal = 0;
}

//Initialization of ADC
void initADC()
{
    ADMUX=(1<<REFS0);
    ADCSRA=(1<<ADEN)|(7<<ADPS0);
}
//ADC conversions
uint16_t readADC(uint8_t ch)
{
    ADMUX&=0xf8;
    ch=ch&0b00000111;
    ADMUX|=ch;

    ADCSRA|=(1<<ADSC);
    while(!(ADCSRA&(1<<ADIF)));
    ADCSRA|=(1<<ADIF);
    uint16_t TEMP=0;
    TEMP|=(ADCH<<8);
    TEMP|=(ADCL);
    //ADC=TEMP;
    return(TEMP);
}
