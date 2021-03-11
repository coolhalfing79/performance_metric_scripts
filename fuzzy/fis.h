#ifndef FIS_H
#define FIS_H

float up(float x1, float x2, float x){
    return 100 * (x - x1)/(x2 - x1);
}
#define down(x1, x2, x3) up(x2, x1, x)

float first_t_norm(float a, float b, float x){
    if (x < a) return 100;
    if (x >= a && x <= b) return down(a, b, x);
    return 0;
}

float t_norm(float a, float b, float c, float d, float x){
    if (x >= a && x < b) return up(a, b, x);
    if (x >= b && x <= c) return 100;
    if (x > c && x <= d) return down(c, d, x);
    return 0;
}

float last_t_norm(float c, float d, float x){
    if (x > c && x < d) return up(c, d, x);
    if (x >= d) return 100;
    return 0;
}

//etx
#define etx_short(etx) first_t_norm(3, 6, etx)
#define etx_avg(etx) t_norm(3, 6, 9, 12, etx)
#define etx_long(etx) last_t_norm(9, 12, etx)

//delay
#define dly_small(dly) first_t_norm(600, 1200, dly)
#define dly_avg(dly) t_norm(600, 1200, 1800, 2400, dly)
#define dly_high(dly) last_t_norm(1800, 2400, dly)

// hc
#define hc_near(hc) first_t_norm(1, 2, hc)
#define hc_avg(hc) t_norm(1, 2, 3, 4, hc)
#define hc_far(hc) last_t_norm(3, 4, hc)

//energy
#define energy_low(eng) first_t_norm(51, 102, (eng))
#define energy_medium(eng) t_norm(51, 102, 153, 205, (eng))
#define energy_full(eng) last_t_norm(153, 205, (eng))


/*QoS = FUZZY_COMBINATION(ETX,DELAY)*/

#define QoS_B1 15
#define QoS_B2 25
#define QoS_B3 35
#define QoS_B4 45
#define QoS_B5 55
#define QoS_B6 65
#define QoS_B7 75
#define QoS_B8 85

#define qos_very_fast(x) first_t_norm(QoS_B1, QoS_B2,x)
#define qos_fast(x) t_norm(QoS_B1, QoS_B2, QoS_B3, QoS_B4, x)
#define qos_avg(x) t_norm(QoS_B3, QoS_B4, QoS_B5, QoS_B6, x)
#define qos_slow(x) t_norm(QoS_B5, QoS_B6, QoS_B7, QoS_B8, x)
#define qos_very_slow(x) last_t_norm(QoS_B7, QoS_B8, x)

#endif
