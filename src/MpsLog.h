/*
 * MpsLog.h
 *
 *  Created on: Apr 13, 2019
 *      Author: dorus
 */

#ifndef MPS_LOG_H_
#define MPS_LOG_H_

#include <stdio.h>

#define MPS_LOG(message, ...) printf("[MPS] "); printf(message, ##__VA_ARGS__); printf("\r\n");

#endif /* MPS_LOG_H_ */
