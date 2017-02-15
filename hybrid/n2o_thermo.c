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
 * These routines look up or calculate the thermodynamic properties of N2O.
 *
 * All of these routine assume the n2o is on its saturation curve.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "state.h"
#include "linkage.h"
#include "rsim.h"

/*
 * Define the first thermo data file
 */
#define	THERMODAT_1	"n2osaturation.csv"

char *columns_1[] ={
	"Temperature (K)",
	"Pressure (MPa)",
	"Density (mol/l)",
	"Internal Energy (kJ/mol)",
	"Enthalpy (kJ/mol)",
	"Entropy (J/mol*K)",
	"Cv (J/mol*K)",
	"Cp (J/mol*K)",
	"Sound Spd. (m/s)",
	"Joule-Thomson (K/MPa)",
};

#define	NCOL_1	((sizeof columns_1)/(sizeof columns_1[0]))


#define	TEMP		0
#define	VAPOR_PRESSURE	1
#define	VAPOR_DENSITY	2
#define	VAPOR_ENERGY	3
#define	VAPOR_ENTHALPY	4
#define	Cv		6
#define	Cp		7
#define	SOUND_SPEED	8

/*
 * Define the second thermo data file.
 */

#define	THERMODAT_2	"n2oliquid.csv"

char *columns_2[] = {
	"Temp",
	"Pressure",
	"Liquid Density",
	"Liquid Energy",
	"Liquid Enthalpy",
};

#define	LIQUID_DENSITY	2
#define	LIQUID_ENERGY	3
#define	LIQUID_ENTHALPY	4

#define	NCOL_2	((sizeof columns_2)/(sizeof columns_2[0]))


#define	MAX_THERMO	100

static void *vapor_pressure_ic;
static void *vapor_density_ic;
static void *vapor_energy_ic;
static void *liquid_density_ic;
static void *liquid_energy_ic;
static void *saturation_temp_ic;
static void *Cv_ic;
static void *Cp_ic;
static void *SoundSpeed_ic;


extern char *myname;

static void
n2o_thermo_init_1()
{
	int i, j, n;
	FILE *input;
	char buffer[256];
	char *ptrs[NCOL_1];
	int vapor_energy_col;

	static double	n2o_temp[MAX_THERMO];
	static double	n2o_vapor_pressure[MAX_THERMO];
	static double	n2o_vapor_density[MAX_THERMO];
	static double	n2o_vapor_energy[MAX_THERMO];
	static double	n2o_Cv[MAX_THERMO];
	static double	n2o_Cp[MAX_THERMO];
	static double	n2o_SoundSpeed[MAX_THERMO];

	vapor_energy_col = use_enthalpy? VAPOR_ENTHALPY: VAPOR_ENERGY;

	if (n2o_mols_per_kg == 0.) {
		fprintf(stderr, "%s: n2o_therm_init error\n", myname);
		error_exit(1);
	}

	input = fopen(THERMODAT_1, "r");
	if (input == NULL) {
		fprintf(stderr, "%s: cannot open data file %s for reading.\n",
			myname, THERMODAT_1);
		perror("open");
		error_exit(1);
	}

	for (i = -1; ; i++) {
		n = csv_read(input, buffer, sizeof buffer, ptrs, NCOL_1);
		if (n == 0)
			break;
		if (i >= MAX_THERMO) {
			fprintf(stderr, "%s: more than %d lines "
					"in data file %s\n",
				myname, MAX_THERMO, THERMODAT_1);
			error_exit(1);
		}

		/* validate column headings */
		if (i < 0) {
		    for (j = 0; j < NCOL_1; j++)
			if (strcmp(ptrs[j], columns_1[j]) != 0) {
			    fprintf(stderr, "%s: bad column heading in "
			    			"data file %s\n",
				myname, THERMODAT_1);
			    fprintf(stderr, "\tExpected \'%s\' got \'%s\' "
			    			"in column %d\n",
				columns_1[j], ptrs[j], j);
			    error_exit(1);
			}
			continue;
		}

		n2o_temp[i] = atof(ptrs[TEMP]);

		/* convert from MPa to pascal */
		n2o_vapor_pressure[i] = atof(ptrs[VAPOR_PRESSURE]) * 1000000;

		/* convert from mols/liter to kg/meter**3 */
		n2o_vapor_density[i] = atof(ptrs[VAPOR_DENSITY]) /
			n2o_mols_per_kg * 1000.;
		
		/* convert from kJ/mol to J/kg */
		n2o_vapor_energy[i] = atof(ptrs[vapor_energy_col]) *
			n2o_mols_per_kg * 1000.;

		/* convert from  J/mol*K  to J/kg*K */
		n2o_Cv[i] = atof(ptrs[Cv]) * n2o_mols_per_kg;

		/* convert from  J/mol*K  to J/kg*K */
		n2o_Cp[i] = atof(ptrs[Cp]) * n2o_mols_per_kg;

		/* sound is in mtrs /sec, which is good */
		n2o_SoundSpeed[i] = atof(ptrs[SOUND_SPEED]);
	}
	fclose(input);

	n = i;

	vapor_pressure_ic = interpolate_1d_context(
				n2o_temp, n2o_vapor_pressure, n);

	saturation_temp_ic = interpolate_1d_context(
				n2o_vapor_pressure, n2o_temp, n);

	vapor_density_ic = interpolate_1d_context(
				n2o_temp, n2o_vapor_density, n);

	vapor_energy_ic = interpolate_1d_context(
				n2o_temp, n2o_vapor_energy, n);

	Cv_ic = interpolate_1d_context(
				n2o_temp, n2o_Cv, n);

	Cp_ic = interpolate_1d_context(
				n2o_temp, n2o_Cp, n);

	SoundSpeed_ic = interpolate_1d_context(
				n2o_temp, n2o_SoundSpeed, n);

}

