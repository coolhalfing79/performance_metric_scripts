/*
 * rpl-of-fuzzy.c
 *
 *  Created on: Jul 21, 2013
 *      Author: kamgueu, nataf
 */

#include "fuzzify.h"
#include "sys/battery_charge.h"
#include "net/delay.h"
#include "net/rpl/rpl-private.h"
#include "net/neighbor-info.h"

//#define DEBUG DEBUG_NONE
#define DEBUG DEBUG_PRINT
#include "net/uip-debug.h"
#include "lib/memb.h"

#define HOPCOUNT_MAX 255
#define QUALITY_MAX 100
#define QUALITY_RANK_DIVISOR 10
#define DEFAULT_RANK_INCREMENT  RPL_MIN_HOPRANKINC


static void reset(rpl_dag_t *);
static void parent_state_callback(rpl_parent_t *, int, int);
static rpl_parent_t *best_parent(rpl_parent_t *, rpl_parent_t *);
static rpl_dag_t *best_dag(rpl_dag_t *, rpl_dag_t *);
static rpl_rank_t calculate_rank(rpl_parent_t *, rpl_rank_t);
static void update_metric_container(rpl_instance_t *);

rpl_of_t rpl_of_fuzzy = {
  reset,
  parent_state_callback,
  best_parent,
  best_dag,
  calculate_rank,
  update_metric_container,
  3
};

/* Reject parents that have a higher link metric than the following. */
#define MAX_LINK_METRIC			10

/* Reject parents that have a higher path cost than the following. */
#define MAX_PATH_COST			100

/*
 * The rank must differ more than PARENT_SWITCH_THRESHOLD in order
 * to switch preferred parent.
 */
#define PARENT_SWITCH_THRESHOLD	 2

typedef uint16_t rpl_path_metric_t;

static void
reset(rpl_dag_t *sag)
{
}

static void
parent_state_callback(rpl_parent_t *parent, int known, int etx)
{
}

#if FUZZY
static rpl_path_metric_t
calculate_etx_path_metric(rpl_parent_t *p)
{
  if(p == NULL || (p->mc.obj.etx == 0 && p->rank > ROOT_RANK(p->dag->instance))) {
    return MAX_PATH_COST * RPL_DAG_MC_ETX_DIVISOR;
  } else {
    long etx = p->link_metric;
    etx = (etx * RPL_DAG_MC_ETX_DIVISOR) / NEIGHBOR_INFO_ETX_DIVISOR;
    return p->mc.obj.etx + (uint16_t) etx;
  }
}

static rpl_path_metric_t
calculate_hopcount_path_metric(rpl_parent_t *p)
{
	if(p == NULL || (p->mc.obj.hopcount == 0 && p->rank > ROOT_RANK(p->dag->instance))) {
	   return HOPCOUNT_MAX;
	} else
		return 1 + p->mc.obj.hopcount;
}

static rpl_path_metric_t
calculate_latency_path_metric(rpl_parent_t *p){

	if(p == NULL /*|| (p->mc.obj.latency == 0 && p->rank > ROOT_RANK(p->dag->instance))*/) {
	   return MAX_DELAY;
	} else {
		uint16_t local_delay = p->delay_metric;
		return p->mc.obj.latency + local_delay;
	}

}

static uint8_t
calculate_energy_path_metric(rpl_parent_t *p){

	if (p == NULL) return 0;
	return (battery_charge_value >= p->mc.obj.energy.energy_est) ? p->mc.obj.energy.energy_est : battery_charge_value;
}

static rpl_path_metric_t
calculate_fuzzy_metric(rpl_parent_t *p)
{
  return quality(qos((p->mc.obj.etx)/RPL_DAG_MC_ETX_DIVISOR, p->mc.obj.latency, p->mc.obj.hopcount), p->mc.obj.energy.energy_est);
}
#endif /* FUZZY */

static rpl_dag_t *
best_dag(rpl_dag_t *d1, rpl_dag_t *d2)
{
  if(d1->grounded != d2->grounded) {
    return d1->grounded ? d1 : d2;
  }

  if(d1->preference != d2->preference) {
    return d1->preference > d2->preference ? d1 : d2;
  }

  return d1->rank < d2->rank ? d1 : d2;
}

