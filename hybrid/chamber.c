/*
  This file is a portion of Hsim 0.4
 
  Hsim is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published
  by the Free Software Foundation; either version 2 of the License,
  or (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

 */

/*
 * Combustion Chamber Equations
 *
 * This routine computes that state of the combustion chamber
 * at the current time.  Also generates many of the primary outputs.
 * Includes nozzle calculations.
 *
 * Presumed to run after the tank model has run.
 *
 * DYNAMIC INPUTS:
 *	chamber_pressure	(from previous iteration)
 *
 *	Depends on the outputs of the tank, injector, and fuel
 *	regression models.  The tank model should be run prior to
 *	calling this model. This model explicitly calls the fueld
 *	and injector model as part of its iterative loop.
 *
 * STATIC INPUTS:
 *	nozzle_throat_area
 *	nozzle_exit_area
 *
 * OUTPUTS:
 *	c_star
 *	chamber_pressure	(for this iteration)
 *	thrust
 *	isp
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "state.h"
#include "linkage.h"

extern char *myname;

#define	MAX_ITERATIONS	10000
#define	TOLERANCE	(1e-6)
static int
converged(double old, double new)
{
	double t;

	t = (old - new) / old;
	if (t < 0)
		t = -t;
	return t <= TOLERANCE;
}

void
chamber_init()
{
	chamber_pressure = atmosphere_pressure;
}

void
chamber()
{
	double adjusted_c_star;
	double adjusted_nozzle_cf;
	double old_cp;
	double injector_pressure_drop;
	double core_throat_ratio;
	int counter;
	int hi_set;
	int lo_set;
	double hi_cp;
	double lo_cp;

	if (dry_fire) {
		c_star = 0.;
		chamber_pressure = 0.;
		thrust = 0.;
		isp = 0.;
		return;
	}

	/* iterate to steady-state on cpropep solution */
	counter = 0;
	hi_set = 0;
	lo_set = 0;
	hi_cp = lo_cp = chamber_pressure;
	do {
		if (counter++ >= MAX_ITERATIONS) {
			fprintf(stderr, "%s: failed to converge "
				"CPROPEP solution after %d iterations\n",
				myname, counter-1);
			error_exit(1);
		}

		/* set n2o_flow_rate */
		injector();

		/* set fuel_flow_rate */
		switch (sim_type) {
		    case HYBRID:
			fuel_regression();
			break;
		    case LIQUID:
			liquid_injector();
			break;
		    default:
		    	fprintf(stderr, " SIM TYPE ERROR IN CHAMBER\n");
			exit(1);
		}

		/* get the new c-star */
		cpropep();

		adjusted_c_star = c_star * combustion_efficiency;

		old_cp = chamber_pressure;
		chamber_pressure = adjusted_c_star *
				(n2o_flow_rate + fuel_flow_rate) /
					nozzle_throat_area;

		/*
		 * Chamber pressure must be less than propellant pressure.
		 */
		if (chamber_pressure >= tank_pressure ||
		   (sim_type == LIQUID && chamber_pressure >= 
		    nitrogen_pressure)) {
			if (!hi_set || hi_cp > chamber_pressure)
				hi_cp = chamber_pressure;
			hi_set = 1;
		}

		if (old_cp > chamber_pressure) {
			if (!hi_set || hi_cp > old_cp)
				hi_cp = old_cp;
			hi_set = 1;
		}
		if (chamber_pressure < atmosphere_pressure) {
			if (!lo_set || lo_cp < chamber_pressure)
				lo_cp = chamber_pressure;
			lo_set = 1;
		}
		if (old_cp < chamber_pressure) {
			if (!lo_set || lo_cp < old_cp)
				lo_cp = old_cp;
			lo_set = 1;
		}
		if (hi_set && lo_set)
			chamber_pressure = (hi_cp + lo_cp) / 2.;

		if (!hi_set)
			hi_cp = chamber_pressure = hi_cp + atmosphere_pressure;

		if (!lo_set)
			lo_cp = chamber_pressure = lo_cp - atmosphere_pressure;

	} while (!converged(old_cp, chamber_pressure));

	/*
	 * Hokey formula to deal with assumption of bad nozzles.
	 */
	adjusted_nozzle_cf = 1 + nozzle_cf_correction *
		(nozzle_cf - 1.);

	/* Momentum-derived thrust.  */
	thrust = chamber_pressure * nozzle_throat_area * adjusted_nozzle_cf;

	/* Half-angle correction */
	thrust *= (1. + cos(nozzle_half_angle)) / 2.;

	/* Pressure based thrust.  */
	thrust += (exit_pressure - ambient_air_pressure) * nozzle_exit_area;


	isp = thrust / (n2o_flow_rate + fuel_flow_rate);


	if (chamber_pressure < 2 * atmosphere_pressure) {
		fprintf(stderr, "%s: Chamber pressure < 2 ATM\n",
			myname);
		error_exit(1);
	}

	/* 
	 * Sanity checks follow.
	 * Some are warnings, some are errors.
	 */

	injector_pressure_drop = tank_pressure - chamber_pressure;

	if (injector_pressure_drop <= atmosphere_pressure) {
		fprintf(stderr, "%s: Chamber pressure exceeds"
				" tank pressure\n",
				myname);
		error_exit(1);
	}

	if (injector_pressure_drop < WARN_INJECTOR_RATIO * chamber_pressure) {
		if (injector_pressure_drop <
		    warn_injector_pressure_drop_value) {
			warn_injector_pressure_drop_value =
				injector_pressure_drop;
			warn_injector_pressure_drop_chamber_value =
				chamber_pressure;
		}
		warn_injector_pressure = 1;
	}

	if (exit_pressure < WARN_EXIT_PRESSURE)
		warn_exit_pressure = 1;

	if (n2o_flux > WARN_N2O_FLUX_LIMIT) {
		if (n2o_flux > warn_n2o_flux_value)
			warn_n2o_flux_value = n2o_flux;
		warn_n2o_flux = 1;
	}
	
	if (sim_type == HYBRID) {
		core_throat_ratio = grain_core * grain_core * pi / 4. / 
			nozzle_throat_area;

		if (core_throat_ratio < 1.) {
			fprintf(stderr, "%s: core throat ratio < 1\n",
				myname);
			error_exit(1);
		}

		if (core_throat_ratio < warn_core_throat_ratio_value)
			warn_core_throat_ratio_value = core_throat_ratio;

		if (core_throat_ratio < WARN_CORE_THROAT_RATIO_1)
			warn_core_throat_ratio = 1;
		else if (core_throat_ratio < WARN_CORE_THROAT_RATIO_2 &&
				warn_core_throat_ratio == 0)
			warn_core_throat_ratio = 2;
	}
}
