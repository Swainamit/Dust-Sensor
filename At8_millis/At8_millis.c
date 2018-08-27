/*
 * Millis_led.c
 *
 * Created: 02-02-2017 18:37:12
 *  Author: LENOVO
 */ 

#define F_CPU 8000000
#include <avr/io.h>
#include <util/delay.h>
#include <lcd_16.h>
#include "lcd_16.c"
#include <avr/timercounter2.h>
#include <usart.h>
#include <avr/interrupt.h>
#include "millis.c"
#include "millis.h"
#define Threshhold 21
float arr[30]={0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int num=0,sum=0;
float arr8[8]={0,0,0,0,0,0,0,0};
int num8=0,sum8=0,run_avg8=0,count8=0,sum18=0;
int p1count=0,run_avg=0,count=0,sum1=0,num_size =0,adcval=0;

int* func(int num1)
{
	num_size=0;
	int n3=num1;
	while(n3)
	{
		num_size++;
		n3/=10;
	}
	int *arr1 =(int*)malloc(sizeof(int)*n3);
	for(int k=num_size-1;k>=0;k--)
	{
		arr1[k]=num1%10 +48;
		num1/=10;
	}
	return arr1;
}
int run_mean()
{
	if(count>30)
	{
		sum+=(p1count-arr[num]);
		arr[num]=p1count;
		run_avg=(sum/30);
		num=(num+1)%30;
	}
	else if (count<=30)
	{
		sum1=sum1+p1count;
		run_avg=sum1/count;
		sum+=(p1count-arr[num]);
		arr[num]=p1count;
		num=(num+1)%30;
	}
}
int run_mean8()
{
	if(count8>8)
	{
		sum8+=(adcval-arr8[num8]);
		arr8[num8]=adcval;
		run_avg8=(sum8/8);
		num8=(num8+1)%8;
	}
	else if (count8<=8)
	{
		sum18=sum18+adcval;
		run_avg8=sum18/count8;
		sum8+=(adcval-arr8[num8]);
		arr8[num8]=adcval;
		num8=(num8+1)%8;
	}
}
void reverse(char *str, int len)
{
	int i=0, j=len-1, temp;
	while (i<j)
	{
		temp = str[i];
		str[i] = str[j];
		str[j] = temp;
		i++; j--;
	}
}

int intToStr(int x, char str[], int d)
{
	int i = 0;
	while (x)
	{
		str[i++] = (x%10) + '0';
		x = x/10;
	}
	
	while (i < d)
	str[i++] = '0';
	
	reverse(str, i);
	str[i] = '\0';
	return i;
}
void adc_init(void)
{
	ADCSRA=(1<<ADEN)|(1<<ADSC)|(1<<ADFR)|(1<<ADPS0)|(1<<ADPS1)|(1<<ADPS2);
	SFIOR=0x00;
}
int read_adc_channel(unsigned char channel)
{
	int adc_value=0;
	unsigned char temp3;
	ADMUX=(1<<REFS0)|channel;
	_delay_ms(1);
	temp3=ADCL;
	adc_value=ADCH;
	adc_value=(adc_value<<8)|temp3;
	//adc_value=ADCW;
	return adc_value;
}
void ftoa(float n, char *res, int afterpoint)
{
	int ipart = (int)n;
	float fpart = n - (float)ipart;
	int i = intToStr(ipart, res, 0);
	if (afterpoint != 0)
	{
		res[i] = '.';  
		fpart = fpart * pow(10, afterpoint);	
		intToStr((int)fpart, res + i + 1, afterpoint);
	}
}

int main(void)
{
	DDRC |= (1 << 5);
	DDRD |= (1 << 2);
	DDRD |= (1 << 3);
	DDRB |= (1 << 3);
	
	PORTC|= (0 << PC5);
	PORTD|= (1 << PD2);
	PORTD|= (0 << PD3);
	
	usart_init();
	adc_init();
	millis_init();
	lcd_init(LCD_DISP_ON);                 
	lcd_home();                        

	int adcval,p2_count;
	float volvalue;
	float dustdensity;
	char* buffer="0000";
	char res[20],res1[20];
	
	sei();
	
	millis_t msChange = 0;
	millis_t secChange = 0;
	lcd_puts("  Dust Sensor  ");
	lcd_gotoxy(0,1);
	lcd_puts(" System Ready ");
	_delay_ms(500);
	_delay_ms(500);
	_delay_ms(500);
	_delay_ms(500);	
	
	set_timercounter2_mode(3);

	set_timercounter2_prescaler(5);

	set_timercounter2_output_mode(2);
	
	set_timercounter2_compare_value(254);
	
	while(1)
	{
		
		millis_t now = millis();
		// Has it been 500ms since last change for LED1?
		//if(now - msChange >= 10)  //Noise Removal and p1_counter
		//{
			PORTC|= (0 << PC5);
			_delay_us(280);
			adcval=read_adc_channel(4);
			_delay_us(40);
			PORTC|= (1 << PC5);
			//_delay_us(9000);
			_delay_us(9680);
			// Store time
			//msChange = now;	
			if(count8<10)
			{
				count8++;
			}
			run_mean8();
			if (adcval>Threshhold)  //->adcval
			{
				p1count++;
			}
		//}

		// Has it been 1000ms since last change 
		if(now - secChange >= 1000)				// final p1_count/sec
		{
			if(count<32)
			{
				count++;
			}
			run_mean();
			lcd_clrscr();
			itoa(run_avg,buffer,10);
			lcd_puts("Pulse_count:");
			//lcd_gotoxy(7,1);
			lcd_puts(buffer);
			// Store time
			lcd_gotoxy(0,1);
			volvalue=0.0049*adcval;
			ftoa(volvalue, res, 3);
			lcd_puts(" Volt:");
			lcd_puts(res);
			secChange = now;
			int* arr1=func(run_avg);
			for(int c1=0;c1<num_size;c1++)
			{
				usart_data_transmit(arr1[c1]);
			}
			//usart_data_transmit(run_avg);
			usart_string_transmit(",ID: 0001");
			usart_data_transmit(0x0d);
			usart_data_transmit(0x0a);
			p1count=0;
			count8=0;// maybe changed later
		}
		
		//_delay_ms(100);
		
	}
}