static rpl_rank_t
calculate_rank(rpl_parent_t *p, rpl_rank_t base_rank)
{
#if FUZZY
	  rpl_rank_t new_rank;
	  rpl_rank_t rank_increase;

	  if(p == NULL) {
	    if(base_rank == 0) {
	      return INFINITE_RANK;
	    }
	    rank_increase = DEFAULT_RANK_INCREMENT;
	  } else {
		/* multiply first, then scale down to avoid truncation effects */
	    rank_increase = p->dag->instance->min_hoprankinc +
	    		((QUALITY_MAX - calculate_fuzzy_metric(p)) * p->dag->instance->min_hoprankinc)/QUALITY_RANK_DIVISOR;
	    if(base_rank == 0) {
	      base_rank = p->rank;
	    }
	  }

	  if(INFINITE_RANK - base_rank < rank_increase) {
	    /* Reached the maximum rank. */
	    new_rank = INFINITE_RANK;
	  } else {
	   /* Calculate the rank based on the new rank information from DIO or
	      stored otherwise. */
	    new_rank = base_rank + rank_increase;
	  }
	  //ANNOTATE("#A R=%u\n",new_rank/p->dag->instance->min_hoprankinc);
	  //ANNOTATE("#A Fix=%u\n",new_rank);
	  return new_rank;
#else
	  return 0;
#endif /* FUZZY */

}

static rpl_parent_t *
best_parent(rpl_parent_t *p1, rpl_parent_t *p2)
{
#if FUZZY
	  rpl_dag_t *dag;
	  rpl_path_metric_t min_diff;
	  rpl_path_metric_t p1_metric;
	  rpl_path_metric_t p2_metric;

	  dag = p1->dag; /* Both parents must be in the same DAG. */

	  min_diff = PARENT_SWITCH_THRESHOLD;

	  /*p1_metric = calculate_fuzzy_metric(p1);
	  p2_metric = calculate_fuzzy_metric(p2);*/
	  p1_metric = calculate_rank(p1,0);
	  p2_metric = calculate_rank(p2,0);


	  /* Maintain stability of the preferred parent in case of similar ranks. */
	  if(p1 == dag->preferred_parent || p2 == dag->preferred_parent) {
	    if(p1_metric < p2_metric + min_diff &&
	       p1_metric > p2_metric - min_diff) {
	      PRINTF("RPL: MRHOF hysteresis: %u <= %u <= %u\n",
	             p2_metric - min_diff,
	             p1_metric,
	             p2_metric + min_diff);
	      return dag->preferred_parent;
	    }
	  }

	  return  p1_metric < p2_metric ? p1 : p2;
#else
	  return NULL;
#endif /* FUZZY */
}


static void
update_metric_container(rpl_instance_t *instance)
{
#if FUZZY
	  rpl_dag_t *dag;

	  instance->mc.flags = RPL_DAG_MC_FLAG_P;
	  instance->mc.aggr = RPL_DAG_MC_AGGR_ADDITIVE;
	  instance->mc.prec = 0;
	  instance->mc.type = RPL_DAG_MC_FUZZY;
	  instance->mc.length = 8;
	  /*instance->mc.length = sizeof(instance->mc.obj);*/

	  dag = instance->current_dag;
	  if (!dag->joined) {
	    /* We should probably do something here */
	    return;
	  }

	  /* If this node is the root, then initialize metrics values accordingly */

	  if(dag->rank == ROOT_RANK(instance)) {
		instance->mc.obj.energy.energy_est = MAX_ENERGY;
		instance->mc.obj.energy.flags = RPL_DAG_MC_ENERGY_TYPE_MAINS << RPL_DAG_MC_ENERGY_TYPE;

	    instance->mc.obj.etx = 0;
	    instance->mc.obj.hopcount = 0;
	    instance->mc.obj.latency = 0;
	  } else {

		  	rpl_path_metric_t energy, etx, hopcount, latency;

			instance->mc.obj.energy.energy_est = calculate_energy_path_metric(dag->preferred_parent);
			energy = (uint16_t) instance->mc.obj.energy.energy_est;
			instance->mc.obj.energy.flags = RPL_DAG_MC_ENERGY_TYPE_BATTERY << RPL_DAG_MC_ENERGY_TYPE;

		    etx = instance->mc.obj.etx = calculate_etx_path_metric(dag->preferred_parent);
		    hopcount = instance->mc.obj.hopcount = calculate_hopcount_path_metric(dag->preferred_parent);
		    latency = instance->mc.obj.latency = calculate_latency_path_metric(dag->preferred_parent);

			/* ANNOTATE("#A Q=%u\n", quality(qos(etx/RPL_DAG_MC_ETX_DIVISOR, latency, hopcount), energy));
			ANNOTATE("#A F=%u\n", calculate_rank(dag->preferred_parent,0));
			ANNOTATE("#A etx=%u\t D=%u\t H=%u E=%u\n", etx/RPL_DAG_MC_ETX_DIVISOR, latency, hopcount, energy); */
			ANNOTATE("#A dec=%lu\n", battery_charge_dec_value);
	  }
#endif  /* FUZZY */
}

