/*
 * BMP280.h
 *
 *  Created on: 23 oct. 2022
 *      Author: yule
 */

#ifndef INC_BMP280_H_
#define INC_BMP280_H_

static const uint16_t addr_BMP280 = 0x77 << 1; //Addrese du capteur

//Variables pour calculer la compensation
typedef uint32_t BMP280_U32_t;
typedef int32_t BMP280_S32_t;
typedef int64_t BMP280_S64_t;

#endif /* INC_BMP280_H_ */
