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
 * Simulator Main
 *
 * Reads a flat parameter file, generates a data file.
 *
 * This program is intended to be run from other more clever user interfaces.
 *
 * This version does not yet model tank fill.
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <math.h>
#include <string.h>
#include <strings.h>
#include <getopt.h>
#include "ts_parse.h"
#include "scio.h"
#include "linkage.h"
#include "fuel.h"
#include "state.h"

#define	P_UNIT	"atm"


static double tankdia;
static double injectordia;
static double noz_t_dia, noz_e_dia;
static double noz_e_ratio;
static int noz_e_dia_set, noz_e_ratio_set;
static double supply_tank_pressure;

static double ventdia;

static int grainlength_set, graindiameter_set, graincore_set;
static double filltemp, filldrop;
static double fillpress;
static double injector_count_d;
static int filltemp_set, filldrop_set, fillpress_set;
static int dry_mass_set;
static int supply_tank_pressure_set;
static int ambient_air_pressure_set;
static int nozzle_half_angle_set;

static double lfuelinjector_count_d;
static int lfuelinjectordia_set;
static int lfuelinjectorid_set, lfuelinjectorod_set;
static int lfuelinjector_count_set;
static int lfuelinjectorcd_set;
static int lfueltankvolume_set;
static int lfuelmass_set;
static int lfuelvolume_set;
static int nitrogen_pressure_initial_set;

static double ullage_height; /* height from vent to top of tank */

struct scio_input_parameter_s scio_input[] = {
{ "fuel",          STRING,      0,        &fuel,                  1, 0, },
{ "tankheight",    LENGTH,      REQUIRED, &tank_height,           1, 0, },
{ "ullageheight",  LENGTH,      REQUIRED, &ullage_height,         1, 0, },
{ "tankdia",       LENGTH,      REQUIRED, &tankdia,               1, 0, },
{ "grainlength",   LENGTH,      0,        &grain_length,          1, &grainlength_set, },
{ "graindiameter", LENGTH,      0,        &grain_diameter,        1, &graindiameter_set, },
{ "graincore",     LENGTH,      0,	  &grain_init_core,       1, &graincore_set, },
{ "nozzlethroat",  LENGTH,      REQUIRED, &noz_t_dia,             1, 0, },
{ "nozzleexit",    LENGTH,      0,        &noz_e_dia,             1, &noz_e_dia_set, },
{ "nozzleratio",   NUMBER,      0,        &noz_e_ratio,           1, &noz_e_ratio_set, },
{ "nozcfadj",      NUMBER,      REQUIRED, &nozzle_cf_correction,  1, 0, },
{ "nozhalfangle",  ANGLE,       0,        &nozzle_half_angle,     1, &nozzle_half_angle_set, },
{ "cstaradj",      NUMBER,      REQUIRED, &combustion_efficiency, 1, 0, },
{ "injectordia",   LENGTH,      REQUIRED, &injectordia,           1, 0, },
{ "injectorcd",    NUMBER,      REQUIRED, &injector_cd,           1, 0, },
{ "injectorcount", NUMBER,      0,        &injector_count_d,      1, 0, },
{ "ventdia",       LENGTH,      REQUIRED, &ventdia,               1, 0, },
{ "ventcd",        NUMBER,      REQUIRED, &vent_cd,               1, 0, },
{ "timestep",      TIME,        0,        &sim_time_step,         1, 0, },
{ "filltemp",      TEMPERATURE, 0,        &filltemp,              1, &filltemp_set, },
{ "filldrop",      PRESSURE,    0,        &filldrop,              1, &filldrop_set, },
{ "fillpress",     PRESSURE,    0,        &fillpress,             1, &fillpress_set, },
{ "drymass",       MASS,        0,        &dry_mass,              1, &dry_mass_set,  },
{ "ambientpressure", PRESSURE,	0,        &ambient_air_pressure,  1, &ambient_air_pressure_set, },
{ "fuelinjectorid", LENGTH,	0,        &lfuelinjectorid,       1, &lfuelinjectorid_set, } ,
{ "fuelinjectorod", LENGTH,	0,        &lfuelinjectorod,       1, &lfuelinjectorod_set, } ,
{ "fuelinjectordia", LENGTH,	0,        &lfuelinjectordia,      1, &lfuelinjectordia_set, },
{ "fuelinjectorcount", NUMBER,	0,	  &lfuelinjector_count_d, 1, &lfuelinjector_count_set, },
{ "fuelinjectorcd", NUMBER,	0,	  &lfuelinjectorcd,       1, &lfuelinjectorcd_set, },
{ "fueltankvolume", VOLUME,	0,	  &lfueltankvolume,       1, &lfueltankvolume_set, },
{ "fuelmass",      MASS,	0,	  &lfuelmass,		  1, &lfuelmass_set, },
{ "fuelvolume",    VOLUME,	0,	  &lfuelvolume,		  1, &lfuelvolume_set, },
{ "nitrogenpressure",PRESSURE,	0,	  &nitrogen_pressure_initial,1, &nitrogen_pressure_initial_set, },

};

