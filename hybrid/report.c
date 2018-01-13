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
 * Reads the raw simulator output and generates a human readable report.
 */

char *Version = "Report Generator 1.0";

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <getopt.h>
#include "rsim.h"
#include "ts_parse.h"
#include "scio.h"
#include "rocksim.h"

char *myname;
FILE *input;
char *input_file_name;
int rocksim_mode;
int html_mode;
char * rocksim_output_name;
FILE *rocksim_output;
int debug;

const double g = 32.174 * 0.3048;

static void
usage()
{
	fprintf(stderr, "Usage: %s <Options>\n", myname);
	fprintf(stderr, "  Reads raw data on stdin, produces "
			"report on stdout.\n");
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "\t-r (rockim file (%s)>\n",
		rocksim_output_name? rocksim_output_name: "none");
	exit(1);
}

static void
set_defaults()
{
	debug = 0;
	rocksim_mode = 0;
	html_mode = 0;
	rocksim_output_name = (char *)0;
}

static void
grok_args(int argc, char **argv)
{
	int c;
	int errors;
	int nargs;

	myname = *argv;
	set_defaults();

	errors = 0;

	while ((c = getopt(argc, argv, "Hdr:h")) != EOF)
	switch (c) {

	    case 'H':
	    	html_mode++;
		break;
	    case 'r':
	    	rocksim_mode++;
		rocksim_output_name = optarg;
		break;
	    case 'd':
	    	debug++;
		break;

	    case 'h':
	    case '?':
	    default:
	    	usage();
		break;
	}

	nargs = argc - optind;

	if (nargs == 0) {
		input = stdin;
		input_file_name = "(stdin)";
	} else if (nargs == 1) {
		input_file_name = argv[optind];
		input = fopen(input_file_name, "r");
		if (input == NULL) {
			fprintf(stderr, "%s: cannot open data file %s "
					"for reading\n",
				myname, input_file_name);
			perror("open");
			errors++;
		}
	} else {
		fprintf(stderr, "%s: only one positional argument "
				"(input file) allowed\n",
			myname);
		errors++;
	}

	if (rocksim_mode) {
		rocksim_output = fopen(rocksim_output_name, "w");
		if (rocksim_output == NULL) {
			fprintf(stderr, "%s: Cannot open rocksim file %s "
					"for writing\n",
				myname, rocksim_output_name);
			perror("open");
			errors++;
		}
	}

	if (errors)
		usage();
}

/*
 * This function reads the simulator input parameters.
 */
double tank_height;
double ullage_height;
double tank_volume;
double grain_length;
double grain_diameter;
double grain_core;
double nozzle_throat;
double nozzle_exit;
double nozzle_cf_adjust;
double nozzle_half_angle;
double cstar;
double cstar_adjust;
double injector_diameter;
double injector_cd;
double injector_count;
double vent_diameter;
double vent_cd;
double fill_temp;
double fill_pressure_drop;
double dry_mass;
double vent_mass;
double engine_dia;
double engine_len;
double supply_press;
double ambientpressure;

double burn_time;

#define	HYBRID	1
#define	LIQUID	2
int sim_type;

char fuel[128];

int tank_height_set;
int ullage_height_set;
int tank_volume_set;
int vent_mass_set;
int grain_length_set;
int grain_diameter_set;
int grain_core_set;
int nozzle_throat_set;
int nozzle_exit_set;
int nozzle_cf_adjust_set;
int cstar_set;
int cstar_adjust_set;
int injector_diameter_set;
int injector_cd_set;
int injector_count_set;
int vent_diameter_set;
int vent_cd_set;
int fill_temp_set;
int fill_pressure_drop_set;
int dry_mass_set;
int engine_dia_set;
int engine_len_set;
int supply_press_set;
int ambientpressure_set;
int nozzle_half_angle_set;

