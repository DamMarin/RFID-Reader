/*
				
		Title: 125kHz RFID tag reader. 

		Author: Vassilis Serasidis
		
		http://www.serasidis.gr
		avrsite@yahoo.gr, info@serasidis.gr
		
		Target: ATtiny13 at 9.6 MHz internal RC oscillator or ATtiny85 at 8 MHz internal RC oscillator.

		- Input pin :  PB1 (RFID signal).
		- Output pin:  PB2 (Buzzer) Sounds when an RFID tag is been read successfully.
		- Output pin:  PB4 (TxD) Transmits the RFID tag serial number (2400 bps 8n1).
                            This pin is TTL logic (0 or +5V). Do not connect it direct to RS232 port. Use a voltage shifter such as MAX232. 		
		
		
		Firmware:
		- v2.01 (12 June 2014)
				* Oscillator frequency selection has been made automatic.
		          The only you have to do is to select the micro-controller type from AVR studio 6 (ATtiny13 or ATtiny13A or ATtiny85) and re-compile the source code.
				* The baud rate has been changed from 2400 bps to 9600 bps. Of course you can change it back to 2400 bps if you want.
		- v2.00 (12 February 2013) Firmware was re-written by Vassilis Serasidis for ATtiny85.
		- v1.00 (18 August 2012)   Initial version for ATtiny13. 

		IDE: AVRstudio 6
		
		This code is distributed under GNU GPL V3 (http://www.gnu.org/copyleft/gpl.html).


		 --== Commercial use of this project can be done after the permission of the author (Vassilis Serasidis). ==--
		
		
		There is no warranty for this project. Use it with your own responsibility.
*/


#define F_CPU 8000000UL					
#define BAUD 9600
#define MYUBRR F_CPU/16/BAUD-1
#define PWM_VALUE 32

#define COIL      PB7	// OC0A (ex PB0/OC0A)
#define SIGNAL_IN PB6 	// PCINT6 (ex PB1/PCINT1)
#define BUZZER    PF0	// (ex PB2)
#define PWM_PIN1  PB3	// (ex PB3)

#define TIME1     17
#define TRUE      0
#define FALSE     1
#define UP        2
#define DOWN      0							
#define ONE_BIT_DELAY (1000000/BAUD)

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>	

char readerID[4] = "A01";
volatile unsigned char counter1 = 0, counter2 = 0;
volatile unsigned char signal_in = FALSE, pulseUpDown = UP;
volatile unsigned char receivedBits = 0;
unsigned char dataBuffer[10]; //1 byte ID, 4 bytes RFID Tag serial number, 1 byte line parity, 4 bytes column parity bits.
unsigned char parity,colParity;

//void sendTxD (unsigned char txd);

void PWM_Init (void);
void USART_Init (unsigned int ubrr);
void USART_Transmit (unsigned char data);
unsigned char USART_Receive (void);
unsigned char readTagSerialNumber (void);
void hex2Ascii (void);
void delay_time(unsigned long counter);

/*
void sendTxD (unsigned char txd)
{
	unsigned char bitCounter = 0;

	PORTE &= ~(1 << TXD);
	_delay_us(ONE_BIT_DELAY);
	for(bitCounter=8; bitCounter>0; bitCounter--)
	{
		if (bit_is_clear(txd, 0))
		PORTE &= ~(1 << TXD);
		else
		PORTE |= (1 << TXD);

		_delay_us(ONE_BIT_DELAY);
		txd >>= 1;
	}
	PORTE |= (1 << TXD);
	_delay_us(500);
}
*/

/*******************************************************************
	USART Initialization
*******************************************************************/
void USART_Init( unsigned int ubrr)
{
/*Set baud rate */
UBRR0H = (unsigned char)(ubrr>>8);
UBRR0L = (unsigned char)ubrr;  

/*Enable receiver and transmitter */
UCSR0B = (1<<RXEN0)|(1<<TXEN0);

/* Set frame format: 2stop bit, 8data  */
UCSR0C = (1<<USBS0)|(3<<UCSZ00);
}