static void
report_input(FILE *datafile)
{
	fprintf(datafile, "SECTION,parameters\n");
	fprintf(datafile, "Parameter,Value,Unit\n");
	fprintf(datafile, "tankheight,%.6e,meters\n", tank_height);
	fprintf(datafile, "ullageheight,%.6e,meters\n", ullage_height);
	fprintf(datafile, "tankvolume,%.6e,meters\n", tank_volume);
	if (sim_type == HYBRID) {
		fprintf(datafile, "grainlength,%.6e,meters\n", grain_length);
		fprintf(datafile, "graindiameter,%.6e,meters\n", grain_diameter);
		fprintf(datafile, "graincore,%.6e,meters\n", grain_init_core);
	} else {
		fprintf(datafile, "fuelinjectordia,%.6e,meters\n", lfuelinjectordia);
		fprintf(datafile, "fuelinjectorid,%.6e,meters\n", lfuelinjectorid);
		fprintf(datafile, "fuelinjectorod,%.6e,meters\n", lfuelinjectorod);
		fprintf(datafile, "fuelinjectorcount,%.6e\n", (double)lfuelinjector_count);
		fprintf(datafile, "fuelinjectorcd,%.6e\n", lfuelinjectorcd);
		fprintf(datafile, "fueltankvolume,%.6e,meters**3\n", lfueltankvolume);
		fprintf(datafile, "fuelmass,%.6e,kg\n", lfuelmass);
		fprintf(datafile, "fuelvolume,%.6e,meters**3\n", lfuelvolume);
	}
	fprintf(datafile, "nozzlethroat,%.6e,meters\n", noz_t_dia);
	fprintf(datafile, "nozzleexit,%.6e,meters\n", noz_e_dia);
	fprintf(datafile, "nozcfadj,%.6e\n", nozzle_cf_correction);
	fprintf(datafile, "nozzlehalfangle,%.6e,radian\n", nozzle_half_angle);
	fprintf(datafile, "cstaradj,%.6e\n", combustion_efficiency);
	fprintf(datafile, "injectordia,%.6e,meters\n", injectordia);
	fprintf(datafile, "injectorcd,%.6e\n", injector_cd);
	fprintf(datafile, "injectorcount,%.6e\n", (double)injector_count);
	fprintf(datafile, "ventdia,%.6e,meters\n", ventdia);
	fprintf(datafile, "ventcd,%.6e\n", vent_cd);
	fprintf(datafile, "timestep,%.6e,seconds\n", sim_time_step);
	fprintf(datafile, "filltemp,%.6e,kelvin\n", filltemp);
	fprintf(datafile, "filldrop,%.6e,pascal\n", filldrop);
	fprintf(datafile, "ambientpressure,%.6e,pascal\n",
				ambient_air_pressure);
	if (supply_tank_pressure_set)
		fprintf(datafile, "supplypress,%.6e,pascal\n",
			supply_tank_pressure);
	if (dry_mass_set)
		fprintf(datafile, "drymass,%.6e,kg\n",dry_mass);
	if (filltemp_set)
		fprintf(datafile, "ventmass,%.6e,kg\n",vent_mass);
	fprintf(datafile, "fuel,%s,\n", fuel);
	fprintf(datafile, "\n");
	fflush(datafile);
}

char *myname;

