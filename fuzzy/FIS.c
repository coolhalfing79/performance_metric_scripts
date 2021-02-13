/*
Anirudha Chari
*/

#include "FIS.h"

unsigned int DOWN(unsigned int x1, unsigned int x2, unsigned int X) {
	return (TRUE * ((long)(X) - (long)(x2)))/((long)(x1) - (long)(x2));
}

unsigned int UP(unsigned int x1, unsigned int x2, unsigned int X) {
	return DOWN(x2, x1, X);
}

unsigned int FIRST_T_NORM(unsigned int b1, unsigned int b2, unsigned int px) {
	if (px <= b1) return TRUE;
	if (px > b1 && px <= b2) return DOWN(b1, b2, px);
	return FALSE;
}

unsigned int LAST_T_NORM(unsigned int b1, unsigned int b2, unsigned int px) {
	if (px <= b1) return FALSE;
	if (px > b1 && px <= b2) return UP(b1, b2, px);
	return TRUE;
}

unsigned int T_NORM(unsigned int b1, unsigned int b2, unsigned int b3, unsigned int b4, unsigned int px) {
	if (px <= b1) return FALSE;
	if (px > b1 && px <= b2) return UP(b1, b2, px);
	if (px > b2 && px <= b3) return TRUE;
	if (px > b3 && px <= b4) return DOWN(b3, b4, px);
	return FALSE;
}


#define min(x,y) (x<y?x:y)
#define max(x,y) (x<y?y:x)

/**********************************************
 *      QoS = FUZZY_COMBINATION(ETX,DELAY)    *
 **********************************************/

unsigned int qos(unsigned int e, unsigned int d, unsigned int hc) {
	unsigned int letx = etx_long(e);
	unsigned int aetx = etx_avg(e);
	unsigned int setx = etx_short(e);
	unsigned int hdly = dly_high(d);
	unsigned int adly = dly_avg(d);
	unsigned int sdly = dly_small(d);
    unsigned int nhc = hc_near(hc);
    unsigned int ahc = hc_avg(hc);
    unsigned int fhc = hc_far(hc);
	unsigned int vfqos = 0, fqos = 0, aqos = 0, sqos = 0, vsqos = 0;

    if (setx && sdly && nhc) vfqos = min(setx, min(sdly, nhc)); 
    if ((setx && adly && nhc) ||
        (setx && sdly && ahc) ||
        (setx && adly && ahc) ||
        (setx && sdly && fhc) ||
        (setx && adly && fhc) ||
        (aetx && sdly && nhc) ||
        (aetx && sdly && ahc) ||
        (aetx && sdly && fhc)) {
		fqos = max(
				max(
					max(min(setx, min(adly, nhc)),min(setx, min(sdly, ahc))),
					max(min(setx, min(adly, ahc)),min(setx, min(sdly, fhc)))
				),
				max(
					max(min(setx, min(adly, fhc)),min(aetx, min(sdly, nhc))),
					max(min(aetx, min(sdly, ahc)),min(aetx, min(sdly, fhc)))
				)
		);
	}
	if ((setx && hdly && nhc) ||
		(setx && hdly && ahc) ||
		(setx && hdly && fhc) ||
		(aetx && adly && nhc) ||
		(aetx && adly && ahc) ||
		(aetx && adly && fhc) ||
		(letx && sdly && nhc) ||
		(letx && sdly && ahc) ||
		(letx && sdly && fhc)) {
		aqos = max(
				max(
					max(
						max(min(setx, min(hdly, nhc)),min(setx, min(hdly, ahc))),
						max(min(setx, min(hdly, fhc)),min(aetx, min(adly, nhc)))
					),
					max(
						max(min(aetx, min(adly, ahc)),min(aetx, min(adly, fhc))),
						max(min(letx, min(sdly, nhc)),min(letx, min(sdly, ahc)))
					)
				),
				min(letx, min(sdly, fhc))
			);
	}
	if ((aetx && hdly && nhc) ||
		(aetx && hdly && ahc) ||
		(aetx && hdly && fhc) ||
		(letx && adly && nhc) ||
		(letx && hdly && nhc) ||
		(letx && adly && ahc) ||
		(letx && hdly && ahc) ||
		(letx && adly && fhc)) {
		sqos = max(
				max(
					max(min(aetx, min(hdly, nhc)),min(aetx, min(hdly, ahc))),
					max(min(aetx, min(hdly, fhc)),min(letx, min(adly, nhc)))
				),
				max(
					max(min(letx, min(hdly, nhc)),min(letx, min(adly, ahc))),
					max(min(letx, min(hdly, ahc)),min(letx, min(adly, fhc)))
				)
		);
	}
	if (letx && hdly && fhc) {
		vsqos = min(letx, min(hdly, fhc));
	}
	

	return (vfqos*10 + fqos*30 + aqos*50 + sqos*70 + vsqos*90)/(vfqos + fqos + aqos + sqos + vsqos);
}


/***************************************************
 *      Quality = FUZZY_COMBINATION(QoS,ENERGY)    *
 ***************************************************/

unsigned int quality(unsigned int q, unsigned int e){
	unsigned int vsqos = qos_very_slow(q);
	unsigned int sqos = qos_slow(q);
	unsigned int aqos = qos_avg(q);
	unsigned int fqos = qos_fast(q);
	unsigned int vfqos = qos_very_fast(q);

	unsigned int leng = energy_low(e);
	unsigned int meng = energy_medium(e);
	unsigned int feng = energy_full(e);

	unsigned int awf = 0, bad = 0, deg = 0, avg = 0, accep = 0, good = 0, exc = 0;

	if (vsqos && leng) awf = min(vsqos, leng);
	if (vsqos && meng) bad = min(vsqos, meng) < bad ? bad : min(vsqos, meng);
	if (vsqos && feng) avg = min(vsqos, feng) < avg ? avg : min(vsqos, feng);
	if (sqos && leng) bad = min(sqos, leng) < bad ? bad : min(sqos, leng);
	if (sqos && meng) deg = min(sqos, meng) < deg ? deg : min(sqos, meng);
	if (sqos && feng) avg = min(sqos, feng) < avg ? avg : min(sqos, feng);
	if (aqos && leng) bad = min(aqos, leng) < bad ? bad : min(aqos, leng);
	if (aqos && meng) avg = min(aqos, meng) < avg ? avg : min(aqos, meng);
	if (aqos && feng) accep = min(aqos, feng) < accep ? accep : min(aqos, feng);
	if (fqos && leng) deg = min(fqos, leng) < deg ? deg : min(fqos, leng);
	if (fqos && meng) accep = min(fqos, meng) < accep ? accep : min(fqos, meng);
	if (fqos && feng) good = min(fqos, feng) < good ? good : min(fqos, feng);
	if (vfqos && leng) avg = min(vfqos, leng) < avg ? avg : min(vfqos, leng);
	if (vfqos && meng) good = min(vfqos, meng) < good ? good : min(vfqos, meng);
	if (vfqos && feng) exc = min(vfqos, feng) < exc ? exc : min(vfqos, feng);

	return (awf*8 + bad*21 + deg*35 + avg*49 + accep*63 + good*77 + exc*91)/(awf + bad + deg + avg + accep + good + exc);
}