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
#define	TE_UNIT	"F"

#define	N2O_MOLAR_MASS	0.044013	// per wikipedia, in kg/mole


double up_pressure;		// upstream pressure, gage, before the orifice
double down_pressure;		// downstream pressure, gage, after the orifice
double orifice_diameter;	// size of the orifice

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
{
	"orifice_diameter",
	LENGTH,
	REQUIRED,
	&orifice_diameter,
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
	n2o_thermo_init();
}

static double up_density;
static double up_temp;
static double up_enthalpy;
static double up_entropy;
static double down_density;
static double down_temp;
static double down_mass_flow;

static void
upstream()
{
	up_temp = temp_from_pressure(up_pressure);
	up_density = vapor_density(up_temp);
	up_enthalpy = vapor_energy(up_temp);
	up_entropy = vapor_entropy(up_temp);
}

static void
downstream()
{
	double speed;
	double area;

	down_temp = down_temp;
	down_density = vapor_density(down_temp);
	speed = sound_speed(down_temp);
	area = pi * orifice_diameter * orifice_diameter / 4.;
	down_mass_flow = speed * area * down_density;
}

/*
 * finds a temp where energy is conserved.
 */
static void
doit()
{
	int iteration;
	double t;
	double enthalpy;
	double ke;
	double temp = 210.;	// degrees K.  This is as cold as our data goes
	double old_temp;

	// KE = 1/2 M V**2
	n2o_thermo_error = 0;
	old_temp = temp;
	for (iteration = 0; iteration < 1000; iteration++) {
		old_temp = temp;
		ke = sound_speed(temp);
		ke = .5 * N2O_MOLAR_MASS * ke * ke;
		if (n2o_thermo_error) {
			fprintf(stderr, "%s: sound_speed thermo error %d on iteration %d\n",
				myname,
				n2o_thermo_error,
				iteration);
			break;
		}
		enthalpy = up_enthalpy - ke;
		temp = temp_from_vapor_energy(enthalpy);
		if (n2o_thermo_error) {
			fprintf(stderr, "%s: temp_from_vapor_energy thermo error %d on iteration %d\n",
				myname,
				n2o_thermo_error,
				iteration);
			break;
		}
		t = temp - old_temp;
		if (t >= .01 && t <= .01)
			break;
	}
	printf("Used %d iterations\n", iteration);
	down_temp = temp;
}
/*
static double up_density;
static double up_temp;
static double up_enthalpy;
static double up_entropy;
static double down_density;
static double down_temp;
static double down_mass_flow;
*/


static void
report()
{
	printf("Upstream temp is %.2fF\n",
		scio_convert(up_temp, TEMPERATURE, TE_UNIT));
	printf("Downstream temp is %.2fF\n",
		scio_convert(down_temp, TEMPERATURE, TE_UNIT));
}

int
main(int argc, char **argv)
{
	myname = *argv; 

	initialize();
	upstream();
	doit();
	downstream();
	report();

	exit(0);
}