static void
parse_input()
{
	struct ts_parsed_s *input_buffer;
	FILE *input;

	ts_parse_init();
	scio_init(scio_input, sizeof (scio_input) / sizeof (scio_input[0]));

	/*
	 * Read until EOF
	 */
	input = stdin;
	input_buffer = NULL;

	for (;;) {
		input_buffer = ts_parse(input, input_buffer);
		if (!input_buffer)
			break;
		scio_input_line(input_buffer);
	}

	scio_term();
}

static void
set_default_parameters()
{
	fuel = "PVC";
	injector_count_d = 1.0;
	lfuelinjector_count_d = 1.0;
	lfuelmass = 0.;
	lfuelvolume = 0.;
	sim_time_step = 0.001;
}

/*
 * Calculate some derived values.
 * Perform some basic input error checking.
 */
static void
parameter_setup()
{
	int errors;
	double d1, d2;

	errors = 0;

	/*
	 * First figure out if this is a liquid fuel or a hybrid fuel simulation
	 */
	if (fuel_data(fuel, 0))
		sim_type = LIQUID;	// not found in the solid fuel database.
	else
		sim_type = HYBRID;

	/*
	 * Legal tank combinations:
	 *   Fill pressure only
	 *	Sets the flight tank to this pressure. No waste calculation.
	 *   Fill pressure and fill temperature
	 *	Sets the flight tank to this pressure. Calculates how
	 *	much nitrous we boiled off to cool to this pressure.
	 *   Fill pressure drop and fill temperature
	 *	Sets the flight tank to the indicated drop from fill tank.
	 *	Calculates how much nitrous boiled off.
	 */
	if (fillpress_set && filldrop_set) {
		fprintf(stderr,"%s: cannot specify both fill pressure ",
			myname);
		fprintf(stderr, " and fill pressure drop\n");
		errors++;
	}

	if (!fillpress_set && (!filldrop_set || !filltemp_set)) {
		fprintf(stderr, "%s: both fill pressure drop and ",
			myname);
		fprintf(stderr, "fill temperature required "
				"to calculate fill pressure.\n");
		errors++;
	}

	if ((noz_e_dia_set && noz_e_ratio_set) ||
	    (!noz_e_dia_set && !noz_e_ratio_set)) {
	    	fprintf(stderr, "%s: must specify exactly one of "
			"nozzleexit or nozzleratio\n",
			myname);
		errors++;
	}

	if (!nozzle_half_angle_set)
		nozzle_half_angle = 15. * pi / 180.;	/* default = 15 */

	if (nozzle_half_angle < 0. || nozzle_half_angle > pi / 2.) {
		fprintf(stderr, "%s: nozzle half angle (%.1f) must be in "
		                "the range [0., 90.] degrees\n",
				myname, nozzle_half_angle);
		errors++;
	}

	injector_count = injector_count_d + .0125;
	if (injector_count < 1) {
		fprintf(stderr, "%s: injector count (%d) must be positive.\n",
			myname, injector_count);
		errors++;
	}

	if (sim_type == LIQUID) {
		lfuelinjector_count = lfuelinjector_count_d + .0125;

		if (!lfuelmass_set && !lfuelvolume_set) {
			fprintf(stderr, "%s: must set either fuelmass or "
					"fuelvolume\n", myname);
			errors++;
		}
		
		if (!lfuelinjectorcd_set) {
			fprintf(stderr, "%s: must set fuelinjectorcd\n",
				myname);
			errors++;
		}
		
		if (!(lfuelinjectordia_set || (lfuelinjectorid_set && lfuelinjectorod_set))) {
			fprintf(stderr, "%s: must set fuelinjectordia or\n",
				myname);
			fprintf(stderr, "\tboth fuelinjectorid and fuelinjectorod\n");
			errors++;
		}

		if (!nitrogen_pressure_initial_set) {
			fprintf(stderr, "%s: must set nitrogenpressure\n",
				myname);
			errors++;
		}
		if (lfuelinjectordia_set) {
			d2 = lfuelinjectordia;
			d1 = 0.;
		} else {
			d2 = lfuelinjectorod;
			d1 = lfuelinjectorid;
		}
		liquid_injector_area = pi/4. * (d2 * d2 - d1 * d1);
	}

	if (sim_type == HYBRID) {
		if (!grainlength_set) {
			fprintf(stderr, "%s: grainlength parameter required ",
				myname);
			fprintf(stderr, "in hybrid simulations.\n");
			errors++;
		}

		if (!graindiameter_set) {
			fprintf(stderr, "%s: graindiameter parameter required ",
				myname);
			fprintf(stderr, "in hybrid simulations.\n");
			errors++;
		}

		if (!graincore_set) {
			fprintf(stderr, "%s: graincore parameter required ",
				myname);
			fprintf(stderr, "in hybrid simulations.\n");
			errors++;
		}
	}

	if (errors)
		error_exit(1);

	tank_volume = pi/4. * tankdia * tankdia * tank_height;
	injector_area = pi/4. * injectordia * injectordia;

	nozzle_throat_area = pi/4. * noz_t_dia * noz_t_dia;
	if (noz_e_dia_set)
		nozzle_exit_area = pi/4. * noz_e_dia * noz_e_dia;
	else {
		nozzle_exit_area = nozzle_throat_area *
			noz_e_ratio;
		noz_e_dia = noz_t_dia * sqrt(noz_e_ratio);
	}

	vent_area = pi/4. * ventdia * ventdia;

	if (!ambient_air_pressure_set)
		ambient_air_pressure = atmosphere_pressure;
}

