#include "spi.h"

void SPI_init(void) {
	// Enable clock network to SPI0
	SIM->SCGC5 |= SIM_SCGC5_PORTD_MASK;
	SIM->SCGC4 |= SIM_SCGC4_SPI0_MASK;
	
	// configure output crossbar
	PORTD->PCR[0] = PORT_PCR_MUX(2);  // PCS
	PORTD->PCR[1] = PORT_PCR_MUX(2);  // SCK
	PORTD->PCR[2] = PORT_PCR_MUX(2);  // MOSI
	PORTD->PCR[3] = PORT_PCR_MUX(2);  // MISO
	
	// configure gpio address select
	/* here */
	
	// Config registers, turn on SPI0 as master
	// Enable chip select
	//SPI0->C1 = 0x52;
	// MSTR  = Master/slave mode select
	// SSOE = Slave select output enable : 
		// 0 When MODFEN is 0: 	In master mode, SS pin function is general-purpose I/O (not SPI). 
		//			In slave mode, SS pin function is slave select input.
		//   When MODFEN is 1: 	In master mode, SS pin function is SS input for mode fault.
		//			In slave mode, SS pin function is slave select input.
		// 1 When MODFEN is 0: 	In master mode, SS pin function is general-purpose I/O (not SPI). 
		//			In slave mode, SS pin function is slave select input.
		//   When MODFEN is 1: 	In master mode, SS pin function is automatic SS output. 
		//			In slave mode: SS pin function is slave select input.
	SPI0_C1 = SPI_C1_MSTR_MASK | SPI_C1_SSOE_MASK;   //Set SPI0 to Master & SS pin to auto SS
	//SPI0->C2 = 0x10;
	// When the SPI is configured for slave mode, this bit has no meaning or effect. (The SS pin is the slave select input.) 
	// In master mode, this bit determines how the SS pin is used. 
	// For details, refer to the description of the SSOE bit in the C1 register.
	SPI0_C2 = SPI_C2_MODFEN_MASK;   //Master SS pin acts as slave select output  
	//SPI0_BR[6->4]: 
	//This 3-bit field selects one of eight divisors for the SPI baud rate prescaler. 
	//The input to this prescaler is the bus rate clock (BUSCLK). 
	//The output of this prescaler drives the input of the SPI baud rate divider. 
	//Refer to the description of “SPI Baud Rate Generation” for details.
	//SPI0_BR[3->0]:
	// This 4-bit field selects one of nine divisors for the SPI baud rate divider. 
	// The input to this divider comes from the SPI baud rate prescaler. 
	// Refer to the description of “SPI Baud Rate Generation” for details.
	// SPI0->BR = 0x00;
	SPI0_BR = (SPI_BR_SPPR(0x02) | SPI_BR_SPR(0x02));     
	//Set baud rate prescale divisor to 3(0x2) & set baud rate divisor to 8(0x2) 
}

// status[7] = SPI read buffer full flag
// status[6] = SPI match flag
// status[5] = SPI transmit buffer empty flag
// status[4] = Master mode fault flag
uint8_t SPI_status(void) {
	return SPI0->S;
}

// Write out all characters in supplied buffer to register at address
void SPI_write(uint8_t* p, int size, uint8_t addr) {
	int i;
	// set SPI line to output (BIDROE = 1)
	//SPI0->C2 |= 0x04;
	for (i = 0; i < size; ++i) {
		// poll until empty
		// if buffer not empty, status[5] = 0 (2^5 = 32 = 0x20)
		while ((SPI_status() & 0x20) != 0x20);
		SPI0->D = p[i];
	}
}

// Read size number of characters into buffer p from register at address
void SPI_read(uint8_t* p, int size, uint8_t addr) {
	int i;
	// set SPI line to input (BIDROE = 0)
	//SPI0->C2 &= 0xF7;
	for (i = 0; i < size; ++i) {
		// poll until full
		SPI0->D = 0x00;
		while ((SPI_status() & 0x80) != 0x80);
		p[i] = SPI0->D;
	}
}
