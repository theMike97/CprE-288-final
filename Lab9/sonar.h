/*
 * sonar.h
 *
 *  Created on: Mar 29, 2018
 *      Author: mlauderb
 */

#ifndef SONAR_H_
#define SONAR_H_

void sonar_init(void);
void sonar_send_pulse(void);
double sonar_get_distance(void);

#endif /* SONAR_H_ */
