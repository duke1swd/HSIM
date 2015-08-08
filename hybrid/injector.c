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
 * Injector Model
 *
 * Called by the chamber simulation.
 * Calculates injector flow given tank and chamber conditions.
 *
 * DYNAMIC INPUTS:
 *	n2o_liquid_density
 *	tank_pressure
 *	chamber_pressure
 *
 * STATIC INPUTS:
 *	injector_area
 *	injector_count
 *	injecto_cd
 *
 * OUTPUTS:
 *	n2o_flow_rate
 */


#include <math.h>
#include <stdio.h>
#include "state.h"
#include "linkage.h"

extern char *myname;

void
injector()
{
	double pressure_drop;

	pressure_drop = tank_pressure - chamber_pressure;

	/*
	 * If the pressure drop is too small then
	 * the motor won't work.  This error is caught
	 * in the chamber calculations.  However, in order
	 * to avoid numerical difficulties we pretend the
	 * pressure drop is never negative.
	 */
	if (pressure_drop < 0.)
		pressure_drop = 0.;

	n2o_flow_rate = injector_cd * injector_area * injector_count *
		sqrt(2 * n2o_liquid_density * pressure_drop);
}
