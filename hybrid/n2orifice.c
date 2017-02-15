/*
 * Simulate N2O gas at saturated conditions flowing through an orifice.
 * Algorithm:
 *   * Find the upstream condidtions.  inputs:
 * 	 - 100% gas
 * 	 - saturation temperature at stated pressure
 * 	 outputs:
 * 	 - density
 * 	 - temperature
 * 	 - enthalpy
 * 	 - entropy
 *   * Numerically solve for orifice conditions
 *	 - Entropy is constant
 *	 - Enthalpy plus kinetic energy = upstream enthalpy
 *	 KE = 1/2 mv**2.  v = speed of sound under orifice conditions
 *	 - need to iterate to find resultant density and speed of sound
 *	?? What about P*V energy terms?
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "ts_parse.h"
#include "scio.h"
#include "linkage.h"
#include "state.h"

#define	P_UNIT	"psi"
#define	V_UNIT	"liter"
#define	T_UNIT	"seconds"
#define	A_UNIT	"cm2"


double up_pressure;		// upstream pressure, gage, before the orifice
double down_pressure;		// downstream pressure, gage, after the orifice

struct scio_input_parameter_s scio_input[] = {
{
	"pressure_up",
	PRESSURE,
	REQUIRED,
	&up_pressure,
	1,
	0,
},
{
	"pressure_down",
	PRESSURE,
	REQUIRED,
	&down_pressure,
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
	up_pressure += atmosphere_pressure;
	down_pressure += atmosphere_pressure;
}

static double up_density;
static double up_temp;
static double up_enthalpy;
static double up_entropy;

static void
upstream()
{
	up_temp = temp_from_pressure(up_pressure);
	up_density = vapor_density(up_temp);
	up_enthalpy = vapor_energy(up_temp);
	up_entropy = vapor_entropy(up_temp);
}

static void
doit()
{
}

int
main(int argc, char **argv)
{
	myname = *argv; 

	initialize();
	upstream();
	doit();

	exit(0);
}
