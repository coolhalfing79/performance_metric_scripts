/*
 * fuzzify.h
 *
 *  Created on: Jul 18, 2013
 *      Authors: kamgueu, manu
 */

#ifndef FUZZIFY_H_
#define FUZZIFY_H_

unsigned int qos(unsigned int etx, unsigned int delay, unsigned int hop_count);
unsigned int quality(unsigned int q, unsigned int energy);

#define TRUE 100
#define FALSE 0


/**********************************************
 *      ETX : Expected Transmission Count     *
 **********************************************/

#define ETX_MAX 15

#define ETX_B1 ETX_MAX / 5
#define ETX_B2 ETX_B1 * 2
#define ETX_B3 ETX_B1 * 3
#define ETX_B4 ETX_B1 * 4

#define etx_short(etx, hc) FIRST_T_NORM((ETX_B1)*(hc), (ETX_B2)*(hc), (etx))
#define etx_avg(etx, hc) T_NORM((ETX_B1)*(hc), (ETX_B2)*(hc), (ETX_B3)*(hc), (ETX_B4)*(hc), (etx))
#define etx_long(etx, hc) LAST_T_NORM((ETX_B3)*(hc), (ETX_B4)*(hc), (etx))


/**********************************************
 *             DELAY  METRIC                  *
 **********************************************/

#define DELAY_MAX 3000

#define DLY_B1 DELAY_MAX / 5
#define DLY_B2 DLY_B1 * 2
#define DLY_B3 DLY_B1 * 3
#define DLY_B4 DLY_B1 * 4

#define dly_small(dly, hc) FIRST_T_NORM((DLY_B1)*(hc), (DLY_B2)*(hc), (dly))
#define dly_avg(dly, hc) T_NORM((DLY_B1)*(hc), (DLY_B2)*(hc), (DLY_B3)*(hc), (DLY_B4)*(hc), (dly))
#define dly_high(dly, hc) LAST_T_NORM((DLY_B3)*(hc), (DLY_B4)*(hc), (dly))


/**********************************************
 *              ENERGY  METRIC                *
 **********************************************/

#define ENERGY_MAX 255

#define ENG_B1 ENERGY_MAX / 5
#define ENG_B2 ENG_B1 * 2
#define ENG_B3 ENG_B1 * 3
#define ENG_B4 ENG_B1 * 4

#define energy_low(eng) FIRST_T_NORM(ENG_B1, ENG_B2, (eng))
#define energy_medium(eng) T_NORM(ENG_B1, ENG_B2, ENG_B3, ENG_B4, (eng))
#define energy_full(eng) LAST_T_NORM(ENG_B3, ENG_B4, (eng))


/**********************************************
 *      QoS = FUZZY_COMBINATION(ETX,DELAY)    *
 **********************************************/

#define QoS_B1 15
#define QoS_B2 25
#define QoS_B3 35
#define QoS_B4 45
#define QoS_B5 55
#define QoS_B6 65
#define QoS_B7 75
#define QoS_B8 85


#define qos_very_slow(x) FIRST_T_NORM(QoS_B1, QoS_B2,(x))
#define qos_slow(x) T_NORM(QoS_B1, QoS_B2, QoS_B3, QoS_B4, (x))
#define qos_avg(x) T_NORM(QoS_B3, QoS_B4, QoS_B5, QoS_B6, (x))
#define qos_fast(x) T_NORM(QoS_B5, QoS_B6, QoS_B7, QoS_B8, (x))
#define qos_very_fast(x) LAST_T_NORM(QoS_B7, QoS_B8, (x))


#endif /* FUZZIFY_H_ */
