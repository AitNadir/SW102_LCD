
#ifndef INCLUDE_BLE_SERVICES_H_
#define INCLUDE_BLE_SERVICES_H_

#include "state.h"

#define BLE_RESET                      0
#define BLE_SET_MAXSPEED               1
#define BLE_SET_WHEEL                  2
#define BLE_SET_BATTERY_CURRENT        3
#define BLE_SET_VOLTAGE_LOWCUTOFF      4
#define BLE_SET_PACK_RESISTANCE        5
#define BLE_SET_VOLTAGE_FULLRESET      6
#define BLE_SET_TOTAL_CAPACITY         7
#define BLE_SET_MAXSPEED_STREET        8
#define BLE_SET_MOTOR_CURRENT          9
#define BLE_SET_MOTOR_ACCELERATION     10
#define BLE_SET_MOTOR_DECELERATION     11
#define BLE_SET_AUTO_POWEROFF          12
#define BLE_SET_VOLTAGE_CALIBRATE      13
#define BLE_SET_AUTO_RESET             14
#define BLE_OPTIONS                    15

void ble_init(void);
void send_bluetooth1(rt_vars_t *rt_vars);
void send_bluetooth2(rt_vars_t *rt_vars);
//void send_bluetooth3(rt_vars_t *rt_vars);
//void send_bluetooth4(rt_vars_t *rt_vars);

#endif /* INCLUDE_BLE_SERVICES_H_ */
