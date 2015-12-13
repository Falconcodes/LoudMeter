/*
 * loudmeter.c
 *
 * Created: 12.12.2015 20:57:14
 * Author: Falcon
 */

#include <mega328p.h>
#include <delay.h>
#include <stdio.h>

#define DDRLED   DDRB.5
#define DDRLEDS  DDRD.6=DDRD.7=DDRB.0=DDRB.1=DDRB.2=DDRB.3=DDRB.4
#define PORTL1 PORTD.6
#define PORTL2 PORTD.7
#define PORTL3 PORTB.0
#define PORTL4 PORTB.1
#define PORTL5 PORTB.2
#define PORTL6 PORTB.3
#define PORTL7 PORTB.4

// Voltage Reference: AVCC pin
#define ADC_VREF_TYPE ((0<<REFS1) | (1<<REFS0) | (0<<ADLAR))

int i;
unsigned long int adc;
float result, silent, loud;

// Read the AD conversion result
unsigned int read_adc(unsigned char adc_input)
{
ADMUX=adc_input | ADC_VREF_TYPE;
// Delay needed for the stabilization of the ADC input voltage
delay_us(10);
// Start the AD conversion
ADCSRA|=(1<<ADSC);
// Wait for the AD conversion to complete
while ((ADCSRA & (1<<ADIF))==0);
ADCSRA|=(1<<ADIF);
return ADCW;
}

void main(void)
{
DDRLED=1;
DDRLEDS=1;
PORTL1=PORTL2=PORTL3=PORTL4=PORTL5=PORTL6=PORTL7=0;

// ADC Clock: 1000,000 kHz Reference: AVCC pin Trigger Source: Free Running ADC0: On, ADC1: Off, ADC2: Off, ADC3: Off ADC4: Off, ADC5: Off
DIDR0=(1<<ADC5D) | (1<<ADC4D) | (1<<ADC3D) | (1<<ADC2D) | (1<<ADC1D) | (0<<ADC0D);
ADMUX=ADC_VREF_TYPE;
ADCSRA=(1<<ADEN) | (0<<ADSC) | (1<<ADATE) | (0<<ADIF) | (0<<ADIE) | (1<<ADPS2) | (0<<ADPS1) | (0<<ADPS0);
ADCSRB=(0<<ADTS2) | (0<<ADTS1) | (0<<ADTS0);

// Communication Parameters: 8 Data, 1 Stop, No Parity Receiver: Off Transmitter: On Mode: Asynchronous Baud Rate: 9600
UCSR0A=(0<<RXC0) | (0<<TXC0) | (0<<UDRE0) | (0<<FE0) | (0<<DOR0) | (0<<UPE0) | (0<<U2X0) | (0<<MPCM0);
UCSR0B=(0<<RXCIE0) | (0<<TXCIE0) | (0<<UDRIE0) | (0<<RXEN0) | (1<<TXEN0) | (0<<UCSZ02) | (0<<RXB80) | (0<<TXB80);
UCSR0C=(0<<UMSEL01) | (0<<UMSEL00) | (0<<UPM01) | (0<<UPM00) | (0<<USBS0) | (1<<UCSZ01) | (1<<UCSZ00) | (0<<UCPOL0);
UBRR0H=0x00;
UBRR0L=0x67;

 //калибровка при перезапуске контроллера - определяется уровень "тишины". Надо обеспечить тишину при включении/перезагрузке.
 for (i=0; i<10000; i++) {
   adc = read_adc (0); //read ADC0 input channel
   result = ((float)adc + 1) * 4.7 / 1024; 
   silent+=result;
 }
silent/=10000.0; //получаем среднее значение ADC при тишине

while(1){
 
 //определяем уровень шума, суммируя импульсы, большие чем (уровень тишины + 0.05)
 loud=0;
 for (i=0; i<10000; i++) {
   adc = read_adc (0); //read ADC0 input channel
   result = ((float)adc + 1) * 4.7 / 1024; //пересчет в Вольты
   if ( result >= (silent + 0.05) )  //отбираем верхние импульсы
   loud+=result; //показатель отражает среднюю громкость шума
  }
 
 loud/=10.0; //приводим к нужному диапазону значений
 
 PORTL1=PORTL2=PORTL3=PORTL4=PORTL5=PORTL6=PORTL7=0;
 
 if (loud>0)    PORTL1=1;
 if (loud>200)  PORTL2=1;
 if (loud>450)  PORTL3=1;
 if (loud>700)  PORTL4=1;
 if (loud>1300) PORTL5=1;
 if (loud>2000) PORTL6=1;
 if (loud>2300) PORTL7=1;
 
 printf("Result: %li", (unsigned long int)loud);
 }
}
