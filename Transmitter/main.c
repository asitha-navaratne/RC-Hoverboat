#include <avr/io.h>
#include <util/delay.h>

void ADC_INIT(void);
void USART_INIT(void);
void ADC_BEGIN(void);
void USART_SEND(unsigned char character);
unsigned char USART_RECEIVE(void);

unsigned char UBRR = 103;							///< Value to be changed to specify the baudrate for the operating clock frequency.
unsigned char transmit[] = {};						///< Array to hold the characters to be transmitted.


int main(void){
	
	ADC_INIT();
	USART_INIT();
	
	unsigned int throttle = 0;						///< Variable to hold the throttle joystick ADC value.
	unsigned int joystickLR = 0;					///< Variable to hold the d-pad joystick X-axis ADC value.
	unsigned int joystickFB = 0;					///< Variable to hold the d-pad joystick Y-axis ADC value.
	
	while(1){
		
		/** Obtain ADC value of throttle joystick */
		
		ADMUX |= (1<<MUX4);						///< Differential input channels ADC0 and ADC1 are enabled.
		ADMUX &= ~((1<<MUX1)|(1<<MUX0));			///< Other ADC channels are disabled.
		ADC_BEGIN();
		throttle = ADC;
		
		/** Obtain ADC value of D-pad joystick in X-axis */
		
		ADMUX |= (1<<MUX1);						///< Single ended input channel ADC2 is enabled.
		ADMUX &= ~((1<<MUX4)|(1<<MUX0));			///< Other ADC channels are disabled.
		ADC_BEGIN();
		joystickLR = ADC;
		
		/** Obtain ADC value of D-pad joystick in Y-axis */
		
		ADMUX |= (1<<MUX1)|(1<<MUX0);				///< Single ended input channel ADC3 is enabled.
		ADMUX &= ~(1<<MUX4);						///< Other ADC channels are disabled.
		ADC_BEGIN();
		joystickFB = ADC;
		
		/** Record status of the X-axis of D-pad joystick for transmission */
		
		if(joystickLR > 800){
			transmit[0] = 'P';										///< If ADC value in joystickLR is larger than 800, transmit[0] is assigned character 'P' for "Port".
		}
		else if(joystickLR <300){
			transmit[0] = 'S';										///< If ADC value in joystickLR is lesser than 300, transmit[0] is assigned character 'S' for "Starboard".
		}
		else if((joystickLR < 800) && (joystickLR >300)){
			transmit[0] = 'A';										///< If ADC value in joystickLR is lesser than 800 and larger than 300, transmit[0] is assigned character 'A' for "Ahead".
		}
		
		/** Record status of the Y-axis of D-pad joystick for transmission */
		
		if(joystickFB > 800){
			transmit[1] = 'F';										///< If ADC value in joystickFB is larger than 800, transmit[1] is assigned character 'F' for "Full Speed".
		}
		else if(joystickFB <300){
			transmit[1] = 'H';										///< If ADC value in joystickFB is lesser than 300, transmit[1] is assigned character 'H' for "Half Speed".
		}
		else if((joystickFB < 800) && (joystickFB >300)){
			transmit[1] = 'M';										///< If ADC value in joystickFB is lesser than 800 and larger than 300, transmit[1] is assigned character 'M' for "Maintain Speed".
		}
		
		/** Record 4-digit ADC value of the throttle joystick for transmission */
		
		transmit[2] = (throttle/1000);
		transmit[3] = ((throttle%1000)/100);
		transmit[4] = ((throttle%100)/10);
		transmit[5] = (throttle%10);
		
		/** Transmit the data package */
		
		for(unsigned char i=0;i<7;i++){
			USART_SEND(transmit[i]);
		}
		USART_SEND(32);									///< A space character is transmitted to seperate consecutive data packets.
	}
}


/*!
 *	@brief Initialize ADC and set prescalar to 128.
 */

void ADC_INIT(void){
	ADMUX = (1<<REFS0);											///< Reference of ADC set to AVCC.
	ADCSRA = ((1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0));		///< ADC enabled and prescalar set to 128 to obtain a clock signal between 50 kHz and 200 kHz.
}

/*!
 *	@brief Enable USART transmission and set baudrate to 9600.
 */

void USART_INIT(void){
	UCSRB |= (1<<TXEN)|(1<<RXEN);					///< USART transmission and receiving enabled.
	UCSRC |= (1<<URSEL)|(1<<UCSZ1)|(1<<UCSZ0);	///< Access UCSRC and set data character size to 8 bits.
	UBRRL = UBRR;
}

/*!
 *	@brief Begin ADC conversion.
 */

void ADC_BEGIN(void){
	ADCSRA |= (1<<ADSC);							///< Start ADC conversion.
	while(ADCSRA&(1<<ADSC));						///< Wait until conversion is complete.
}

/*!
 *	@brief Transmit a character over USART.
 */

void USART_SEND(unsigned char character){
	while(!(UCSRA & (1<<UDRE)));					///< Wait until UDR is empty.
	UDR = character;								///< Load character to be transmitted to UDR.
	while(!(UCSRA & (1<<TXC)));					///< Wait until transmission complete.
}

/*!
 *	@brief Receive a character over USART.
 *	@return Data received over USART.
 */

unsigned char USART_RECEIVE(void){
	while(!(UCSRA & (1<<RXC)));					///< Wait until data receiving is complete.
	return(UDR);
}