struct input_param_s {
	char *name;
	double *value;
	int *set;
} input_parameters[] = {
  { "tankheight",    &tank_height,        &tank_height_set,        },
  { "ullageheight",  &ullage_height,      &ullage_height_set,      },
  { "tankvolume",    &tank_volume,        &tank_volume_set,        },
  { "ventmass",      &vent_mass,          &vent_mass_set,          },
  { "grainlength",   &grain_length,       &grain_length_set,       },
  { "graindiameter", &grain_diameter,     &grain_diameter_set,     },
  { "graincore",     &grain_core,         &grain_core_set,         },
  { "nozzlethroat",  &nozzle_throat,      &nozzle_throat_set,      },
  { "nozzleexit",    &nozzle_exit,        &nozzle_exit_set,        },
  { "nozcfadj",      &nozzle_cf_adjust,   &nozzle_cf_adjust_set,   },
  { "nozzlehalfangle",&nozzle_half_angle, &nozzle_half_angle_set,  },
  { "cstaradj",      &cstar_adjust,       &cstar_adjust_set,       },
  { "injectordia",   &injector_diameter,  &injector_diameter_set,  },
  { "injectorcd",    &injector_cd,        &injector_cd_set,        },
  { "injectorcount", &injector_count,     &injector_count_set,     },
  { "ventdia",       &vent_diameter,      &vent_diameter_set,      },
  { "ventcd",        &vent_cd,            &vent_cd_set,            },
  { "filltemp",      &fill_temp,          &fill_temp_set,          },
  { "filldrop",      &fill_pressure_drop, &fill_pressure_drop_set, },
  { "supplypress",   &supply_press,       &supply_press_set,       },
  { "drymass",       &dry_mass,           &dry_mass_set,           },
  { "enginedia",     &engine_dia,         &engine_dia_set,         },
  { "enginelen",     &engine_len,         &engine_len_set,         },
  { "ambientpressure",&ambientpressure,   &ambientpressure_set,    },
  { (char *)0,       (double *)0,         (int *)0,                },
};

/*
 * Process lines from the first section of the file.
 */

static void
input_1_headers(int nptrs, char **ptrs)
{
	struct input_param_s *ip;

	/* mark all input parameters as not yet received. */
	for (ip = input_parameters; ip->name; ip++)
		*(ip->set) = 0;
}

/*
 * Find parameter values.
 * Handles "fuel" separately, as it is a string, not a float.
 */
static void
input_1(int nptrs, char **ptrs)
{
	struct input_param_s *ip;

	/* search for the match */
	if (strcmp(ptrs[0], "fuel") == 0)
		strncpy(fuel, ptrs[1], sizeof fuel);
	else
	    for (ip = input_parameters; ip->name; ip++)
		if (strcmp(ptrs[0], ip->name) == 0) {
			/* matched */
			*(ip->value) = atof(ptrs[1]);
			*(ip->set) += 1;
			break;
		}
}

static void
print_input1()
{
	struct input_param_s *ip;

	for (ip = input_parameters; ip->name; ip++) {
		printf("%s: ", ip->name);
		if (*(ip->set))
			printf("%f\n", *(ip->value));
		else
			printf("not set\n");
	}
}

static char *column_names[] = {
	"tank n2o mass",
#define	TANK_N2O_MASS		0

	"tank pressure",
#define	TANK_PRESSURE		1

	"tank temperature",
#define	TANK_TEMPERATURE	2

	"n2o liquid mass",
#define	N2O_LIQUID_MASS		3

	"n2o liquid density",
#define	N2O_LIQUID_DENSITY	4

	"fuel mass",
#define	FUEL_MASS		5

	"grain core",
#define	GRAIN_CORE		6

	"chamber pressure",
#define	CHAMBER_PRESSURE	7

	"n2o flow rate",
#define N2O_FLOW_RATE		8

	"n2o vent rate",
#define	N2O_VENT_RATE		9

	"n2o flux",
#define	N2O_FLUX		10

	"fuel flow rate",
#define	FUEL_FLOW_RATE		11

	"isp",
#define	ISP			12

	"nozzle cf",
#define	NOZZLE_CF		13

	"thrust",
#define	THRUST			14

	"exit pressure",
#define	EXIT_PRESSURE		15

	"XXX O/F RATIO",		/* computed */
#define	OF_RATIO		16

	"XXX injector pressure ratio",	/* computed */
#define	IP_RATIO		17

	"time",
#define BURN_TIME		18

	"liquid fuel mass",
#define	LFUEL_MASS		19

	"liquid fuel volume",
#define	LFUEL_VOLUME		20

	"nitrogen pressure",
#define	LFUEL_PRESSURE		21

	"XXX LF injector pressure ratio",	/* computed */
#define	IPL_RATIO		22
};

