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


/* Bus busy loop */
int32_t I2C_Bus_Busy_Wait(volatile uint32_t *mcs)
{
	/* Wait until I2C controller is not busy (BUSY bit = 0) */
	while((*mcs & I2C_BUSY_BIT) != 0U) {
		/* Busy – wait */
	}
	
	/* Update status of mcs after bus is done */
	uint32_t status_E = *mcs;
	
	/* Check for errors */
	if((status_E & I2C_MCS_ALL_ERRORS) != 0) 
	{
		if((status_E & I2C_MCS_ARBLST) != 0) /**< Abritation failed - Bus conection lost */
		{
			return I2C_E_ARBLST;
		} 
		if((status_E & I2C_MCS_ADRACK) != 0) /**< Address not aknowledged */
		{
			return I2C_E_ADRACK;
		} 
		if((status_E & I2C_MCS_DATACK) != 0)	/**< Data not aknowledged */
		{
			return I2C_E_DATACK;
		}
	}
	else
	{	
		/* Success - No ERROR */
		return I2C_OK;
	}
}

	/**
*@brief Initialize the I2C-BUS
*/
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

/* 
	1. Set slave address (WRITE)
	2. Write register address
	3. START + RUN
	4. Wait BUSY = 0
	5. Check ACK

	6. Write data
	7. RUN + STOP
	8. Wait BUSY = 0
	9. Check ACK
*/ 
int32_t I2C_WriteReg(uint8_t module, uint8_t devSlave, uint8_t reg, uint8_t data)
{
		
	/* Pointer to registers */
	volatile uint32_t *msa; 	/* I2C Master Slave Address */
	volatile uint32_t *mdr;		/* I2C Master Data */
	volatile uint32_t *mcs;		/* I2C Master Control/Status */


	/* Get the base-address of used I2C-module */
	uint32_t base = I2C_Base[module];
	int32_t status = 0U;
	
	/* Address - Pointer - Arithmetik */
	msa = (volatile uint32_t*)(base + I2C_MSA_OFFSET);
	mdr = (volatile uint32_t*)(base + I2C_MDR_OFFSET);
	mcs = (volatile uint32_t*)(base + I2C_MCS_OFFSET);

	/* Send slave-register to be written to */
	
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
	/* If error accured return ERROR  */
	status = I2C_Bus_Busy_Wait(mcs);
	
	if(status != 0) 
	{
		return status;
	}
	
	/* Send send data to slave-register */
	*mdr = data;
	
	/* Master writes 0x5 to the I2CMCS register to stop a transfer */ 
	*mcs = RUN_STOP;
	
	/* Wait until I2C controller is not busy (BUSY bit = 0) and check for error */
	/* If error accured reeturn -1 */
	status = I2C_Bus_Busy_Wait(mcs);
	
	if(status != 0) 
	{
		return status;
	}
	
	/* No errors accured succeded! */
	return 0;
}

/* 
	1. Set slave address (WRITE)
	2. Write register
	3. START + RUN
	4. Wait + check

	5. Set slave address (READ)
	6. START + RUN + STOP
	7. Wait + check

	8. Read data
*/
int32_t I2C_ReadReg(uint8_t module, uint8_t devSlave, uint8_t reg, uint8_t* data)
{
	/* Pointer to registers */
	volatile uint32_t *msa; 	/* I2C Master Slave Address */
	volatile uint32_t *mdr;		/* I2C Master Data */
	volatile uint32_t *mcs;		/* I2C Master Control/Status */
	
	/* Get the base-address of used I2C-module */
	uint32_t base = I2C_Base[module];
	int32_t status = 0U;
	
	/* Address - Pointer - Arithmetik */
	msa = (volatile uint32_t*)(base + I2C_MSA_OFFSET);
	mdr = (volatile uint32_t*)(base + I2C_MDR_OFFSET);
	mcs = (volatile uint32_t*)(base + I2C_MCS_OFFSET);
	
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
	/* If error accured reeturn -1 */
	status = I2C_Bus_Busy_Wait(mcs);
	
	if(status != 0) 
	{
		return status;
	}
	
	/* 	
		Master writes the slave address to the I2CMSA register and configures
		the R/S bit for the desired transfer type.
		
		This address is 7-bits long followed by an eighth bit, which is
		a data direction bit (R/S bit in the I2CMSA register)	
	*/
	*msa = ((devSlave << 1U) | 1U); /**< 0. Bit = R/S bit ==> Write -> 1. */
	
	/* Master writes 0x7 to the I2CMCS register to initiate a transfer */
	*mcs = START_RUN_STOP;
	
	/* Wait until I2C controller is not busy (BUSY bit = 0) and check for error */
	/* If error accured reeturn status */
	status = I2C_Bus_Busy_Wait(mcs);
	
	if(status != 0) 
	{
		return status;
	}
	
	/* write data from I2CNDR register to *data - use only the lower 8 bits */
	*data = (uint8_t)(*mdr & 0xFF);

	return 0;
}


/* 
	1. Write register address 

	2. Switch to READ

	3. START + RUN
	4. Read byte 1 -> ACK

	5. RUN
	6. Read byte 2 -> ACK

	7. RUN + STOP
	8. Read last byte -> NACK
*/