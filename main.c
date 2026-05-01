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
#define BME280_CTRL_MEAS	0xF4 /**< pressure and temperature data acquisition options of the device */
#define BME280_MSB_T	0xFA /**< MSB part ut[19:12] of the raw temperature */
#define BME280_LSB_T	0xFB /**< LSB part ut[11:4] of the raw temperature */
#define BME280_XLSB_T 0xFC /**< LSB part ut[3:0] of the raw temperature */


/* Test Variable */
volatile uint8_t chipID = 0;
static volatile int32_t Raw_Temperature = 0U; 
/* 3 Bytes  */
uint8_t bufferi[3];

void BME280_Init(void){
	uint8_t data;
	int32_t status_read = I2C_ReadReg(I2C_0, BME280_ADDR, BME280_REG_ID, &data);
	
	I2C_Status_Handler(status_read);
	
	chipID = data;
}

void BME280_Temp_Init(void){
	/* Register 0xF4 “ctrl_meas”: 101(osrs_t = oversampling ×16) 000(osrs_p) 11(Normal mode)*/
	uint8_t data2 = 0xA3;
	/* */
	int32_t status_read = I2C_WriteReg(0, BME280_ADDR, BME280_CTRL_MEAS, data2);
	
	I2C_Status_Handler(status_read);
}

void BME280_Temp(void){
	uint8_t Temp_Bytes = 3U;
	int32_t status_read = I2C_ReadBurst(0, BME280_ADDR, BME280_MSB_T, bufferi, Temp_Bytes);
	
	I2C_Status_Handler(status_read);
	/* 0xFA ut[19:12] | 0xFB ut[11:4] | 0xFC ut[3:0] */
	Raw_Temperature = (((int32_t)bufferi[0] << 12U) | ((int32_t)bufferi[1] << 4U) | ((int32_t)bufferi[2] >> 4U));
	
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
			for(uint32_t volatile j = 0; j <= 200; j++){}
			BME280_Temp();			
    }  
}