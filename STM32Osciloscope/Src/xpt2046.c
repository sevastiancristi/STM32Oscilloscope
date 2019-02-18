/*Include lcd header to disable the spi transmit while the lcd is transmiting*/
#include "stm32f103xb.h"
#include "ili9341.h"
#include "xpt2046.h"


#define XPT2046_CFG_START   0b10000000

#define XPT2046_CFG_MUX(v)  ((v&0b111) << (4))

#define XPT2046_CFG_8BIT    0b00001000
#define XPT2046_CFG_12BIT   (0)

#define XPT2046_CFG_SER     0b00000100
#define XPT2046_CFG_DFR     (0)

#define XPT2046_CFG_PWR(v)  ((v&0b11))

#define XPT2046_MUX_Y       0b101
#define XPT2046_MUX_X       0b001

#define XPT2046_MUX_Z1      0b011
#define XPT2046_MUX_Z2      0b100

#define WAIT_TX_CHECK_TIMEOUT(Timeout) while(!(TOUCH_SPI_MODULE->SR & SPI_SR_TXE )){ if(TimeCounter-tickstart_local > Timeout) return 1;}

extern volatile uint32_t TimeCounter;

static inline void XPT2046_SPI_SS_disable()
{
	/*Disable TOUCH SS*/
	TOUCH_CS_PORT->BSRR=TOUCH_CS_PIN;
	/*Enable LCD SS*/
	LCD_CS_PORT->BSRR = (uint32_t)LCD_CS_PIN << 16U;
}

static inline void XPT2046_SPI_SS_enable()
{
	/*First disable LCD SS*/
	LCD_CS_PORT->BSRR = (uint32_t)LCD_CS_PIN;
	/*Enable TOUCH SS*/
	TOUCH_CS_PORT->BSRR=(uint32_t)TOUCH_CS_PIN << 16U;
}

int XPT2046_enable_irq()
{
	const uint32_t tickstart_local = TimeCounter;
	const uint8_t buf[4] = { (XPT2046_CFG_START | XPT2046_CFG_12BIT | XPT2046_CFG_DFR | XPT2046_CFG_MUX(XPT2046_MUX_Y)), 0x00, 0x00, 0x00 };
	/*SPI Enable touch and disable LCD*/
	XPT2046_SPI_SS_enable();
	WAIT_TX_CHECK_TIMEOUT(10)
	for(int i=0;i<4;i++)
	{
		*((__IO uint8_t*)&TOUCH_SPI_MODULE->DR) = buf[i];
		WAIT_TX_CHECK_TIMEOUT(50)
	}
	/*SPI Disable touch and enble LCD*/
	XPT2046_SPI_SS_disable();
	return 0;
}

void internalInterruptConfig()
{
	/*Enable Timer 2 clock*/
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
	/*Timer 2 config used to disable the touch interrupt for a period after it is activated*/
	TIM2->DIER |= TIM_DIER_TIE;
	/*Prescaler*/
	TIM2->PSC = 32000; /*32MHz/32000 = 1000 Hz*/
	/*Auto-reload value*/
	TIM2->ARR = 500; /* 500 ms */
	/*Enable interrupt in the NVIC*/
	NVIC_EnableIRQ(TIM2_IRQn);

	/*Configure Alternate function for PORTB4 interrupt on EXTI4 interrupt line*/
	AFIO->EXTICR[1] |= AFIO_EXTICR2_EXTI4_PB;
	/*Set interrupt trigger on rising Rising Edge for EXTI4 -> PORTB4 */
	EXTI->RTSR |= EXTI_RTSR_TR4;
	/*Clear peripheral pending register to avoid activation of core pending register */
	EXTI->PR |= EXTI_PR_PR4;
	/*Unmask EXTI4 interrupt line*/
	EXTI->IMR |= EXTI_IMR_MR4;
	/*Core NVIC configuration*/
	NVIC_SetPriority(EXTI4_IRQn, 1);
	/*Clear internal pending register to avoid immediate servicing of the interrupt*/
	NVIC_ClearPendingIRQ(EXTI4_IRQn);
	/*Enable interrupt*/
	NVIC_EnableIRQ(EXTI4_IRQn);
}

static inline void maskInterrupt()
{
	/*Mask EXTI4 interrupt line*/
	EXTI->IMR &= ~EXTI_IMR_MR4;
	/*Clear peripheral pending register to avoid activation of core pending register */
	EXTI->PR |= EXTI_PR_PR4;
}

static inline void unmaskInterrupt()
{
	/*Clear peripheral pending register to avoid activation of core pending register */
	EXTI->PR |= EXTI_PR_PR4;
	/*Unmask EXTI4 interrupt line*/
	EXTI->IMR |= EXTI_IMR_MR4;
}

void TIM2_IRQHandler()
{
	/*Disable Timer*/
	TIM2->CR1 &= ~TIM_CR1_CEN;
	/*Unmask IRQ pin interrupt*/
	unmaskInterrupt();
}

void EXTI4_IRQHandler()
{
	maskInterrupt();
#ifdef PORTC13_PROBING
	GPIOC->BSRR = GPIO_BSRR_BR13;
#endif

	/*TODO: DFA GOES HERE*/

#ifdef PORTC13_PROBING
	GPIOC->BSRR = GPIO_BSRR_BS13;
#endif
	/*Keep interrupt masked for 500ms after which timer overflows and activates timer interrupt that unmasks IRQ pin interrupt*/
	TIM2->CR1 |= TIM_CR1_CEN;
}
