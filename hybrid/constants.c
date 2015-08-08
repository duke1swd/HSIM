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
 * Define some constants.
 */

#include <stdio.h>
#include "state.h"
#include "linkage.h"


void
constants_init()
{
	pi = 3.14159265358979323844;
	atmosphere_pressure = 101325.;
	n2o_mols_per_kg = 1000. / 44.01;
	ideal_gas_constant = 8.314472;	/* QQQ Needs to be checked for units */
	earth_gravity = 32.174 * 0.3048;
}