/*******************************************************************
	Data Transmission
*******************************************************************/
void USART_Transmit( unsigned char data ) 
{
/* Wait for empty transmit buffer */
while (!( UCSR0A & (1<<UDRE0)));
/* Put data into buffer, sends the data */
UDR0 = data;
}

/*******************************************************************
	Data Reception
*******************************************************************/
unsigned char USART_Receive( void )
{
/* Wait for data to be received */
while (!(UCSR0A & (1<<RXC0)));
/* Get and return received data from buffer */
return UDR0;
}

/*******************************************************************
	Reads the 32-bit RFID tag serial number and
	calculates the parity bits.
*******************************************************************/
unsigned char readTagSerialNumber (void)
{
	unsigned char i,k,m,pulseUpDownBackup,ones,parity;
	/* Bytes 6,7,8 and 9 are used to store the sum of parity bits of the four RFID data columns. */
	dataBuffer[6]=0; /* Clear the parity buffer */
	dataBuffer[7]=0;
	dataBuffer[8]=0;
	dataBuffer[9]=0;
	
	parity = TRUE;
	for(i=0;i<11;i++) /* Repeat 10 times the 5-bit lines of RFID data.*/
	{
		ones = 0; /* Clear the ones buffer. */
		for(k=0;k<4;k++)
		{
			_delay_us(1);
			m=0;
			pulseUpDownBackup = pulseUpDown; //Get a backup the current status of pulseUpDown (see ISR(TIM0_OVF_vect) interrupt for this variable).
			PORTF &= ~(1<<BUZZER); //This command adds an extra delay. Do not remove it.
			dataBuffer[i/2] <<= 1; //Shift one bit to the left the dataBuffer[i/2] byte.
			if(pulseUpDown == UP)  //If level is up (Logic HIGH) then,
			{
				dataBuffer[i/2] |= (1<<0);  //Make logic one the bit 0,
				ones++;                     //Increase the line one's,
				dataBuffer[k+6]++;          //and increase the column one's.
			}
			PORTF &= ~(1<<BUZZER); //This command adds an extra delay. Do not remove it.
			while(m<7) //Delay some time (one bit length).
			{
				_delay_us(1);
				m++;
				if(pulseUpDownBackup != pulseUpDown) //If in the meantime we have a change to the pulseUpDown status, break the delay.
					break;
			}
		}
		//Go get the status of the fifth bit (parity bit).
		_delay_us(1);
		m=0;
		pulseUpDownBackup = pulseUpDown; //Get a new backup the current status of pulseUpDown
		PORTF &= ~(1<<BUZZER); //This command adds an extra delay. Do not remove it.
		if(pulseUpDown == UP)
			ones++;
			
		ones %= 2; //See if parity byte is Even.
		if((ones != 0)&&(i < 10)) //Exclude the 11th 4-bit array (column parity).
			parity = FALSE; //If parity bit is odd, then mark it as FALSE. 
		PORTF &= ~(1<<BUZZER); //This command adds an extra delay. Do not remove it.
		while(m<7) //Delay some time (one bit length).
		{
			_delay_us(1);
			m++;
			if(pulseUpDownBackup != pulseUpDown)
				break;
		}
	}

/* Check the column parity bits (4 columns). */
	if(((dataBuffer[6] % 2) != 0)||
	  ((dataBuffer[7] % 2) != 0)||
	  ((dataBuffer[8] % 2) != 0)||
	  ((dataBuffer[9] % 2) != 0))
		parity = FALSE;

	return(parity); //Return the parity status (TRUE: if parity is OK, FALSE: is parity is not OK).
}


/*******************************************************************
	Interrupts         
*******************************************************************/
ISR(PCINT0_vect) //for pins PCINT0-PCINT7
{
	signal_in = TRUE;
}

