#include <stdint.h>
#include "stm32f103xb.h"

void PeripheralConfiguration()
{
	/*Enable SPI2 Clock*/
	RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
	/* Enable PORTA and PORTB Clock */
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN;
	/*Turn on alternate function IO clock*/
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;

	/* Configure LCD Slave Select Pin(PORTA15) for SPI2 */
	GPIOA->CRH = (GPIOA->CRH & (~(GPIO_CRH_MODE15_0 | GPIO_CRH_CNF15))) | (GPIO_CRH_MODE15_1);
	/* Configure LCD DC Pin (PORTA12) */
	GPIOA->CRH = (GPIOA->CRH & (~(GPIO_CRH_MODE12_0 | GPIO_CRH_CNF12))) | (GPIO_CRH_MODE12_1);
	/* Configure LCD RST Pin (PORTA11) */
	GPIOA->CRH = (GPIOA->CRH & (~(GPIO_CRH_MODE11_0 | GPIO_CRH_CNF11))) | (GPIO_CRH_MODE11_1);

	/*Disable SPI2 first*/
	SPI2->CR1 &= ~(SPI_CR1_SPE);

	/*Configure alternate function GPIO pins for SCK(PB13), MISO(PB14) and MOSI(PB15)*/
	GPIOB->CRH = (GPIOB->CRH & (~(GPIO_CRH_CNF13_0))) | (GPIO_CRH_MODE13 | GPIO_CRH_CNF13_1); /*PB13:Alternate function output push-pull*/
	GPIOB->CRH = (GPIOB->CRH & (~(GPIO_CRH_MODE14 | GPIO_CRH_CNF14_0))) | (GPIO_CRH_CNF14_1); /*PB14:Input with pull-up/pulldown*/
	GPIOB->CRH = (GPIOB->CRH & (~(GPIO_CRH_CNF15_0))) | (GPIO_CRH_MODE15 | GPIO_CRH_CNF15_1); /*PB15:Alternate function output push-pull*/

	/*Configure SPI2*/
	SPI2->CR1 = !(SPI_CR1_CPHA)		|/* Clock phase = First clock transition */
				!(SPI_CR1_CPOL)		|/* Clock polarity = CLK to 0 when idle */
				 (SPI_CR1_MSTR)		|/* Master selection = Master */
				!(SPI_CR1_BR)		|/* Baud rate[2:0] = fPCLK/2 */
				!(SPI_CR1_SPE)		|/* SPI not enabled !!!Configure before enabling */
				!(SPI_CR1_LSBFIRST)	|/* First bit transmited = MSB */
				 (SPI_CR1_SSI)		|/* Internal slave select = set to 1 */
				 (SPI_CR1_SSM)		|/* Software slave management = enabled */
				!(SPI_CR1_RXONLY)	|/* Receive only = Full duplex */
				!(SPI_CR1_DFF)		|/* Data frame format = 8bit */
				!(SPI_CR1_CRCNEXT)	|/*todo: check it out*//* CRC transfer next = No data phase */
				!(SPI_CR1_CRCEN)	|/* Hardware CRC calculation enable = disabled */
				!(SPI_CR1_BIDIOE)	|/* Output enable on bidimode = not used in this mode*/
				!(SPI_CR1_BIDIMODE); /* Bidirectional data mode enable = 2 line unid */

	SPI2->CR2 = !(SPI_CR2_RXDMAEN)	|/* RX buffer DMA disabled */
				!(SPI_CR2_TXDMAEN)	|/* TX buffer DMA disabled */
				!(SPI_CR2_SSOE)		|/* SS output disabled -> multiple SS pins needed->configured as gpio outputs */
				!(SPI_CR2_ERRIE)	|/*todo: check it out*//* Error interrupt is masked */
				!(SPI_CR2_RXNEIE)	|/*todo: check it out*//* RX buffer not empty interrupt masked */
				!(SPI_CR2_TXEIE);    /*todo: check it out*//* TX buffer empty interrupt enabled */

	/* Enable SPI2 */
	SPI2->CR1 |= SPI_CR1_SPE;

	/*Systick Config*/
#ifdef PORTC13_PROBING
	/*Enable PORTC clock for execution time probing probing*/
	RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;
	/*Set PORTC_PIN13 as output push-pull*/
	GPIOC->CRH = (GPIOC->CRH & (~(GPIO_CRH_MODE13_0 | GPIO_CRH_CNF13))) | (GPIO_CRH_MODE13_1);
#endif
}