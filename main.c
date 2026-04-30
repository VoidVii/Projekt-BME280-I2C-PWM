#include "TM4C123GH6PM.h"       // TI - MCU - CMSIS - Hardware 
#include "system_TM4C123.h"     // TI - Board - CMSIS - start up code
#include "core_cm4.h"           // CMSIS Header __disable_irq(), ...
#include "SysTick.h"
#include "GPIO.h"
#include "ViiROS.h"
#include "I2C.h"

/* Freq. 50MHz */
/*Connecting SDO to GND results in slave address
1110110 (0x76)*/
#define BME280_ADDR 0x76

/* 5.4.1 Register 0xD0 “id”
The “id” register contains the chip identification number chip_id[7:0]V*/
#define BME280_REG_ID 0xD0 /**< Read value is 0x60 */

#define BME280_MSB	0xFA /**< MSB part ut[19:12] of the raw temperature */
#define BME280_LSB	0xFB /**< LSB part ut[11:4] of the raw temperature */
#define BME280_XLSB 0xFC /**< LSB part ut[3:0] of the raw temperature */


/* Test Variable */
volatile uint8_t chipID = 0;
static volatile int8_t temperature0 = 0U; 
static volatile int8_t temperature1 = 0U;
static volatile uint8_t temperature2 = 0U;
uint8_t bufferi[3];

void BME280_Init(void){
	uint8_t data;
	int32_t status_read = I2C_ReadReg(I2C_0, BME280_ADDR, BME280_REG_ID, &data);

	if((status_read) != 0)
	{
		switch(status_read)
		{
			case I2C_E_ARBLST:
				GPIO_WritePin(GPIO_PORTF, GREEN_LED, ON);
				break;
			
			case I2C_E_ADRACK:
				GPIO_WritePin(GPIO_PORTF, RED_LED, ON);
				break;
			
			case I2C_E_DATACK:
				GPIO_WritePin(GPIO_PORTF, BLUE_LED, ON);
				break;
		}
	}
		chipID = data;
}

void BME280_Temp_Init(void){
	uint8_t data2 = 0x27;
	//int32_t status_read = I2C_ReadBurst(0, BME280_ADDR, 0xF5, bufferi, 3);
	int32_t status_read = I2C_WriteReg(0, BME280_ADDR, 0xF4, data2);
	
	if((status_read) != 0)
	{
		switch(status_read)
		{
			case I2C_E_ARBLST:
				GPIO_WritePin(GPIO_PORTF, GREEN_LED, ON);
				break;
			
			case I2C_E_ADRACK:
				GPIO_WritePin(GPIO_PORTF, RED_LED, ON);
				break;
			
			case I2C_E_DATACK:
				GPIO_WritePin(GPIO_PORTF, BLUE_LED, ON);
				break;
		}
	}
}

void BME280_Temp(void){
	
	int32_t status_read = I2C_ReadBurst(0, BME280_ADDR, 0xFA, bufferi, 3);
	
	if((status_read) != 0)
	{
		switch(status_read)
		{
			case I2C_E_ARBLST:
				GPIO_WritePin(GPIO_PORTF, GREEN_LED, ON);
				break;
			
			case I2C_E_ADRACK:
				GPIO_WritePin(GPIO_PORTF, RED_LED, ON);
				break;
			
			case I2C_E_DATACK:
				GPIO_WritePin(GPIO_PORTF, BLUE_LED, ON);
				break;
		}
	}
	temperature0 = bufferi[2];
	temperature1 = bufferi[1];
	temperature2 = bufferi[0];
}


int main(void) {
    SystemCoreClockUpdate();
    __disable_irq();
    /* GPIO - Initialization */
    GPIO_EnablePort(GPIO_PORTF); /**< Enable clock for Port F - AHB */
    I2C_Init(I2C_0, GPIO_PORTB, 2U, 3U);
	
    /* Configure pins as outputs */
    GPIO_ConfigureOutput(GPIO_PORTF, RED_LED);  
    GPIO_ConfigureOutput(GPIO_PORTF, BLUE_LED); 
    GPIO_ConfigureOutput(GPIO_PORTF, GREEN_LED);
	
    /* defined state == 0 */
    GPIO_WritePin(GPIO_PORTF, RED_LED, OFF); 
    GPIO_WritePin(GPIO_PORTF, BLUE_LED, OFF);
    GPIO_WritePin(GPIO_PORTF, GREEN_LED, OFF);
	
    BME280_Init();
		BME280_Temp_Init();
		BME280_Temp();
    __enable_irq();

    while(1) 
    {
			for(uint32_t volatile i = 0; i <= 200; i++){}
			
			
			BME280_Temp();			
    }  
}