static void
n2o_thermo_init_2()
{
	int i, j, n;
	int liquid_energy_col;
	FILE *input;
	char buffer[256];
	char *ptrs[NCOL_2];

	static double	n2o_temp[MAX_THERMO];
	static double	n2o_liquid_density[MAX_THERMO];
	static double	n2o_liquid_energy[MAX_THERMO];

	liquid_energy_col = use_enthalpy? LIQUID_ENTHALPY: LIQUID_ENERGY;

	input = fopen(THERMODAT_2, "r");
	if (input == NULL) {
		fprintf(stderr, "%s: cannot open data file %s for reading.\n",
			myname, THERMODAT_2);
		perror("open");
		error_exit(1);
	}

	for (i = -1; ; i++) {
		n = csv_read(input, buffer, sizeof buffer, ptrs, NCOL_2);
		if (n == 0)
			break;
		if (i >= MAX_THERMO) {
			fprintf(stderr, "%s: more than %d lines "
					"in data file %s\n",
				myname, MAX_THERMO, THERMODAT_2);
			error_exit(1);
		}

		/* validate column headings */
		if (i < 0) {
		    for (j = 0; j < NCOL_2; j++)
			if (strcmp(ptrs[j], columns_2[j]) != 0) {
			    fprintf(stderr, "%s: bad column heading in "
			    			"data file %s\n",
				myname, THERMODAT_2);
			    fprintf(stderr, "\tExpected \'%s\' got \'%s\' "
			    			"in column %d\n",
				columns_2[j], ptrs[j], j);
			    error_exit(1);
			}
			continue;
		}

		n2o_temp[i] = atof(ptrs[TEMP]);

		/* convert from mols/liter to kg/meter**3 */
		n2o_liquid_density[i] = atof(ptrs[LIQUID_DENSITY]) /
			n2o_mols_per_kg * 1000.;
		
		/* convert from kJ/mol to J/kg */
		n2o_liquid_energy[i] = atof(ptrs[liquid_energy_col]) *
			n2o_mols_per_kg * 1000.;
	}
	fclose(input);

	n = i;

	liquid_density_ic = interpolate_1d_context(
				n2o_temp, n2o_liquid_density, n);

	liquid_energy_ic = interpolate_1d_context(
				n2o_temp, n2o_liquid_energy, n);
}

void
n2o_thermo_init()
{
	n2o_thermo_init_1();
	n2o_thermo_init_2();
}


/*
 * Interpolate, handling errors from the interpolation function.
 */
static double
i_i(double x, void *ic)
{
	int r;
	double y;

	r = interpolate_1d(x, &y, ic);
	if (r < 0)
		n2o_thermo_error = r;

	return y;
}

/*
 * Saturation point liquid density.
 */
double
liquid_density(double temp)
{
	return i_i(temp, liquid_density_ic);
}

/*
 * Saturation point vapor density.
 */
double
vapor_density(double temp)
{
	return i_i(temp, vapor_density_ic);
}

/*
 * Saturation point pressure
 */
double
saturation_pressure(double temp)
{
	return i_i(temp, vapor_pressure_ic);
}

/*
 * Liquid phase energy
 * in joules/kg
 */
double
liquid_energy(double temp)
{
	return i_i(temp, liquid_energy_ic);
}

/*
 * Vapor phase energy
 * in joules/kg
 */
double
vapor_energy(double temp)
{
	return i_i(temp, vapor_energy_ic);
}

/*
 * Ratio of specific heats
 */
double
cpcv(double temp)
{
	return i_i(temp, Cp_ic) / i_i(temp, Cv_ic);
}

/*
 * Speed of sound in m/s at temp
 */
double
sound_speed(double temp)
{
	return i_i(temp, SoundSpeed_ic);
}

double
temp_from_pressure(double pressure)
{
	return i_i(pressure, saturation_temp_ic);
}