#define	NCOL	(sizeof (column_names) / sizeof (column_names[0]))

static int column_numbers[NCOL];

static double column_min[NCOL];
static double column_max[NCOL];
static double column_sum[NCOL];
static double column_init[NCOL];
static double column_final[NCOL];

static int nrow;

#define	RS_MAX_TIMES	10000
#define	RS_AVERAGE	10		/*average 10 lines together */

static struct rse_datapoint_s *rs_data;
int rs_n_points;
int rs_n_av;

/*
 * Compute an impulse class.
 * Doesn't work for smaller than C class engines.
 */
static char
impulse_to_class(double impulse)
{
	char c;

	c = 'C';
	while (impulse >= 10.) {
		c++;
		impulse /= 2;
	}
	return c;
}

static double
impulse_to_class_fraction(double impulse)
{
	while (impulse >= 10.)
		impulse /= 2;

	return (impulse - 5.) / 5.;
}

static void
rocksim_init()
{
	int errors;

	rs_n_points = 0;
	rs_n_av = 0;

	if (!rocksim_mode)
		return;

	errors = 0;

	rs_data = (struct rse_datapoint_s *)malloc(
		RS_MAX_TIMES * sizeof (struct rse_datapoint_s));
	if (rs_data == (struct rse_datapoint_s *)0) {
		fprintf(stderr, "%s: failed to malloc memory for "
				"rocksim data.\n", myname);
		errors++;
	}

	if (!dry_mass_set) {
		fprintf(stderr, "%s: motor dry mass was not specified.\n",
			myname);
		errors++;
	}

	if (errors) {
		fprintf(stderr, "\tRocksim report disabled\n");
		rocksim_mode = 0;
		return;
	}
}

static void
rocksim_point_close()
{
	struct rse_datapoint_s *rp;

	if (rs_n_av == 0)
		return;

	rp = rs_data + rs_n_points;

	rp->Time /= (double)rs_n_av;
	rp->Thrust /= (double)rs_n_av;
	rp->Mass /= (double)rs_n_av;
	rp->CG /= (double)rs_n_av;
	
	rs_n_av = 0;
	rs_n_points++;
}

/*
 * Computes the rocksim data points.
 * CG stuff is currently NYI.
 */
void
rocksim_point(double sim_time, double thrust, double fuelmass, double cg)
{
	struct rse_datapoint_s *rp;

	if (!rocksim_mode)
		return;

	rp = rs_data + rs_n_points;
	if (rs_n_av == 0) {
		rp->Time = 0.;
		rp->Thrust = 0.;
		rp->Mass = 0.;
		rp->CG = 0.;
	}
	rp->Time += sim_time;
	rp->Thrust += thrust;
	rp->Mass += fuelmass;
	rp->CG += cg;

	if (++rs_n_av >= RS_AVERAGE || rs_n_points == 0)
		rocksim_point_close();
}

static void
input_2_headers(int nptrs, char **ptrs)
{
	int i, j;

	if (debug)
	print_input1();

	nrow = 0;

	for (i = 0; i < NCOL; i++)
		column_numbers[i] = -1;

	for (j = 0; j < nptrs; j++)
	    for (i = 0; i < NCOL; i++)
	    	if (strcmp(ptrs[j], column_names[i]) == 0) {
			column_numbers[i] = j;
			break;
		}

	if (debug) {
		for (i = 0; i < NCOL; i++)
			printf("COL: %s is col %d\n",
				column_names[i],
				column_numbers[i]);
	}
	rocksim_init();
}

