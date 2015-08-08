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
 * Fuel Regression Model
 *
 * This module works for solid fuels in the hybrid model.
 *
 * This routine computes the fuel regression rate.
 * The formulas and parameters are taken from HDAS 1.5
 *
 * Fuel parameters are found in the fuel.csv file.
 *
 * DYNAMIC INPUTS:
 *	n2o_flow_rate		(from tank model)
 *	fuel_mass
 *
 * STATIC INPUTS:
 * 	grain length
 *	grain_diameter
 *
 * COMPILED IN CONSTANTS:
 *	fuel_n
 *	fuel_a
 *	fuel_k
 *	fuel_density
 *
 * OUTPUTS:
 *	grain_core		current port diameter
 *	fuel_flow_rate
 * 	n2o_flux
 *	fuel_rb
 */


#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "state.h"
#include "fuel.h"
#include "linkage.h"

extern char *myname;

void
fuel_init()
{
	struct fuel_data_s f;

	if (sim_type != HYBRID) {
		fprintf(stderr, "%s: INTERAL ERROR in fuel_init\n", myname);
		exit(1);
	}

	if (fuel_data(fuel, &f))
		error_exit(1);

    	fuel_n = f.fuel_n;
    	fuel_a = f.fuel_a;
    	fuel_k = f.fuel_k;
    	fuel_density = f.fuel_density;

	/*
	 * calculate the initial fuel mass.
	 */
	fuel_mass = fuel_density * pi / 4. * grain_length *
		(grain_diameter * grain_diameter -
			grain_init_core * grain_init_core);
}

void
fuel_regression()
{
	double a1, a2;

	/*
	 * first compute the area of the grain cross section.
	 */
	a1 = fuel_mass / fuel_density / grain_length;

	/*
	 * Now compute the area of the port
	 */
	a2 = pi/4. * grain_diameter  * grain_diameter - a1;
	if (a2 < 0.) {
		fprintf(stderr, "%s: fuel port shrank!\n",
			myname);
		error_exit(1);
	}

	/*
	 * Finally the diameter of the port
	 */
	grain_core = sqrt(a2 / (pi/4.));

	/*
	 * Oxidizer flux is flow rate divided by port area
	 */
	n2o_flux = n2o_flow_rate / (pi / 4. * grain_core * grain_core);

	/*
	 * Fuel regression rate is from HDAS model
	 */
	fuel_rb = fuel_k * pow(n2o_flux * fuel_a, fuel_n);

	/*
	 * Fuel flow rate is regression rate times the burning area.
	 */
	fuel_flow_rate = fuel_rb * (pi * grain_core) * 
		grain_length * fuel_density;
	
	/* this should *never* happen */
	if (fuel_flow_rate < 0.)
		fuel_flow_rate = 0.;
}

/*
 * Returns true if we still have fuel.
 */
int
fuel_step(double delta_t)
{
	fuel_mass -= fuel_flow_rate * delta_t;

	return fuel_mass >= 0.;
}
