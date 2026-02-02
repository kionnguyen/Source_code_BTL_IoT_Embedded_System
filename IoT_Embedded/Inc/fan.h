/*
 * fan.h
 *
 *  Created on: 22 thg 1, 2026
 *      Author: Admin
 */
#include <stdint.h>

#ifndef FAN_H_
#define FAN_H_

void Fan_On(void);
void Fan_Off(void);
int Fan_Read_Pin(void);

#endif /* FAN_H_ */