/* 
 * This function reads the time-sequence data from the simulator.
 */
static void
input_2(int nptrs, char **ptrs)
{
	int i;
	double v;
	double r_time, r_thrust, r_mass;

	r_mass = 0;
	for (i = 0; i < NCOL; i++) {
		if (i == OF_RATIO)
			v = atof(ptrs[column_numbers[N2O_FLOW_RATE]]) /
			    atof(ptrs[column_numbers[FUEL_FLOW_RATE]]);
		else if (i == IP_RATIO)
			v = atof(ptrs[column_numbers[TANK_PRESSURE]]) /
			    atof(ptrs[column_numbers[CHAMBER_PRESSURE]]);
		else if (i == IPL_RATIO)
			v = atof(ptrs[column_numbers[LFUEL_PRESSURE]]) /
			    atof(ptrs[column_numbers[CHAMBER_PRESSURE]]);
		else if (column_numbers[i] >= 0)
			v = atof(ptrs[column_numbers[i]]);
		else
			v = 0.;

		switch(i) {
		case BURN_TIME:
			r_time = v;
			break;
		case THRUST:
			r_thrust = v;
			break;
		case TANK_N2O_MASS:
		case FUEL_MASS:
		case LFUEL_MASS:
			r_mass += v;
			break;
		}

		if (nrow == 0) {
			column_min[i] = v;
			column_max[i] = v;
			column_sum[i] = v;
			column_init[i] = v;
		} else {
			if (v < column_min[i])
				column_min[i] = v;
			if (v > column_max[i])
				column_max[i] = v;
			column_sum[i] += v;
		}

		column_final[i] = v;
	}
	nrow++;

	rocksim_point(r_time, r_thrust, r_mass, 0.);
}

#define	SAVE_INCR	4096
static int saved_n_bytes;
static int saved_size;
static char *saved_text;

static void
save_text_init()
{
	saved_text = malloc(SAVE_INCR);
	if (saved_text == (char *)0) {
		fprintf(stderr, "%s: malloc of %d bytes failed\n",
			myname, SAVE_INCR);
		exit(1);
	}
	saved_size = SAVE_INCR;
	saved_n_bytes = 0;
}

static void
save_text(char *p)
{
	int l;

	l = strlen(p);
	while (saved_n_bytes + l + 1 >= saved_size) {
		saved_size += SAVE_INCR;
		saved_text = realloc(saved_text, saved_size);
		if (saved_text == (char *)0) {
			fprintf(stderr, "%s: realloc of %d bytes failed\n",
				myname, saved_size);
			exit(1);
		}
		
	}
	strcpy(saved_text + saved_n_bytes, p);
	saved_n_bytes += l;
}

static void
save_text_dump(FILE *output)
{
	fwrite(saved_text, 1, saved_n_bytes, output);
}

void
input_errors(int nptrs, char **ptrs)
{
	int i;

	for (i = 0; i < nptrs - 1; i++) {
		save_text(ptrs[i]);
		save_text(",");
	}
	save_text(ptrs[i]);
	save_text("\n");
}

void
input_errors_headers(int nptrs, char **ptrs)
{
	save_text_init();
	input_errors(nptrs, ptrs);
}

struct section_s {
	char *section;
	void (*parse_headers)(int nptrs, char **ptrs);
	void (*parser)(int nptrs, char **ptrs);
} parsers[] = {
	{ "parameters", input_1_headers,       input_1,      },
	{ "timeseries", input_2_headers,       input_2,      },
	{ "errors",     input_errors_headers,  input_errors, },
	{ (char *)0, },
};


/*
 * Process the input file.
 * Uses the section table above
 * Returns true if the END-OF-DATA marker was found
 */
