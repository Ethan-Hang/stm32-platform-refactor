/******************************************************************************
 * @file
 *
 * @par dependencies
 *
 * @author Ethan-Hang
 *
 * @brief
 *
 * Processing flow:
 *
 *
 * @version V1.0 2026--
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

#pragma once
#ifndef __I2C_PORT_H__
#define __I2C_PORT_H__

//******************************** Includes *********************************//
#include "main.h"
#include "i2c.h"
#include "iic_hal.h"

#include "osal_mutex.h"

//******************************** Includes *********************************//

//******************************** Defines **********************************//
typedef enum
{
    CORE_I2C_OK = 0,
    CORE_I2C_ERROR = 1,
    CORE_I2C_BUSY = 2,
    CORE_I2C_TIMEOUT = 3
} core_i2c_status_t;

typedef enum
{
    HARDWARE_I2C = 0,
    SOFTWARE_I2C = 1
} i2c_port_type_t;
    
typedef struct
{
    i2c_port_type_t     core_iic_state;
    iic_bus_t           soft_iic_bus_inst;
    I2C_HandleTypeDef  *hard_iic_handle;
    osal_mutex_handle_t os_mutexid;
} i2c_port_t;

typedef enum
{
    CORE_I2C_BUS_1 = 0,
    CORE_I2C_BUS_2,
    CORE_I2C_BUS_MAX
} core_i2c_bus_t;

//******************************** Defines **********************************//

//******************************* Declaring *********************************//
core_i2c_status_t core_soft_i2c_start       (core_i2c_bus_t   bus);
core_i2c_status_t core_soft_i2c_stop        (core_i2c_bus_t   bus);
core_i2c_status_t core_soft_i2c_send_byte   (core_i2c_bus_t   bus, 
                                                    uint8_t  data);
core_i2c_status_t core_soft_i2c_wait_ack    (core_i2c_bus_t   bus);
core_i2c_status_t core_soft_i2c_receive_byte(core_i2c_bus_t   bus, 
                                                    uint8_t *data);
core_i2c_status_t core_soft_i2c_send_ack    (core_i2c_bus_t   bus);
core_i2c_status_t core_soft_i2c_send_nack   (core_i2c_bus_t   bus);
//******************************* Declaring *********************************//

//******************************* Functions *********************************//

/* Software I2C bus primitives */
#define SENSOR_SOFTWARE_I2C_START() \
                    core_soft_i2c_start(CORE_I2C_BUS_2);

#define SENSOR_SOFTWARE_I2C_STOP() \
                    core_soft_i2c_stop(CORE_I2C_BUS_2);  

#define SENSOR_SOFTWARE_I2C_SEND_BYTE(data) \
                    core_soft_i2c_send_byte(CORE_I2C_BUS_2, data);

#define SENSOR_SOFTWARE_I2C_WAIT_ACK() \
                    core_soft_i2c_wait_ack(CORE_I2C_BUS_2);

#define SENSOR_SOFTWARE_I2C_RECEIVE_BYTE(data) \
                    core_soft_i2c_receive_byte(CORE_I2C_BUS_2, data);                    

#define SENSOR_SOFTWARE_I2C_SEND_ACK() \
                    core_soft_i2c_send_ack(CORE_I2C_BUS_2);

#define SENSOR_SOFTWARE_I2C_SEND_NACK() \
                    core_soft_i2c_send_nack(CORE_I2C_BUS_2);                    

//******************************* Functions *********************************//

#endif /* __I2C_PORT_H__ */