ISR(TIMER0_OVF_vect)
{
	if(counter1 < 255)
		counter1++;
		
	if((signal_in == TRUE) && counter1 > 60)
	{
		signal_in = FALSE;
		if(bit_is_set(PINB,SIGNAL_IN))
		{
			PORTB |= (1<<PWM_PIN1);	
			pulseUpDown = UP;
		}
		else
		{
			PORTB &= ~(1<<PWM_PIN1);
			pulseUpDown = DOWN;
		}
			
		counter1 = 0;
	}
	signal_in = FALSE;
}


/*******************************************************************
	PWM function. Produces the 125kHz square wave.         
*******************************************************************/
void PWM_Init (void)
{	
	// Configure timer 1 for CTC mode [Clear Timer on Compare Match (CTC) Mode]

	TCCR0A |= (1<<WGM00 | 1<<WGM01 | 1<<COM0A0 | 1<<COM0B0);  
		// TCCR0A – Timer/Counter Control Register A [ page 77, tinny85 | page 104, mega328p | page 126, mega2560 ]
		// WGM0[1:0]: Waveform Generation Mode (1 1 1 -> Fast PWM)
		// COM0A1:0: Compare Match Output A Mode
		// COM0B1:0: Compare Match Output B Mode 

	TCCR0B |= (1<<WGM02 | 1<<CS00 | 1<<FOC0A | 1<<FOC0B); 
		// TCCR0B – Timer/Counter Control Register B [ page 79, tinny85 | page 107, mega328p | page 129, mega2560 ]
		// FOC0A: Force Output Compare A (Is only active when the WGM bits specify a non-PWM mode) 
		// FOC0B: Force Output Compare B (Is only active when the WGM bits specify a non-PWM mode)
		// CS02:0: Clock Select { clkI/O/(No prescaling) }
		// WGM02: Waveform Generation Mode
		
	TIMSK0 |= (1<<TOIE0);
		// TIMSK0 – Timer/Counter Interrupt Mask Register 
		// TIMSK0 for ATtiny13 | TIMSK for ATtiny85 (page 81) |TIMSK0 for ATmega328p (page 109) | TIMSK0 for mega2560 (page 131) ]
		// TOIE0: Overflow Interrupt Enable
	
	PCMSK0  |= (1<<PCINT6); // PCMSK0  |= (1<<PCINT1); 
		// PCMSK2 – Pin Change Mask Register 0
		// PCMSK2 – Pin Change Mask Register 2
		// PCMSK for attinny85 (page 52) & attinny13  | PCMSK0 for ATMEGA328p (page 74) & ATMEGA2560 (PAGE 113)
		// PCINT[23:16]: Pin Change Enable Mask 23...16
		
	PCICR  |= (1<<PCIE0);  // PCIE0 giati xrisimopoihsa to PCINT6 (PB6)
		// PCICR: Pin Change Interrupt Control Register (page 73, mega328p | page 112, mega2560)
		// PCIE0: Pin Change Interrupt Enable 0 
		// attiny85: GIMSK  |= (1<<PCIE);  GIMSK – General Interrupt Mask Register (page 51) 
	
	
	// Output Compare Registers (OCR0A and OCR0B) (page 65, tinny85) & (page 95, mega328p)
	OCR0A   = PWM_VALUE;            // Set CTC compare value | 38 for ATtiny13 at 9.6 MHz | 32 for ATtiny85 at 8 MHz
	OCR0B   = PWM_VALUE;            // Set CTC compare value | 38 for ATtiny13 at 9.6 MHz | 32 for ATtiny85 at 8 MHz
} 