static int
do_input()
{
	int first_line;
	int state;
	int nptrs;
	int found_end_of_data;
	char *p;
	struct section_s *sp;
	char *ptrs[32];
	char buffer[512];

	state = -1;
	first_line = 1;
	found_end_of_data = 0;

	/* for each input line */
	while ((nptrs = csv_read(input, buffer, sizeof (buffer),
	       ptrs, sizeof (ptrs) / sizeof (char *))) > 0) {
 
		for (p = ptrs[0]; *p == ' ' || *p == '\t'; p++ ) ;
		if (strcmp(p, "END-OF-DATA") == 0) {
			found_end_of_data = 1;
			continue;
		}

		if (strcmp(p, "SECTION") == 0) {
			for (sp = parsers; sp->section; sp++)
				if (strcmp(ptrs[1], sp->section) == 0) {
					state = sp - parsers;
					first_line = 1;
					goto contin;
				}
			fprintf(stderr, "%s: unknown data section \"%s\"\n",
				myname, ptrs[1]);
			state = -1;
		}

		if (state < 0)
			continue;
		if (first_line)
			(sp->parse_headers)(nptrs, ptrs);
		else
			(sp->parser)(nptrs, ptrs);
		first_line = 0;
	     contin:;
	}

	if (nrow <= 0) {
		fprintf(stderr, "%s: No input data\n", myname);
		exit(1);
	}
	return found_end_of_data;
}

static void
set_fuel_type()
{
	if (strcmp(fuel, "ipa") == 0 ||
	    strcmp(fuel, "IPA") == 0)
	    	sim_type = LIQUID;
	else
		sim_type = HYBRID;
}

static char warning[] =
   "THERE IS NO WARRANTY FOR THIS PROGRAM, TO THE EXTENT PERMITTED\n"
   "BY APPLICABLE LAW.  THE COPYRIGHT HOLDERS AND/OR OTHER PARTIES\n"
   "PROVIDE THIS PROGRAM \"AS IS\" WITHOUT WARRANTY OF ANY KIND,\n"
   "EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,\n"
   "THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR\n"
   "A PARTICULAR PURPOSE. THE ENTIRE RISK AS TO THE QUALITY AND\n"
   "PERFORMANCE OF THE PROGRAM IS WITH YOU.\n"
   "\n"
   "Specifically, while this simulator has been somewhat tested\n"
   "and appears to give reasonable results over a useful range\n"
   "of inputs, the authors make no claims of correctness or even\n"
   "reasonableness.\n"
   "\n";

static void
print_header(FILE *stream)
{
	fputs(warning, stream);
}

/*
 * Format and print the report.
 *
 * Section 1: summary of geometry.
 * Section 2: fill conditions (pressure, temp, N2O mass, N2O density)
 * Section 3: final tank conditions (same, less density, incl ullage %).
 * Section 4: chamber summary: initial/final grain port, i/f grain mass,
 *	delta mass burned, m/m/a pressure, m/m/a O/F
 * Section 5: injector specs
 * Section 6: nozzle stuff: m/m/a exit pressure
 * Section 7: initial + m/m/a trust, burn time, total impulse, delivered ISP
 */
