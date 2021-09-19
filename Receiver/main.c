#include <avr/io.h>
#include <util/delay.h>

#define MUX_DDR 	DDRC							///< Data Direction Register of Port of MUX control pins A and B.
#define MUX_PORT 	PORTC							///< Port of MUX control pins A and B.
#define MUX_PINA 	PC0							///< Microcontroller pin of MUX control pin A.
#define MUX_PINB 	PC1							///< Microcontroller pin of MUX control pin B.

void PORT_INIT(void);
void TIMER_INIT(void);
void USART_INIT(void);
unsigned char USART_RECEIVE(void);
void USART_SEND(unsigned char character);

unsigned char UBRR = 103;							///< Value to be changed to specify the baudrate for the operating clock frequency.

int main(void){
	PORT_INIT();
	TIMER_INIT();
	USART_INIT();
	
	unsigned char receive[] = {};						///< Array to hold the received characters.
	unsigned int result;							///< Variable to hold the ADC result value of throttle joystick received over USART.
	unsigned int pwm;							///< Variable to hold the value controlling the width of PWM signal generated at OC1A.

	double val = 0;								///< Variable to hold the value controlling the maximum width of PWM pulse at OC1A.
	
	while(1){
		
		/** Receive data from transmitter */
		
		MUX_PORT &= ~((1<<MUX_PINA)|(1<<MUX_PINB));			///< Set MUX control pins LOW to connect microcontroller to RF module.
		
		for(uint8_t i=0;i!=7;i++){
			receive[i] = USART_RECEIVE();
		}
		
		/** Control PWM signal generated at OC1B */
		
		if(receive[0] == 'A'){
			OCR1B = 2160;						///< If 'A' is received, generate a 7.5% duty cycle PWM signal at OC1B.
		}
		else if(receive[0] == 'P'){
			OCR1B = 1000;						///< If 'P' is received, generate a 5% duty cycle PWM signal at OC1B.
		}
		else if(receive[0] == 'S'){
			OCR1B = 5320;						///< If 'S' is received, generate a 10% duty cycle PWM signal at OC1B.
		}
		
		/** Control maximum width of the PWM signal at OC1A */
		
		if(receive[1] == 'M'){
			val = 1.7;						///< If 'M' is received, set max width of PWM signal at OC1A to approximately 1.7 ms.
		}
		else if(receive[1] == 'F'){
			val = 1.9;						///< If 'F' is received, set max width of PWM signal at OC1A to approximately 2 ms.
		}
		else if(receive[1] == 'H'){
			val = 1.4;						///< If 'H' is received, set max width of PWM signal at OC1A to approximately 1.5 ms
		}
		
		/** Generate PWM signal response to throttle joystick */
		
		result = ((receive[2]*1000)+(receive[3]*100)+(receive[4]*10)+receive[5]);		///< Set received throttle joystick ADC value to variable result.
		pwm = val*result;									///< Convert the variable result to generate the required PWM signal at OC1A.
		OCR1A = pwm;										///< Set OCR1A register to pwm to generate the required signal.
	}
}


/*!
 *	@brief Initialize required I/O Ports.
 */

void PORT_INIT(void){
	DDRD |= (1<<4)|(1<<5);						///< PORTD is set to Output mode to generate the PWM signals at OC1A and OC1B.
	MUX_DDR |= (1<<MUX_PINA)|(1<<MUX_PINB);
}

/*!
 *	@brief Initialize Timer1 to generate a 50 Hz PWM signal to control the motors.
 */

void TIMER_INIT(void){
	TCCR1A = (1<<COM1A1)|(1<<COM1B1);				///< Set Compare Output Mode of channels A and B to clear OC1A and OC1B on compare match. 
	TCCR1B = (1<<WGM13)|(1<<CS11);					///< Set Waveform Generation Mode to PWM, Phase and Frequency Correct with ICR1 as TOP and timer prescalar as 8.
	ICR1 = 20000;							///< Set ICR1 to determine the TOP value of the counter and thereby the frequency of the PWM signal. 
}

/*!
 *	@brief Enable USART transmission and set baudrate to 9600 at 16 MHz.
 */

void USART_INIT(void){
	UCSRB |= (1<<TXEN)|(1<<RXEN);					///< USART transmission and receiving enabled.
	UCSRC |= (1<<URSEL)|(1<<UCSZ1)|(1<<UCSZ0);			///< Access UCSRC and set data character size to 8 bits.
	UBRRL = UBRR;							///< Set baudrate of USART operation.
}

/*!
 *	@brief Receive a character over USART.
 *	@return Data received over USART.
 */

unsigned char USART_RECEIVE(void){
	while(!(UCSRA & (1<<RXC)));					///< Wait until data receiving is complete.
	return(UDR);							///< Return received data stored in UDR register.
}

/*!
 *	@brief Transmit a character over USART.
 */

void USART_SEND(unsigned char character){
	while(!(UCSRA & (1<<UDRE)));					///< Wait until UDR is empty.
	UDR = character;						///< Load character to be transmitted to UDR.
	while(!(UCSRA & (1<<TXC)));					///< Wait until transmission complete.
}
