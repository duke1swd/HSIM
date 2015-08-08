/*
 * Find out what happens to saturated N2O as you lower its pressure.
 */

#include <stdio.h>
#include <stdlib.h>
#include "ts_parse.h"
#include "scio.h"
#include "linkage.h"
#include "state.h"

#define	P_UNIT	"psi"
#define	T_UNIT	"F"
#define	V_UNIT	"liter"
#define	L_UNIT	"in"
#define	M_UNIT	"kg"
#define	D_UNIT	"g/cc"
#define	E_UNIT	"joule"

double pressure;
double pressure_increment;

struct scio_input_parameter_s scio_input[] = {
{
	"pressure",
	PRESSURE,
	REQUIRED,
	&pressure,
	1,
	0,
},
{
	"increment",
	PRESSURE,
	0,
	&pressure_increment,
	1,
	0,
},

};

char *myname;

static void
initialize()
{
	struct ts_parsed_s *input_buffer;
	FILE *input;

	use_enthalpy = 1;
	constants_init();
	n2o_thermo_init();

	pressure_increment = scio_f_convert(10., PRESSURE, P_UNIT);	// 10 PSI

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

double	temp;
double	liquid_fraction;
double	energy;
double	density;

static void
report_header()
{
	printf("Pressure  Temperature  Liquid Fraction  Density\n");
	printf("   %s           %s                      %s\n", P_UNIT, T_UNIT, D_UNIT);
};

static void
report()
{
	printf("%7.1f   %10.1f   %12.3f   %5.3f\n",
		scio_convert(pressure, PRESSURE, P_UNIT),
		scio_convert(temp, TEMPERATURE, T_UNIT),
		liquid_fraction,
		scio_convert(density, DENSITY, D_UNIT));
}


/*
 * Loop reducing the pressure, printing out the resultant density, liquid fraction, and temperature.
 */
static void
doit()
{
	double new_le, new_ve;

	temp = temp_from_pressure(pressure);
	energy = liquid_energy(temp);

	for (; pressure > 0.; pressure -= pressure_increment) {
		temp = temp_from_pressure(pressure);
		new_le = liquid_energy(temp);
		new_ve = vapor_energy(temp);
		liquid_fraction = (energy - new_ve) / (new_le - new_ve);
		density = 1. / (liquid_fraction / liquid_density(temp) + 
					(1. - liquid_fraction)/vapor_density(temp));
		report();
	}
}


int
main(int argc, char **argv)
{
	myname = *argv; 

	initialize();
	report_header();
	doit();

	exit(0);
}