static void
print_report()
{
	int icnt;
	double total_impulse;
	double ave_isp;

	if (html_mode)
		printf("<pre>\n");

	print_header(stdout);

	/* Section 1 */
	printf("\nSection 1: Geometry\n");
	printf("\tTank Height              %.3f meters\n", tank_height);
	printf("\tTank Volume              %.3f liters\n", tank_volume * 1000.);
	printf("\tUllage Height            %.3f meters\n", ullage_height);
	if (sim_type == HYBRID)
		printf("\tGrain Length             %.3f meters\n", grain_length);
	printf("\tNozzle Throat            %.3f inches\n",
		scio_convert(nozzle_throat, LENGTH, "in"));
	printf("\tNozzle Exit              %.3f inches\n",
		scio_convert(nozzle_exit, LENGTH, "in"));
	if (nozzle_half_angle_set)
	    printf("\tNozzle Half Angle        %.1f degrees\n",
		scio_convert(nozzle_half_angle, ANGLE, "degree"));
	printf("\tC* Adjustment            %.2f\n", cstar_adjust);
	printf("\tCf Adjustment            %.2f\n", nozzle_cf_adjust);
	if (!ambientpressure_set)
		ambientpressure = 101325.;
	printf("\tAmbient Pressure      %6.1f  atm\n",
			scio_convert(ambientpressure, PRESSURE, "atm"));

	/* Section 2 */
	printf("\nSection 2: Fill Conditions\n");
	if (supply_press_set)
		printf("\tN2O Supply Pressure      %.1f psi\n",
			scio_convert(supply_press, PRESSURE, "psi"));
	printf("\tInit Pressure            %.1f psi\n", 
			scio_convert(column_init[TANK_PRESSURE],
				PRESSURE, "psi"));
	printf("\tInit Temp                %5.0f F\n", 
			scio_convert(column_init[TANK_TEMPERATURE],
				TEMPERATURE, "F"));
	printf("\tInit N2O Total Mass      %5.2f kg\n", 
			scio_convert(column_init[TANK_N2O_MASS],
				MASS, "kg"));
	printf("\tInit N2O Liquid Mass     %5.2f kg\n", 
			scio_convert(column_init[N2O_LIQUID_MASS],
				MASS, "kg"));
	printf("\tInit N2O Density         %5.2f g/cc\n", 
			scio_convert(column_init[N2O_LIQUID_DENSITY],
				DENSITY, "g/cc"));
	printf("\tInit N2O Liquid Volume   %5.2f cc\n", 
			scio_convert(column_init[N2O_LIQUID_MASS] /
				    column_init[N2O_LIQUID_DENSITY],
				VOLUME, "cc"));
	if (vent_mass_set) {
		printf("\tN2O Vented to chill      %5.2f kg\n",
			scio_convert(vent_mass,
				MASS, "kg"));
		printf("\t                         %5.2f lbs\n",
			scio_convert(vent_mass,
				MASS, "lbm"));
		printf("\tTotal N2O Consumed       %5.2f lbs\n",
			scio_convert(vent_mass + column_init[TANK_N2O_MASS],
				MASS, "lbm"));
	}
	if (sim_type == LIQUID) {
		printf("\tInit LFuel Pressure      %.1f psi\n", 
				scio_convert(column_init[LFUEL_PRESSURE],
					PRESSURE, "psi"));
		printf("\tInit Lfuel Mass          %5.2f kg\n", 
				scio_convert(column_init[LFUEL_MASS],
					MASS, "kg"));
	}
	printf("\tFuel                      %s\n", fuel);

	/* Section 3 */
	printf("\nSection 3: Empty Conditions\n");
	printf("\tFinal Pressure           %.1f psi\n", 
			scio_convert(column_final[TANK_PRESSURE],
				PRESSURE, "psi"));
	printf("\tFinal Temp               %5.0f F\n", 
			scio_convert(column_final[TANK_TEMPERATURE],
				TEMPERATURE, "F"));
	printf("\tUllage N2O Mass          %5.2f kg\n", 
			scio_convert(column_final[TANK_N2O_MASS],
				MASS, "kg"));
	printf("\tUllage Percentage         %.1f %%\n",
			100 * column_final[TANK_N2O_MASS] /
			    column_init[TANK_N2O_MASS]);

	if (sim_type == LIQUID) {
		printf("\tFinal N2O Liquid Mass    %5.2f kg\n", 
				scio_convert(column_final[N2O_LIQUID_MASS],
					MASS, "kg"));
		printf("\tFinal LFuel Pressure     %.1f psi\n", 
				scio_convert(column_final[LFUEL_PRESSURE],
					PRESSURE, "psi"));
		printf("\tFinal LFuel Mass         %5.2f kg\n", 
				scio_convert(column_final[LFUEL_MASS],
					MASS, "kg"));
	}

	/* Section 4 */
	if (sim_type == HYBRID) {
		printf("\nSection 4: Chamber Summary        Init  Final   Average\n");
		printf("\tGrain Port               %6.3f %6.3f %6.3f inches\n",
			scio_convert(column_init[GRAIN_CORE],
				LENGTH, "in"),
			scio_convert(column_final[GRAIN_CORE],
				LENGTH, "in"),
			scio_convert(column_sum[GRAIN_CORE] / (double) nrow,
				LENGTH, "in"));
		printf("\tGrain Mass               %6.3f %6.3f kg\n",
			scio_convert(column_init[FUEL_MASS],
				MASS, "kg"),
			scio_convert(column_final[FUEL_MASS],
				MASS, "kg"));
		printf("\tFuel Consumed                   %6.3f kg\n",
			scio_convert(column_init[FUEL_MASS] -
					column_final[FUEL_MASS],
				MASS, "kg"));
		printf("\t\t\t      Min     Max     Average\n");
	} else
		printf("\nSection 4: Chamber Summary        Min     Max     Average\n");
	printf("\tChamber Pressure         %7.2f %7.2f %7.2f psi\n",
			scio_convert(column_min[CHAMBER_PRESSURE],
				PRESSURE, "psi"),
			scio_convert(column_max[CHAMBER_PRESSURE],
				PRESSURE, "psi"),
			scio_convert(column_sum[CHAMBER_PRESSURE] / (double) nrow,
				PRESSURE, "psi"));
	printf("\tO/F Ratio                %6.1f  %6.1f  %6.1f\n",
			column_min[OF_RATIO],
			column_max[OF_RATIO],
			column_sum[OF_RATIO] / (double) nrow);

	/* Section 5 */
	printf("\nSection 5: Injector Summary \n");
	icnt = injector_count + .001;
	if (icnt > 1) {
		printf("\tInjector Count           %d\n", icnt);
		printf("\tDiameter of Injectors    %.2f  mm\n",
			scio_convert(injector_diameter, LENGTH, "mm"));
		printf("\t                         %.3f inches\n",
			scio_convert(injector_diameter, LENGTH, "in"));
		printf("\tCd of Injectors          %.2f\n", injector_cd);
	} else {
		printf("\tInjector Diameter     %.2f  mm\n",
			scio_convert(injector_diameter, LENGTH, "mm"));
		printf("\t                      %.3f inches\n",
			scio_convert(injector_diameter, LENGTH, "in"));
		printf("\tInjector Cd           %.2f\n", injector_cd);

	}
	printf("\tTank/Chamber Pressure Ratio\n");
	printf("\t                        Min     Max    Average\n");
	printf("\t   N2O                  %.2f    %.2f    %.2f\n",
			column_min[IP_RATIO],
			column_max[IP_RATIO],
			column_sum[IP_RATIO] / (double) nrow);
	if (sim_type == LIQUID)
		printf("\t   LFuel                %.2f    %.2f    %.2f\n",
			column_min[IPL_RATIO],
			column_max[IPL_RATIO],
			column_sum[IPL_RATIO] / (double) nrow);

	/* Section 6 */
	printf("\nSection 6: Nozzle Summary        Min     Max    Average\n");
	printf("\tExit Pressure        %7.2f %7.2f %7.2f atm\n",
			scio_convert(column_min[EXIT_PRESSURE],
				PRESSURE, "atm"),
			scio_convert(column_max[EXIT_PRESSURE],
				PRESSURE, "atm"),
			scio_convert(column_sum[EXIT_PRESSURE] / (double) nrow,
				PRESSURE, "atm"));
	
	/* Section 7 */

	burn_time = column_final[BURN_TIME] - column_init[BURN_TIME];
	total_impulse = column_sum[THRUST] / (double)nrow * burn_time;
	if (sim_type == HYBRID) {
	    ave_isp = total_impulse /
		(column_init[TANK_N2O_MASS] - column_final[TANK_N2O_MASS] +
		 column_init[FUEL_MASS] - column_final[FUEL_MASS]);
	} else {
	    ave_isp = total_impulse /
		(column_init[TANK_N2O_MASS] - column_final[TANK_N2O_MASS] +
		 column_init[LFUEL_MASS] - column_final[LFUEL_MASS]);
	}

	printf("\nSection 7: Performance Summary  Init   Min    Max    Average\n");
	printf("\tThrust               %6.0f %6.0f %6.0f %6.0f lbf\n",
			scio_convert(column_init[THRUST], FORCE, "lb"),
			scio_convert(column_min[THRUST], FORCE, "lb"),
			scio_convert(column_max[THRUST], FORCE, "lb"),
			scio_convert(column_sum[THRUST] / (double) nrow,
				FORCE, "lb"));
	printf("\t                     %6.0f %6.0f %6.0f %6.0f N\n",
			scio_convert(column_init[THRUST], FORCE, "N"),
			scio_convert(column_min[THRUST], FORCE, "N"),
			scio_convert(column_max[THRUST], FORCE, "N"),
			scio_convert(column_sum[THRUST] / (double) nrow,
				FORCE, "N"));
	printf("\tDelivered ISP        %6.0f %6.0f %6.0f %6.0f meters/sec\n",
			scio_convert(column_init[ISP], VELOCITY, "m/s"),
			scio_convert(column_min[ISP], VELOCITY, "m/s"),
			scio_convert(column_max[ISP], VELOCITY, "m/s"),
			scio_convert(ave_isp, VELOCITY, "m/s"));
	printf("\tDelivered ISP        %6.0f %6.0f %6.0f %6.0f seconds\n",
			scio_convert(column_init[ISP]/g, TIME, "sec"),
			scio_convert(column_min[ISP]/g, TIME, "sec"),
			scio_convert(column_max[ISP]/g, TIME, "sec"),
			scio_convert(ave_isp/g, TIME, "sec"));
	printf("\tBurn Time            %6.3f seconds\n", burn_time);

	printf("\tTotal Impulse        %6.0f N-seconds\n", total_impulse);

	printf("\tMotor Designation     %c-%d (%.0f%%)\n",
		impulse_to_class(total_impulse),
		(int)(scio_convert(column_sum[THRUST] / (double) nrow,
			FORCE, "N") + .5),
		impulse_to_class_fraction(total_impulse) * 100.);

	if (html_mode)
		printf("</pre>\n");
}

