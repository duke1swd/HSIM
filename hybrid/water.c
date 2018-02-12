/*
 * Simulate a cold flow test using pressurized nitrogen (or air) and water
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

#define	MAX_TIME 30.		// cannot take longer than this to drain tank.

double pressure_init;
double pressure_final;
double tank_volume;
double water_volume;
double time_increment;
double total_time;

struct scio_input_parameter_s scio_input[] = {
{
	"pressure_init",
	PRESSURE,
	REQUIRED,
	&pressure_init,
	1,
	0,
},
{
	"pressure_final",
	PRESSURE,
	REQUIRED,
	&pressure_final,
	1,
	0,
},
{
	"tank_volume",
	VOLUME,
	REQUIRED,
	&tank_volume,
	1,
	0,
},
{
	"water_volume",
	VOLUME,
	REQUIRED,
	&water_volume,
	1,
	0,
},
{
	"water_density",
	DENSITY,
	0,
	&lfueldensity,
	1,
	0,

},
{
	"time_total",
	TIME,
	REQUIRED,
	&total_time,
	1,
	0,
},
{
	"time_increment",
	TIME,
	0,
	&time_increment,
	1,
	0,
},
{
	"injector_area",
	AREA,
	REQUIRED,
	&liquid_injector_area,
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
	time_increment = 1./5000.;
	lfueldensity = 1000.;  // kg per cubic meter

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
	pressure_init += atmosphere_pressure;
	pressure_final += atmosphere_pressure;
}

double time;
double pressure;


/*
 * Calculates how long it takes for the tank to empty.
 */
static double
sim1(double cd)
{
	int i;
	int iterations;
	extern double lf_pvgamma;

	iterations = MAX_TIME / time_increment;

	chamber_pressure = atmosphere_pressure;
	nitrogen_pressure_initial = pressure_init;
	nitrogen_pressure = nitrogen_pressure_initial;
	nitrogen_gamma = 1.41;
	lfuelvolume = water_volume;
	lfuelmass = water_volume * lfueldensity;
	lfueltankvolume = tank_volume;
	lf_pvgamma = nitrogen_pressure *
		pow(lfueltankvolume - lfuelvolume, nitrogen_gamma);
	lfuelinjector_count = 1;
	lfuelinjectorcd = cd;

	/*
	 * Simulate the tank draining.
	 */
	for (i = 0; i < iterations; i++) {
		/*
		 * Calculate fuel flow rate
		 */
		liquid_injector();

		/*
		 * Update nitrogen pressure and water mass.
		 */
		if (!liquid_step(time_increment))
			break;
	}
	return i * time_increment;
}

static double
calc_cd()
{
	double low, high;
	double t;
	double guess;

	low = .00001;
	high = 2.;

	if (total_time >= MAX_TIME) {
		fprintf(stderr, "requested time (%.1f) is greater than allowed (%.1f)\n",
			total_time,
			MAX_TIME);
		return -1.;
	}

	if (sim1(high) > total_time) {
		fprintf(stderr, "total time (%.1f) is too long.\n", total_time);
		return -1.;
	}

	while (high - low >= 1./1024.) {
		guess = (high + low) / 2.;
		t = sim1(guess);
		if (t == total_time)
			break;
		if (t < total_time)
			high = guess;
		else
			low = guess;
	}
	return guess;
}

static double
sim_vol()
{
	double cd;
	double high, low;
	double t;


	t = tank_volume;
	high = tank_volume * 20.;
	low = tank_volume / 2.;

	while (high - low >= t/1024.) {
		tank_volume = (high + low) / 2.;
		cd = calc_cd();
		if (nitrogen_pressure == pressure_final)
			break;
		else if (nitrogen_pressure < pressure_final)
			low = tank_volume;
		else
			high = tank_volume;
	}
	return cd;
}

static void
doit()
{
	double cd;
	double vr;

	cd = calc_cd();
	if (cd < 0.)
		return;

	printf("NOTE: All pressures a GAUGE pressures\n\n");
	printf("Constant Pressure Calculated Cd is %.3f\n",
		water_volume/(liquid_injector_area * lfuelinjector_count) *
			sqrt(lfueldensity/(pressure_init + pressure_final - 2 * atmosphere_pressure))/total_time);

	printf("At measured tank volume of %.1f %s\n",
		 scio_convert(tank_volume, VOLUME, V_UNIT), V_UNIT);
	printf("\tSimulated Injector Cd is %.3f\n", cd);
	printf("\tFinal Presure (predicted) is %.2f\n", 
		scio_convert(nitrogen_pressure-atmosphere_pressure, PRESSURE, P_UNIT));
	printf("\tFinal Presure (measured) is %.2f\n", 
		scio_convert(pressure_final-atmosphere_pressure, PRESSURE, P_UNIT));

	vr = exp(log(pressure_init/pressure_final)/nitrogen_gamma);
	tank_volume = vr * water_volume / (vr - 1.);

	cd = calc_cd();
	if (cd < 0.)
		return;
	printf("At calculated volume of %.3f %s\n", 
		scio_convert(tank_volume, VOLUME, V_UNIT), V_UNIT);
	printf("\tSimulated Injector Cd is %.3f\n", cd);
}


int
main(int argc, char **argv)
{
	myname = *argv; 

	initialize();
	doit();

	exit(0);
}
