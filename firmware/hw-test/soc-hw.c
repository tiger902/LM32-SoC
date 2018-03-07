#include "soc-hw.h"

uart_t  *uart0  = (uart_t *)   0x20000000;
timer_t *timer0 = (timer_t *)  0x30000000;
gpio_t  *gpio0  = (gpio_t *)   0x40000000;
spi_t   *spi0   = (spi_t *)    0x50000000;
i2c_t   *i2c0   = (i2c_t *)    0x60000000;
isr_ptr_t isr_table[32];

void tic_isr();

/* GPI0 */

void gpio_config_dir(uint32_t vdir){	// Configures PIN Address
	gpio0 -> dir = vdir;
}

void gpio_write(uint32_t vpins){	    // Writes data on the PIN
	gpio0 -> wr = vpins;
}

uint32_t gpio_read(){	                // Writes data on the PIN
	return gpio0 -> rd;
}

/* IRQ Handling */

void isr_null(){
}

void irq_handler(uint32_t pending){
	int i;
	for(i=0; i<32; i++) {
		if (pending & 0x01) (*isr_table[i])();
		pending >>= 1;
	}
}

void isr_init(){
	int i;
	for(i=0; i<32; i++)
		isr_table[i] = &isr_null;
}

void isr_register(int irq, isr_ptr_t isr){
	isr_table[irq] = isr;
}

void isr_unregister(int irq){
	isr_table[irq] = &isr_null;
}

/* Timer Functions */

void msleep(uint32_t msec){
	uint32_t tcr;
	// Use timer0.1
	timer0->compare1 = (FCPU/1000)*msec;
	timer0->counter1 = 0;
	timer0->tcr1 = TIMER_EN;
	do {
		//halt();
 		tcr = timer0->tcr1;
 	} while ( ! (tcr & TIMER_TRIG) );
}

void nsleep(uint32_t nsec){
	uint32_t tcr;
	// Use timer0.1
	timer0->compare1 = (FCPU/1000000)*nsec;
	timer0->counter1 = 0;
	timer0->tcr1 = TIMER_EN;
	do {
		//halt();
 		tcr = timer0->tcr1;
 	} while ( ! (tcr & TIMER_TRIG) );
}

uint32_t tic_msec;

void tic_isr(){
	tic_msec++;
	timer0->tcr0 = TIMER_EN | TIMER_AR | TIMER_IRQEN;
}

void tic_init(){
	tic_msec = 0;
	// Setup timer0.0
	timer0->compare0 = (FCPU/10000);
	timer0->counter0 = 0;
	timer0->tcr0     = TIMER_EN | TIMER_AR | TIMER_IRQEN;
	isr_register(1, &tic_isr);
}

/* UART Functions */

void uart_init(){
//	uart0->ier = 0x00;  // Interrupt Enable Register
//	uart0->lcr = 0x03;  // Line Control Register: 8N1
//	uart0->mcr = 0x00;  // Modem Control Register
	// Setup Divisor register (FCLK/BAUD)
//	uart0->div = (FCPU/(57600*16));
}

char uart_getchar(){   
	while (! (uart0->ucr & UART_DR)) ;
	return uart0->rxtx;
}

void uart_putchar(char c){
	while (uart0->ucr & UART_BUSY) ;
	uart0->rxtx = c;
}

void uart_putstr(char *str){
	char *c = str;
	while(*c) {
		uart_putchar(*c);
		c++;
	}
}

/* I2C Functions */

void i2c_init(){
	i2c0->prerl =0x00;
	i2c0->prerh =0x50;
	i2c0->ctr =0x80;
}

void i2c_write(char addrDev, char addrReg, char dat){
   i2c0->TxRx=(addrDev<<1 +1);
   i2c0->crsr =0x90;
   while((i2c0->crsr)& I2C_TIP);
   i2c0->TxRx=addrReg;
   i2c0->crsr =0x10;
   while((i2c0->crsr)& I2C_TIP);
   i2c0->TxRx=dat;
   i2c0->crsr =0x10;
   while((i2c0->crsr)& I2C_TIP);
   i2c0->TxRx=dat; 
   i2c0->crsr =0x50;
   while((i2c0->crsr)& I2C_TIP);
}

char i2c_getchar();