static void
rocksim_report()
{
	int i;
	struct rse_s rse;

	if (!rocksim_mode)
		return;

	rocksim_point_close();

	rse.EngineMfg = "Evan Daniel";
	rse.TotalImpulse =
		column_sum[THRUST] / (double)nrow * burn_time;
	rse.EngineImpulseClass =
		impulse_to_class(rse.TotalImpulse);
	rse.EngineType = "hybrid";
	rse.EngineDia = 0.054;	/* reasonable default */
	if (engine_dia_set)
		rse.EngineDia = engine_dia;
	rse.EngineLen = 0.900;	/* reasonable default */
	if (engine_len_set)
		rse.EngineLen = engine_len;
	rse.PropellantMass = column_init[TANK_N2O_MASS] +
				column_init[FUEL_MASS];
	rse.EngineWetMass = dry_mass + rse.PropellantMass;
	rse.PeakThrust = column_max[THRUST];
	rse.AverageThrust = column_sum[THRUST]/ (double) nrow;
	rse.NozzleThroatDia = nozzle_throat;
	rse.NozzleExitDia = nozzle_exit;
	rse.BurnTimeSecs = burn_time;
	rse.MassFrac = 1. - (dry_mass + column_final[TANK_N2O_MASS] +
					column_final[FUEL_MASS]) /
				rse.EngineWetMass;
	rse.ISPSecs = column_sum[ISP] / (double) nrow;


	rse.comment = "Simulated by HSim Version 0.3.\n";

	rse_begin(rocksim_output);
	rse_datafile(&rse);
	for (i = 0; i < rs_n_points; i++)
		rse_datafile_point(rs_data + i);
	rse_end();
}

/*
 * Top Level Flow Control
 */
int
main(int argc, char **argv)
{
	grok_args(argc, argv);

	if (do_input()) {
		set_fuel_type();
		print_report();
		rocksim_report();
	}
	putchar('\n');
	save_text_dump(stdout);
	exit(0);
}
