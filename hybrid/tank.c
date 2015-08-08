/*
  This file is a portion of Hsim 0.2
 
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
 * Nitrous Tank Model
 *
 * This routine uses thermo data to predict the state of Nitrous in
 * the tank as a function of tank energy and nitrous mass.
 *
 * DYNAMIC INPUTS:
 *	tank_energy
 *	tank_n2o_mass
 *
 * STATIC INPUTS:
 *	tank_volume
 *	vent_area
 *	vent_cd
 *
 * RETURNS:
 *	If n2o_thermo_error is set then all results are invalid.
 *	TRUE if there is still liquid N2O in the tank.
 *	FALSE ifthere is no liquid N2O in the tank.
 *
 * OUTPUTS:
 *	tank_pressure
 *	tank_temperature
 *	n2o_liquid_mass
 *	n2o_liquid_density
 *	tank_center_of_gravity
 *	n2o_thermo_error.
 */

/*
 * Notes on tank solutions.
 *
 * Assumption: tank is in equilibrium between vapor and liquid phase.
 *  We know the energy, mass, and volume.
 *  Need to solve for the temperature, pressure, and fraction liquid.
 *
 * Thermo data
 *  Vapor phase
 *    Density(temp)
 *    Energy/mol(temp)
 *    Pressure(temp) (same as vapor pressure of liquid)
 *  Liquid phase
 *    Density(temp)
 *    Energy/mol(temp)
 *
 * Solution strategy
 *  1) Assume we know the temperature (use old temp)
 *  2) Calculate the density of liquid and gas
 *  3) Using the volume, solve for the liquid fraction
 *  4) Calculate the energy of liquid and gas.
 *  5) If the total tank energy is too high, reduce temperature, else increase
 *  iterate
 */


#include <stdio.h>
#include <stdlib.h>
#include "state.h"
#include "linkage.h"

#define	MAX_ITERATIONS	10000
#define	TOLERANCE	(1e-6)

extern char *myname;


/***********************\
*                       *
* TANK STATE CALCUATION *
*                       *
\***********************/

/*
 * This routine takes a temperature and calculates
 * the state and the energy.
 *
 * The calculated energy is returned.
 *
 * These globals are set as side effects:
 *	n2o_liquid_density
 *	n2o_liquid_mass
 *	tank_pressure
 *	n2o_thermo_error
 */

static double
tank_thermo()
{
	double calc_tank_energy;
	double n2o_vapor_density;
	double n2o_vapor_mass;
	double liquid_fraction;
	double average_density;

	average_density = tank_n2o_mass / tank_volume; 

	n2o_vapor_density = vapor_density(tank_temperature);
	n2o_liquid_density = liquid_density(tank_temperature);

	liquid_fraction = 
		(1./average_density - 1./n2o_vapor_density) /
		 (1./n2o_liquid_density - 1./n2o_vapor_density);

	n2o_liquid_mass = liquid_fraction * tank_n2o_mass;
	n2o_vapor_mass = tank_n2o_mass - n2o_liquid_mass;
/*xxx*/if(n2o_liquid_mass < 0){n2o_liquid_mass = 0;n2o_vapor_mass=tank_n2o_mass;}
/*xxx*/if(n2o_vapor_mass < 0){n2o_vapor_mass = 0;n2o_liquid_mass=tank_n2o_mass;}

	calc_tank_energy = 
		n2o_liquid_mass *
		liquid_energy(tank_temperature);

	calc_tank_energy += 
		n2o_vapor_mass *
		vapor_energy(tank_temperature);

	tank_pressure = saturation_pressure(tank_temperature);

	return calc_tank_energy;
}


/*
 * This routine iterates until the tank state converges.
 */
static void
tank_state()
{
	int i;
	double t;
	double calc_tank_energy;

	int hi_temp_set, lo_temp_set;
	double hi_temp, lo_temp;


	hi_temp_set = 0;
	lo_temp_set = 0;

	/*
	 * Convergence Loop.
	 */
	for (i = 0; i < MAX_ITERATIONS; i++) {

		n2o_thermo_error = 0;
		calc_tank_energy = tank_thermo();

		if (n2o_thermo_error)
			return;
		
		/*
		 * Have we converged?
		 */
		t = calc_tank_energy - tank_energy;
		if (t < 0)
			t = -t;
		if (t / tank_energy < TOLERANCE)
			return;	/* Done! */

		/*
		 * No, guess a new temperature.
		 */
		if (calc_tank_energy > tank_energy) {
			hi_temp = tank_temperature;
			hi_temp_set = 1;
			if (!lo_temp_set) {
				tank_temperature -= 1;
				continue;
			}
		} else {
			lo_temp = tank_temperature;
			lo_temp_set = 1;
			if (!hi_temp_set) {
				tank_temperature += 1;
				continue;
			}
		}
		tank_temperature = (hi_temp + lo_temp) * .5;
	}

	/*
	 * Falls out of the loop if we fail to converge.
	 */

	fprintf(stderr, "%s: failed to converge "
		"tank temperature solution after %d iterations\n",
		myname, i);
	error_exit(1);
}

/*
 * Calculate the tank state, check for errors, and then run the vent model.
 */
int
tank()
{
	tank_state();
	if (n2o_thermo_error)
		return 0;

	if (n2o_liquid_mass <= 0.)
		return 0;

	vent();
	return 1;
}

/*
 * TIME STEP
 * Returns 0 on tank empty
 * Returns 1 on tank not empty.
 */
int
tank_step(double delta_t)
{
	int r;
	double n2o_loss;

	/*
	 * Loss of vapor due to vent
	 */
	n2o_loss = n2o_vent_rate * delta_t;
	tank_n2o_mass -= n2o_loss;
	tank_energy -= n2o_loss * vapor_energy(tank_temperature);
	tank_energy -= tank_pressure * n2o_loss /
		vapor_density(tank_temperature);

	/*
	 * Loss of liquid due to injector
	 */
	r = 1;
	n2o_loss = n2o_flow_rate * delta_t;
	if (n2o_loss > n2o_liquid_mass)
		r = 0;

	tank_n2o_mass -= n2o_loss;
	tank_energy -= n2o_loss * liquid_energy(tank_temperature);
	tank_energy -= tank_pressure * n2o_loss /
		liquid_density(tank_temperature);
	
	return r;
}

/*
 * Compute the boil-off during fill.
 *
 * Take a unit volume of supply N2O.
 * Expand it at constant enthalpy until the pressure drops to flight
 * tank pressure.
 * Find the volume of liquid.
 * Scale up by the amount ofliquid in the tank.
 * Find the amount of gas.  Subtract the ullage.
 * The remaining gas is the boil-off.
 */
void
tank_boil_off(double filltemp)
{
	double fill_energy_density;	/* J/kg */
	double tank_led;		/* J/kg, liquid in flight tank */
	double tank_ved;		/* J/kg, vapor in flight tank */
	double r;			/* mass fraction of liquid */
	double vapor_mass;		/* vapor boiled off */

	fill_energy_density = liquid_energy(filltemp);
	tank_led = liquid_energy(tank_temperature);
	tank_ved = vapor_energy(tank_temperature);
	r = (fill_energy_density - tank_ved) / (tank_led - tank_ved);

	vapor_mass = n2o_liquid_mass * (1 -r) / r;
	vent_mass = vapor_mass - (tank_n2o_mass - n2o_liquid_mass);
}