/*
 * Find the energy at which the tank is at the requested tempurature.
 */

static int
converged(double temp)
{
	double delta;

	delta = tank_temperature - temp;
	if (delta < 0)
		delta = -delta;

	return (delta < 0.01);
}

static void
tank_set_temp(double temp)
{
	int i;
	int low_set;
	int high_set;
	double low_energy;
	double high_energy;

	low_set = 0;
	high_set = 0;
	tank_energy = 32768;


	i = 0;
	while (!low_set || !high_set || !converged(temp)) {
		tank_temperature = temp;
		tank();

		if (n2o_thermo_error == TOO_HOT) {
			high_energy = tank_energy;
			high_set = 1;
			if (!low_set)
				tank_temperature -= 1;
		} else if (n2o_thermo_error == TOO_COLD) {
			low_energy = tank_energy;
			low_set= 1;
			if (!high_set)
				tank_temperature += 1;
		} else if (tank_temperature < temp) {
			low_energy = tank_energy;
			low_set= 1;
		} else {
			high_energy = tank_energy;
			high_set = 1;
		}

		if (!low_set)
			tank_energy /= 1.125;
		else if (!high_set)
			tank_energy *= 1.125;
		else
			tank_energy = (low_energy + high_energy) / 2.;
		if (tank_energy < 1.) {
			printf("FAILED: energy == 0\n");
			break;
		} else if (tank_energy > 1e8) {
			printf("FAILED: energy > 100,000,000\n");
			break;
		}
		if (i++ > 1000) {
			printf("FAILED %d iterations\n", i);
			break;
		}
	}
}

/*
 * Find the temperature that matches the required pressure.
 */
static double
n2o_temp(double pressure)
{
	double lo_temp, hi_temp;
	double min_press, max_press;
	double temp, tp;

	lo_temp = 250.;
	min_press = saturation_pressure(lo_temp);

	if (pressure < min_press) {
		fprintf(stderr, "%s: requested flight tank pressure (%.1f %s) "
				"is less than minimum (%.1f %s)\n",
			    myname,
			    scio_convert(pressure, PRESSURE, P_UNIT), P_UNIT,
			    scio_convert(min_press, PRESSURE, P_UNIT), P_UNIT);
		error_exit(1);
	}

	hi_temp = 309.;
	max_press = saturation_pressure(hi_temp);

	if (pressure > max_press) {
		fprintf(stderr, "%s: requested flight tank pressure (%.1f %s) "
				"is less than minimum (%.1f %s)\n",
			    myname,
			    scio_convert(pressure, PRESSURE, P_UNIT), P_UNIT,
			    scio_convert(max_press, PRESSURE, P_UNIT), P_UNIT);
		error_exit(1);
	}

	while (hi_temp - lo_temp > .01) {
		temp = (lo_temp + hi_temp) / 2.;
		tp = saturation_pressure(temp);
		if (tp < pressure)
			lo_temp = temp;
		else
			hi_temp = temp;
	}
	return temp;
}

