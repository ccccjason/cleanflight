/*
 * filter.h
 *
 *  Created on: 24 jun. 2015
 *      Author: borisb
 */

#define YAW_PTERM_CUT_RATIO 2

typedef struct filterStatePt1_s {
	float state;
	float RC;
} filterStatePt1_t;

float filterApplyPt1(float input, filterStatePt1_t *filter, uint8_t f_cut, float dt);
