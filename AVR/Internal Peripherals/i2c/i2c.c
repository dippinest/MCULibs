// =============================================================================================================
//  Header              : i2c.h                                                                                -
//  Created             : 20.02.2022                                                                           -
//  Author              : Denis Chicherov (dippinest)                                                          -
//  Description         : Defining functions to work with the TWI Interface Module (I2C)                      -
// =============================================================================================================

#include "i2c.h"

static volatile uint32_t __i2c_control_timeout__;

void I2C_Initialize(uint32_t i2c_freq_hz_speed)
{
	__i2c_control_timeout__ = (F_CPU / i2c_freq_hz_speed);
	
	i2c_freq_hz_speed = _I2C_GET_HEX_CODE_SPEED_TWPS00(i2c_freq_hz_speed);
	
	TWSR = (0 << TWPS0) | (0 << TWPS1);
	TWBR = i2c_freq_hz_speed;
}

I2C_STATUS I2C_Start()
{
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
	
	uint32_t timeout_counter = 0;
	
	while (!(TWCR & (1 << TWINT)))
	{
		if (timeout_counter > __i2c_control_timeout__)
		{
			return I2C_STATUS_ERROR;
		}
		++timeout_counter;
	}
	
	if ((TWSR & TW_NO_INFO) != TW_START)
	{
		return I2C_STATUS_ERROR;
	}
	
	return I2C_STATUS_NOERROR;
}

I2C_STATUS I2C_Stop()
{
	TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
	
	uint32_t timeout_counter = 0;
		
	while(TWCR & (1 << TWSTO))
	{
		if (timeout_counter > __i2c_control_timeout__)
		{
			return I2C_STATUS_ERROR;
		}
		timeout_counter++;
	}
	
	return I2C_STATUS_NOERROR;
}

I2C_STATUS I2C_Restart()
{
	I2C_Start();
	
	if ((TWSR & TW_NO_INFO) != TW_REP_START)
	{
		return I2C_STATUS_ERROR;
	}
	
	return I2C_STATUS_NOERROR;
}

I2C_STATUS I2C_Send_Byte(uint8_t byte)
{
	TWDR = byte;
	TWCR = (1 << TWINT) | (1 << TWEN);
	
	uint32_t timeout_counter = 0;
	
	while (!(TWCR & (1 << TWINT)))
	{
		if (timeout_counter > __i2c_control_timeout__)
		{
			return I2C_STATUS_ERROR;
		}
		timeout_counter++;
	}
	
	return I2C_STATUS_NOERROR;
}

I2C_STATUS I2C_Read_Byte_With_Confirmation(uint8_t *byte)
{
	TWCR = (1 << TWINT) | (1 << TWEA) | (1 << TWEN);
	
	uint32_t timeout_counter = 0;
	
	while (!(TWCR & (1 << TWINT)))
	{
		if (timeout_counter > __i2c_control_timeout__)
		{
			return I2C_STATUS_ERROR;
		}
		timeout_counter++;
	}
	
	if ((TWSR & TW_NO_INFO) != TW_MR_DATA_ACK)
	{
		return I2C_STATUS_ERROR;
	}
	
	*byte = TWDR;
	
	return I2C_STATUS_NOERROR;
}

I2C_STATUS I2C_Read_Byte_Without_Confirmation(uint8_t *byte)
{
	TWCR = (1 << TWINT) | (1 << TWEN);
	
	uint32_t timeout_counter = 0;
	
	while (!(TWCR & (1 << TWINT)))
	{
		if (timeout_counter > __i2c_control_timeout__)
		{
			return I2C_STATUS_ERROR;
		}
		timeout_counter++;
	}
	
	if ((TWSR & TW_NO_INFO) != TW_MR_DATA_NACK)
	{
		return I2C_STATUS_ERROR;
	}
	
	*byte = TWDR;
	
	return I2C_STATUS_NOERROR;
}

void I2C_Start_Without_I2CStatus_Control()
{
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
	
	while (!(TWCR & (1 << TWINT)));
}

void I2C_Stop_Without_I2CStatus_Control()
{
	TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
	
	while(TWCR & (1 << TWSTO));
}

void I2C_Restart_Without_I2CStatus_Control()
{
	I2C_Start();
}

void I2C_Send_Byte_Without_I2CStatus_Control(uint8_t byte)
{
	TWDR = byte;
	TWCR = (1 << TWINT) | (1 << TWEN);
	
	while (!(TWCR & (1 << TWINT)));
}

uint8_t I2C_Read_Byte_With_Confirmation_Without_I2CStatus_Control()
{
	TWCR = (1 << TWINT) | (1 << TWEA) | (1 << TWEN);
	
	while (!(TWCR & (1 << TWINT)));
	
	return TWDR;
}

uint8_t I2C_Read_Byte_Without_Confirmation_Without_I2CStatus_Control()
{
	TWCR = (1 << TWINT) | (1 << TWEN);
	
	while (!(TWCR & (1 << TWINT)));
	
	return TWDR;
}

bool I2C_Check_Device_By_Address(uint8_t dev_addr)
{
	if (I2C_Start() == I2C_STATUS_ERROR)
	{
		I2C_Stop();
		return false;
	}
		
	I2C_Send_Byte(dev_addr << 1);
		
	if ((TWSR & TW_NO_INFO) != TW_MT_SLA_ACK)
	{
		I2C_Stop();
		return false;
	}
		
	I2C_Stop();
	return true;
}