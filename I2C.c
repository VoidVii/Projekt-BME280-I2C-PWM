#include "I2C.h"
#include "TM4C123GH6PM.h"       // TI - MCU - CMSIS - Hardware 
#include "GPIO.h"

/* Base-Address: I2C0 - I2C3 -> Datasheet */
uint32_t I2C_Base[] = {
		0x40020000U,	//I2C0
		0x40021000U,	//I2C1
		0x40022000U,	//I2C2
		0x40023000U,	//I2C3
};

/**
*@brief Initialize the I2C-BUS
*/

/* Bus busy loop */
int32_t I2C_Bus_Busy_Wait(volatile uint32_t *mcs)
{
		/* Wait until I2C controller is not busy (BUSY bit = 0) */
		while(*mcs & I2C_BUSY_BIT) {
				/* Busy – wait */
		}
		
				if((*mcs & I2C_MCS_ERROR) != 0)
		{
			/* --> ERROR */    
			return -1;
		}
		else
		{
			/* No ERROR */
			return 0;
		}
}


	

void I2C_Init(uint8_t module, uint8_t port, uint8_t pin_I2CSCL, uint8_t pin_I2CSDA)
{	
		volatile uint32_t *mcr; 	/* I2C Master Configuration */
		volatile uint32_t *mtpr;	/* I2C Master Timer Period*/
		
		/* Get the base-address of used I2C-module */
		uint32_t base = I2C_Base[module];
		
		/* 	Calculate value for I2CMTPR 

				I2CMTPR register represents the number of system clock periods
				in one SCL clock period.	
				TPR = (System Clock/(2*(SCL_LP + SCL_HP)*SCL_CLK))-1;
				TPR = (20MHz/(2*(6+4)*100000))-1;
				TPR = 9
		*/
		uint32_t TPR = (SystemCoreClock / (2U * (SCL_LP + SCL_HP) *SCL_CLK)) - 1U;
	
		/* I2C0 Clock enable */
	  SYSCTL->RCGCI2C |= (1U << module);         // I2C0 Clock enable
	
		/* Wait until change took place */
		while((SYSCTL->PRI2C & (1U << module)) == 0U){}
			
		/* GPIOB - Pin-Mux */
		GPIO_ConfigureI2C(port, pin_I2CSCL, pin_I2CSDA);
		
		/* Address - Pointer - Arithmetik */
		mcr = (volatile uint32_t *)(base + I2C_MCR_OFFSET);
		mtpr = (volatile uint32_t *)(base + I2C_MTPR_OFFSET);
	
		/* Initialize the I2C Master by writing the I2CMCR register with a value of 0x0000.0010. */
    *mcr = I2C_MCR_MASTER; 
	
		/* Write the I2CMTPR register with the value of TPR */
    *mtpr = TPR;    // für 50 MHz SCL = 100 kHz
}

int32_t I2C_WriteReg(uint8_t module, uint8_t devSlave, uint8_t reg, uint8_t data){
		
		/* Pointer to registers */
		volatile uint32_t *msa; 	/* I2C Master Slave Address */
		volatile uint32_t *mdr;		/* I2C Master Data */
		volatile uint32_t *mcs;		/* I2C Master Control/Status */
	
	
		/* Get the base-address of used I2C-module */
		uint32_t base = I2C_Base[module];
		
		/* Address - Pointer - Arithmetik */
		msa = (volatile uint32_t*)(base + I2C_MSA_OFFSET);
		mdr = (volatile uint32_t*)(base + I2C_MDR_OFFSET);
		mcs = (volatile uint32_t*)(base + I2C_MCS_OFFSET);
	
		/* Send slave-register to be written to */
	
		/* 	Send register-address of slave to write to:
				I2C0->MSA = (devSlave << 1);
				I2C0->MDR = reg;
				I2C0->MCS = 0x03; // START + RUN
				while(I2C0->MCS & 0x01);
		*/
		
		/* 	
				Master writes the slave address to the I2CMSA register and configures
				the R/S bit for the desired transfer type.
				
				This address is 7-bits long followed by an eighth bit, which is
				a data direction bit (R/S bit in the I2CMSA register)	
		*/
		*msa = (devSlave << 1U); /**< 0. Bit = R/S bit ==> Write -> 0 */
		
		/* Data is written to the I2CMDR register - what slave-register to read from */
		*mdr = reg;
		
		/* Master writes 0x3 to the I2CMCS register to initiate a transfer */
		*mcs = START_RUN;
		
		/* Wait until I2C controller is not busy (BUSY bit = 0) and check for error */
		I2C_Bus_Busy_Wait(mcs);
		
		/* If error accured reeturn -1 */
		if(I2C_Bus_Busy_Wait(mcs) != 0) 
		{
			return -1;
		}
		/* Send send data to slave-register */
		*mdr = data;
		
		/* Master writes 0x5 to the I2CMCS register to stop a transfer */ 
		*mcs = RUN_STOP;
		
		/* Wait until I2C controller is not busy (BUSY bit = 0) and check for error */
		I2C_Bus_Busy_Wait(mcs);
		
		/* If error accured reeturn -1 */
		if(I2C_Bus_Busy_Wait(mcs) != 0) 
		{
			return -1;
		}
		/* No errors accured succeded! */
		return 0;
}