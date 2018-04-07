/*
 * IR.h
 *
 * Mike
 *
 */

#ifndef IR_H_
#define IR_H_

#include <inc/tm4c123gh6pm.h>
#include <stdint.h>

void IR_init(void);
unsigned IR_read(void);
double IR_get_distance(void);

#endif /* IR_H_ */
