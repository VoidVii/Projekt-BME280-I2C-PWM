#ifndef I2C_H_
#define I2C_H_

#include <stdint.h>

/* Array for module selection */
extern uint32_t I2C_Base[];

/*START and STOP Conditions - STOP=1, START=1, and RUN=1 
*/
#define START_RUN				0x03U
#define RUN_STOP				0x05U
#define START_RUN_STOP	0x07U


/* I2C_MCS (Master Control/Status) bits */
#define I2C_BUSY_BIT 		(1U << 0U) /**< Bus is busy */
#define I2C_MCS_ERROR 	(1U << 1U) /**< Some Error accured */
#define I2C_MCS_ADRACK 	(1U << 2U) /**< Address not aknowledged */
#define I2C_MCS_DATACK 	(1U << 3U) /**< Data not aknowledged */
#define I2C_MCS_ARBLST 	(1U << 4U) /**< Abritation failed - Bus conection lost */

/* All Bus-Errors  */
#define I2C_MCS_ALL_ERRORS (I2C_MCS_ARBLST | I2C_MCS_ERROR | I2C_MCS_ADRACK | I2C_MCS_DATACK )

/* Busy Bus return values*/
#define I2C_OK				 0	
#define I2C_E_ARBLST	-1
#define I2C_E_ADRACK	-2	
#define I2C_E_DATACK	-3

/* TPR - Value - Calculation - Datasheet - "normal speed" values*/
#define SCL_LP					6U
#define SCL_HP					4U
#define SCL_CLK 				100000U /**< 100KHz */

/* I2CMCR Vlaue to set I2C Mater */
#define I2C_MCR_MASTER		0x10U

//#define I2C0_Base 0x40020000U

#define I2C_0 0U 
#define I2C_1 1U 
#define I2C_2 2U 
#define I2C_3 3U 

/* **** 16.5 Register Map - Datasheet **** */

/*
Base:

I2C 0: 0x4002.0000
I2C 1: 0x4002.1000
I2C 2: 0x4002.2000
I2C 3: 0x4002.3000 
*/

#define I2C_MSA_OFFSET		0x000U /* I2C Master Slave Address */
#define I2C_MCS_OFFSET		0x004U /* I2C Master Control/Status */
#define I2C_MDR_OFFSET		0x008U /* I2C Master Data */
#define I2C_MTPR_OFFSET		0x00CU /* I2C Master Timer Period */
#define I2C_MCR_OFFSET		0x020U /* I2C Master Configuration */


/* **** **** **** **** **** **** **** **** */



/*==============================================================================
 *                                FUNCTION PROTOTYPES
 *==============================================================================*/

/**
*@brief Initialize the I2C-BUS
*
*@param module 			Used I2C - module (0-3) 
*@param port				GPIO-Port used for I2C
*@param pin_I2CSCL 	GPIO-Pin used for I2C-SCL
*@param pin_I2CSDA	GPIO-Pin used for I2C-SDA
*/
void I2C_Init(uint8_t module, uint8_t port, uint8_t pin_I2CSCL, uint8_t pin_I2CSDA);

/* Bus busy loop + Error check */
int32_t I2C_Bus_Busy_Wait(volatile uint32_t* mcs);

/**
*@brief Write to slave in register with data
*
*@param module 		Used I2C - module (0-3) 
*@param devSlave	Slave device address (7-bit)
*@param reg 			Slave device register to write to
*@param data 			data to write in register
*/
int32_t I2C_WriteReg(uint8_t module, uint8_t devSlave, uint8_t reg, uint8_t data);

/**
*@brief Read from slave data in a register
*
*@param module 		Used I2C - module (0-3)
*@param devSlave	Slave device address (7-bit)
*@param reg 			Slave device register to writ to
*
*@return 					Returns the data from the slave data-register
*/
int32_t I2C_ReadReg(uint8_t module, uint8_t devSlave, uint8_t reg , uint8_t* data);

// void I2C_ReadBurst(uint8_t addr, uint8_t reg, uint8_t* buffer, uint8_t len);


#endif // I2C_H_