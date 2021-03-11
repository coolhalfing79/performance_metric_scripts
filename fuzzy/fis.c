#include <stdio.h>
#include "fis.h"

float min(float a, float b){
    if(a>b) return b;
    return a;
}
float max(float a, float b){
    if(a>b) return a;
    return b;
}

//QoS = FIS(ETX, Delay, HopCount)
float qos(float etx, float delay, float hc){
    float setx = etx_short(etx);
    float aetx = etx_avg(etx);
    float letx = etx_long(etx);

    float sdly = dly_small(delay);
    float adly = dly_avg(delay);
    float ldly = dly_high(delay);

    float nhc = hc_near(hc);
    float ahc = hc_avg(hc);
    float fhc = hc_far(hc);
    
    // if etx is short and delay is small and hop count is near then QoS is very fast
    float vfqos = min(setx, min(sdly, nhc));
    // if (etx is short and delay is average and hc is near) or ...
    float fqos = max(
        max(
            max(min(setx, min(adly, nhc)),min(setx, min(sdly, ahc))),
            max(min(setx, min(adly, ahc)),min(setx, min(sdly, fhc)))
        ),
        max(
            max(min(setx, min(adly, fhc)),min(aetx, min(sdly, nhc))),
            max(min(aetx, min(sdly, ahc)),min(aetx, min(sdly, fhc)))
        )
    );
    float aqos = max(
        max(
            max(
                max(min(setx, min(ldly, nhc)),min(setx, min(ldly, ahc))),
                max(min(setx, min(ldly, fhc)),min(aetx, min(adly, nhc)))
            ),
            max(
                max(min(aetx, min(adly, ahc)),min(aetx, min(adly, fhc))),
                max(min(letx, min(sdly, nhc)),min(letx, min(sdly, ahc)))
            )
        ),
        min(letx, min(sdly, fhc))
    );
    float sqos = max(
        max(
            max(min(aetx, min(ldly, nhc)),min(aetx, min(ldly, ahc))),
            max(min(aetx, min(ldly, fhc)),min(letx, min(adly, nhc)))
        ),
        max(
            max(min(letx, min(ldly, nhc)),min(letx, min(adly, ahc))),
            max(min(letx, min(ldly, ahc)),min(letx, min(adly, fhc)))
        )
    );
    float vsqos = min(letx, min(ldly, fhc));
    return (vfqos*10 + fqos*30 + aqos*50 + sqos*70 + vsqos*90)/(vfqos + fqos + aqos + sqos + vsqos);
}

//Quality = FIS(QoS, Energy)
float quality(float q, float e){
    float vsqos = qos_very_slow(q);
    float sqos = qos_slow(q);
    float aqos = qos_avg(q);
    float fqos = qos_fast(q);
    float vfqos = qos_very_fast(q);

    float leng = energy_low(e);
    float meng = energy_medium(e);
    float feng = energy_full(e);

    float awf = min(vsqos, leng);
    float bad = max(min(vsqos, meng), min(sqos, leng));
    float deg = max(min(sqos, meng), min(aqos, leng));
    float avg = max(
        max(
            max(min(vsqos, feng), min(sqos, feng)),
            max(min(aqos, meng), min(fqos, leng))
            ),
        min(vfqos, leng)
    );
    float accep = max(min(aqos, feng), min(fqos, meng));
    float good = max(min(fqos, feng), min(vfqos, meng));
    float exc = min(vfqos, feng);

    return (awf*9 + bad*21 + deg*35 + avg*49 + accep*63 + good*77 + exc*91)/(awf + bad + deg + avg + accep + good + exc);
}

//code to test the FIS
int main(){
    float q;
    float qual;
    float etx[] = {7, 4, 15, 15, 7.5};
    float delay[] = {2500, 1700, 2200, 2500, 1500};
    float hc[] = {1, 2, 3, 2, 1};
    float energy[] = {200, 150, 100, 250, 128};
    for(int i = 0; i<5; i++){
        q = qos(etx[i], delay[i], hc[i]);
        qual = quality(q, energy[i]);
        printf("etx\t\tdelay\t\thc\t\tenergy\t\tQoS\t\tQuality\n");
        printf("%f\t%f\t%f\t%f\t%f\t%f\n\n", etx[i], delay[i], hc[i], energy[i], q, qual);
    }
}