static double filltemp, filldrop;


/*
 * This tank fill routine uses a very crude model.
 * Assume we know (from observation) how much pressure drop
 * we have between the supply tank and the flight tank.
 * Fill accordingly.
 *
 * Yuck.
 */

static void
simple_tank_fill()
{
	double flight_tank_pressure;
	double ullage_volume;
	double flight_tank_temp;

	supply_tank_pressure_set = 0;
	if (filltemp_set) {
		supply_tank_pressure = saturation_pressure(filltemp);
		supply_tank_pressure_set = 1;
	}

	if (fillpress_set)
		flight_tank_pressure = fillpress;
	else
		flight_tank_pressure = supply_tank_pressure - filldrop;

	if (supply_tank_pressure_set && fillpress_set &&
	    supply_tank_pressure - fillpress < WARN_SUPPLY_PRESSURE_DROP) {
		warn_supply_pressure = 1;
		warn_supply_pressure_drop_value = supply_tank_pressure -
			fillpress;
	}

	flight_tank_temp = n2o_temp(flight_tank_pressure);
	
	/* compute the mass of N2O vapor in the tank */
	ullage_volume = (ullage_height / tank_height) * tank_volume;
	tank_n2o_mass = ullage_volume * vapor_density(flight_tank_temp);

	/* now N2O liquid */
	tank_n2o_mass += (tank_volume - ullage_volume) *
				liquid_density(flight_tank_temp);


	/* Lastly, the initial energy state of the tank. */
	tank_set_temp(flight_tank_temp);

	/* How much nitrous did we boil off cooling the tank? */
	if (filltemp_set)
		tank_boil_off(filltemp);
}

static void
initialize()
{
	constants_init();
	set_default_parameters();
	parse_input();
	parameter_setup();
	sim_init();
	simple_tank_fill();
}

static void
set_defaults()
{
	ok_to_create_nzr = NZR_CREATE_NONE;
	use_enthalpy = 1;
}


static void
usage()
{
	set_defaults();
	fprintf(stderr, "Usage: %s <options>\n", myname);
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "\t-N: <nozzle data create mode (none)>\n");
	fprintf(stderr, "\t\tnone = do not create data\n");
	fprintf(stderr, "\t\tsystem = create using \"system\" library calls "
				"(recommended for Linux)\n");
	fprintf(stderr, "\t\texec = create using \"exec\" system calls "
				"(recommended for Windows)\n");
	fprintf(stderr, "\t-E: use internal energy, not enthalphy for thermo\n");
	fprintf(stderr, "\t-w: print the warrentee\n");
	fprintf(stderr, "\t-l: print the license\n");
	fprintf(stderr, "\t-v: print the version\n");
	fprintf(stderr, "\nInput is on stdin, output on stdout.\n");
	exit(0);
}

static void
grok_args(int argc, char **argv)
{
	int c;
	int errors;
	int nargs;

	myname = *argv; 

	errors = 0;
	set_defaults();
	while ((c = getopt(argc, argv, "vwlEN:h")) != EOF)
	switch (c) {
	
		case 'v':
		case 'w':
		case 'l':
			license(c);
		case 'E':
			use_enthalpy = 0;
			break;
		case 'N':
			if (strcmp(optarg, "none") == 0)
				ok_to_create_nzr = NZR_CREATE_NONE;
			else if (strcmp(optarg, "system") == 0)
				ok_to_create_nzr = NZR_CREATE_SYSTEM;
			else if (strcmp(optarg, "exec") == 0)
				ok_to_create_nzr = NZR_CREATE_EXEC;
			else {
				fprintf(stderr, "%s: bad -N option\n",
					myname);
				errors++;
			}
			break;
		case 'h':
		case '?':
		default:
			usage();
	}

	nargs = argc - optind;
	if (nargs)
		errors++;

	if (errors)
		usage();
}


int
main(int argc, char **argv)
{
	FILE *datafile;

	grok_args(argc, argv);

	datafile = stdout;

	initialize();
	report_input(datafile);
	record_data_init(0., datafile);
	sim_loop();
	record_data_term();
	print_errors(stderr);
	fprintf(datafile, "SECTION,errors\n");
	print_errors(datafile);
	fprintf(datafile, "\n");
	exit(0);
}
