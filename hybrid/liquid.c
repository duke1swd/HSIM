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
 * These routines look up or calculate the thermodynamic
 * properties of liquid fuels and nitrogen pressurant gas.
 *
 * All of these routine assume the nitrogen is always gasseous and
 * the liquid fuel is always liquid.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "liquid_fuel.h"
#include "state.h"
#include "linkage.h"
#include "rsim.h"

extern char *myname;

double lf_pvgamma;

void
liquid_init()
{
	int r;
	struct liquid_fuel_data_s lfd;
	extern char *myname;

	if (sim_type != LIQUID) {
		fprintf(stderr, "%s: INTERAL ERROR in liquid_init\n", myname);
		exit(1);
	}

	r = liquid_fuel_data(fuel, &lfd);
	if (r != 0) {
		fprintf(stderr, "%s: unknown fuel %s\n",
			myname, fuel);
		exit(1);
	}

	/*
	 * Use the density to make sure we have both mass and volume.
	 */
	lfueldensity = lfd.fuel_density;
	if (lfuelvolume == 0.)
		lfuelvolume = lfuelmass / lfueldensity;
	else
		lfuelmass = lfuelvolume * lfueldensity;

	nitrogen_pressure = nitrogen_pressure_initial;

	nitrogen_gamma = 1.41;

	lf_pvgamma = nitrogen_pressure *
		pow(lfueltankvolume - lfuelvolume, nitrogen_gamma);
}

/*
 * Called after the liquid fuel mass has been reduced during simulation.
 * Updates the liquid fuel volume and the nitrogen pressure.
 *
 * Works under the assumption that nitrogen is a gas for which P * V**gamma
 * is a constant, and gamma is 1.4.
 */
static void
liquid_fuel_used()
{
	// update volume
	lfuelvolume = lfuelmass / lfueldensity;

	// update pressure
	nitrogen_pressure = lf_pvgamma /
		pow(lfueltankvolume - lfuelvolume, nitrogen_gamma);
}

/*
 * Called for each time step.
 *
 * Returns true if we still have fuel.
 *
 * NOTE Abitrary decision: stop simulating when nitrogen pressure
 * reaches 1.5 atm absolute.
 */
int
liquid_step(double delta_t)
{
	lfuelmass -= fuel_flow_rate * delta_t;
	liquid_fuel_used();

	return nitrogen_pressure >= 1.5 * atmosphere_pressure && lfuelmass >= 0.;
}
