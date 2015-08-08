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
 * This is the main simulator loop.
 */


#include <stdio.h>
#include <stdlib.h>
#include "state.h"
#include "linkage.h"

/*
 * Given the state of tank and fuel grain, find the new steady stae.
 * Returns true if we should keep going.
 */
static int
sim_to_steady_state()
{
	int r;

	r = tank();
	if (n2o_thermo_error == TOO_HOT) {
	    	fprintf(stderr, "N2O tank too hot.\n");
		error_exit(1);
	}
	if (n2o_thermo_error == TOO_COLD) {
	    	fprintf(stderr, "N2O tank too cold at time %.3f.\n",
			 sim_time);
		error_exit(1);
	}

	if (!r)
		return 0;
	chamber();
	return 1;
}

#ifdef NOTYET

/*
 * Routines to save and restore the state.
 *
 * These will (eventually) be used as part of dynamic time-step control.
 */

struct state_s {
	double tank_energy;
	double tank_n2o_mass;
	double fuel_mass;
};

static struct state_s saved;

static void
sim_save_state()
{
	saved.tank_energy = tank_energy;
	saved.tank_n2o_mass = tank_n2o_mass;
	saved.fuel_mass = fuel_mass;
}

static void
sim_restore_state()
{
	tank_energy = saved.tank_energy;
	tank_n2o_mass = saved.tank_n2o_mass;
	fuel_mass = saved.fuel_mass;

	sim_to_steady_state();
}

#endif /* NOTYET */

/*
 * Simulate a time step.
 * Returns 0 on tank empty or out of fuel.
 * Returns 1 on tank not empty and still have fuel.
 */
static int
sim_step(double delta_t)
{
	int r;

	r = tank_step(delta_t);
	switch (sim_type) {
	    case HYBRID:
		r = (r && fuel_step(delta_t));
		break;
	    case LIQUID:
		r = (r &&liquid_step(delta_t));
		break;
	    default:
		fprintf(stderr, " SIM TYPE ERROR IN sim_step\n");
		exit(1);
	}
	
	return r;
}

void
sim_init()
{
	extern char *myname;

	errors_init();
	switch (sim_type) {
	     case HYBRID:
		fuel_init();
		break;
	     case LIQUID:
	     	liquid_init();
		break;
	     default:
	     	fprintf(stderr, "%s: INTERNAL ERROR in sim_init\n", myname);
		exit(1);
	}
	n2o_thermo_init();
	chamber_init();
}

void
sim_loop()
{
	int i;
	int r;

	for (i = 0; ; i++) {
		sim_time = sim_time_step * i;
		r = sim_to_steady_state();
		if (!r || tank_pressure < 2 * atmosphere_pressure)
			break;	/* tank is dry */
		record_data();
		if (!sim_step(sim_time_step))
			break;
	}
}
