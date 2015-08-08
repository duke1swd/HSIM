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
 * Function to record the data at each time step
 */

#include <stdio.h>
#include <stdlib.h>
#include "state.h"
#include "linkage.h"

static double last_time;
static double step;
static FILE *output;

void
record_data_init(double s, FILE *out)
{
	output = out;
	step = s;
	last_time = 0.;
	fprintf(output, "SECTION,timeseries\n");
	switch (sim_type) {
	    case HYBRID:
		fprintf(output, 
			"time,"
			"tank energy,"
			"tank n2o mass,"
			"tank pressure,"
			"tank temperature,"
			"n2o liquid mass,"
			"n2o liquid density,"
			"fuel mass,"
			"grain core,"
			"fuel rb,"
			"chamber pressure,"
			"c star,"
			"n2o flow rate,"
			"n2o vent rate,"
			"n2o flux,"
			"fuel flow rate,"
			"isp,"
			"nozzle cf,"
			"thrust,"
			"exit pressure\n");
		break;
	    case LIQUID:
		fprintf(output, 
			"time,"
			"tank energy,"
			"tank n2o mass,"
			"tank pressure,"
			"tank temperature,"
			"n2o liquid mass,"
			"n2o liquid density,"
			"liquid fuel mass,"
			"liquid fuel volume,"
			"nitrogen pressure,"
			"chamber pressure,"
			"c star,"
			"n2o flow rate,"
			"n2o vent rate,"
			"n2o flux,"
			"fuel flow rate,"
			"isp,"
			"nozzle cf,"
			"thrust,"
			"exit pressure\n");
	    	break;
	    default:
	    	fprintf(stderr, "BAD SIM TYPE in record_data.c\n");
		exit(1);
	}
}

void
record_data_term()
{
	fprintf(output, "END-OF-DATA\n\n");
	fflush(output);
}

void
record_data()
{
	if (sim_time < last_time + step)
		return;
	last_time = sim_time;

	switch (sim_type) {
	    case HYBRID:
		fprintf(output,"%f,", sim_time);
		fprintf(output,"%f,", tank_energy);
		fprintf(output,"%f,", tank_n2o_mass);
		fprintf(output,"%f,", tank_pressure);
		fprintf(output,"%f,", tank_temperature);
		fprintf(output,"%f,", n2o_liquid_mass);
		fprintf(output,"%f,", n2o_liquid_density);
		fprintf(output,"%f,", fuel_mass);
		fprintf(output,"%f,", grain_core);
		fprintf(output,"%f,", fuel_rb);
		fprintf(output,"%f,", chamber_pressure);
		fprintf(output,"%f,", c_star);
		fprintf(output,"%f,", n2o_flow_rate);
		fprintf(output,"%f,", n2o_vent_rate);
		fprintf(output,"%f,", n2o_flux);
		fprintf(output,"%f,", fuel_flow_rate);
		fprintf(output,"%f,", isp);
		fprintf(output,"%f,", nozzle_cf);
		fprintf(output,"%f,", thrust);
		fprintf(output,"%f\n ", exit_pressure);
		break;
	    case LIQUID:
		fprintf(output,"%f,", sim_time);
		fprintf(output,"%f,", tank_energy);
		fprintf(output,"%f,", tank_n2o_mass);
		fprintf(output,"%f,", tank_pressure);
		fprintf(output,"%f,", tank_temperature);
		fprintf(output,"%f,", n2o_liquid_mass);
		fprintf(output,"%f,", n2o_liquid_density);
		fprintf(output, "%f,", lfuelmass);
		fprintf(output, "%f,", lfuelvolume);
		fprintf(output, "%f,", nitrogen_pressure);
		fprintf(output,"%f,", chamber_pressure);
		fprintf(output,"%f,", c_star);
		fprintf(output,"%f,", n2o_flow_rate);
		fprintf(output,"%f,", n2o_vent_rate);
		fprintf(output,"%f,", n2o_flux);
		fprintf(output,"%f,", fuel_flow_rate);
		fprintf(output,"%f,", isp);
		fprintf(output,"%f,", nozzle_cf);
		fprintf(output,"%f,", thrust);
		fprintf(output,"%f\n ", exit_pressure);
		break;
	}
	fflush(output);
}