//===================================================================
// Convert a Hex number into ASCII and send it to serial port.
//===================================================================
void hex2Ascii (void)
{
	uint32_t temp = 0;
	unsigned char i;
	char buffer[12];
	char str[50];
	char str2[5];
	int checksum=0;
	int k;

	for(i=1;i<5;i++) //Get the 32-bit serial number of the Tag.
	{
		temp <<= 8;
		temp |= dataBuffer[i];
	}
	ultoa(temp,buffer,10); //Convert the 32-bit number 'temp' into ASCII string 'buffer'.

	
	// $RFTG;A01;5724697*95 ipologizw kai to dolario
	strcpy (str, "$RFTG;");
	strcat (str, readerID);
	strcat (str, ";");
	strcat (str, buffer);

    for(k = 0; k < strlen(str); k++)
	{
	    checksum ^= str[k];
    }
    
	strcat(str, "*");
    sprintf(str2,"%d", checksum);
    strcat(str, str2);	


	char ATCIPSTART[] = "AT+CIPSTART=\"TCP\",\"192.168.88.232\",884\r\n";
	char ATCIPSEND[] = "AT+CIPSEND=20\r\n";
	char ATCIPCLOSE[] = "AT+CIPCLOSE\r\n";
	
	/*
	char AT[] = "AT\r\n";
	for(i=0;i<strlen(AT);i++){
		USART_Transmit(AT[i]);
	}
	_delay_ms(1000);
	*/
	
	for(i=0;i<strlen(ATCIPSTART);i++){
		USART_Transmit(ATCIPSTART[i]);
		_delay_ms(5);
	}
	
	_delay_ms(1500);
	
	for(i=0;i<strlen(ATCIPSEND);i++){
		USART_Transmit(ATCIPSEND[i]);
		_delay_ms(5);
	}		
	
	_delay_ms(500);
	
	for(i=0;i<strlen(str);i++){
		USART_Transmit(str[i]);
		_delay_ms(5);
	}
		
	_delay_ms(500);
	
	for(i=0;i<strlen(ATCIPCLOSE);i++){
		USART_Transmit(ATCIPCLOSE[i]);
		_delay_ms(5);
	}

	
	
}


// void sendRFTG (unsigned char sentence[50])


/*******************************************************************
Main function.
Initializes PWM (Pulse Width Modulation for 125 kHz)
Reads the 32-bit (4-byte) RFID tag serial number.
If RFID serial number is read OK (parity bits are OK) then enable buzzer for 1 sec.
*******************************************************************/
int main (void)
{
	unsigned char parityStatus = FALSE;
	unsigned char startBitCounter,i;
	unsigned int temp;
	
	DDRB |= (1<<COIL);
	DDRF |= (1<<BUZZER);
	DDRB |= (1<<PWM_PIN1);
	//DDRE |= (1<<TXD);//////////////////////////////////////////////

	USART_Init ( MYUBRR );	// Initialize USART
	PWM_Init();				// Initialize PWM
	
	sei();                  // Enable global interrupts
	
	startBitCounter = 0;
	
	for (;;) //Loop for ever.
	{
		while(pulseUpDown == UP) //Wait here until you find a steady Logic ONE. This represents the 9 start bits of RFID tag.
		{
			startBitCounter++;
			_delay_us(8);
		}
		
		if(startBitCounter >= 12) // If the length of the Logic ONE pulse is ok then, 
		{
			parityStatus = readTagSerialNumber(); //Read the RFID tag serial number and get the parity status.
			if(parityStatus == TRUE) //If parity is ok,
			{
				cli(); // clear interrupts (Stop interrupts).
				temp = 0;
				for(i=1;i<5;i++) //Get the 32-bit serial number of the Tag.
				temp += dataBuffer[i]; //Add to temp the sum of the 4-byte serial number.
				if(temp > 0) //If serial number is 0 then serial number was probably produced by electrical noise. Not from an RFID tag.
				{
					hex2Ascii();         //Go to send the Tag's serial number as ASCII through TxD pin.
					PORTF |= (1<<BUZZER); //Enable buzzer.
					_delay_ms(500);
					PORTF &= ~(1<<BUZZER); //Disable buzzer.
					_delay_ms(500);
				}
				sei();  //Enable the interrupts again.
			}
			startBitCounter = 0;
		}
		if(pulseUpDown == DOWN)
			startBitCounter = 0;
	}
}
