/*
  This file is a portion of Hsim 0.1
 
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
 * Scientific I/O routines.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ts_parse.h"
#include "scio.h"
#include "rsim.h"


/*
 * Table of Known Units.
 *
 * NOTE: unit names must be case insensitive and unique within type.
 */

struct unit_s {
	int	type;
	char *	name;
	double	multiply;
	double	add;
} unit_defines[] = {
	{NUMBER,	"",	1.,		0.,	},	/* unitless */

	{LENGTH,	"meter",1.,		0.,	},	/* meter */
	{LENGTH,	"m",	1.,		0.,	},	/* meter */
	{LENGTH,	"cm",	0.01,		0.,	},	/* centimeter */
	{LENGTH,	"mm",	0.001,		0.,	},	/* millimeter */
	{LENGTH,	"in",	0.0254,		0.,	},	/* inch */
	{LENGTH,	"ft",	0.3048,		0.,	},	/* foot */
	{LENGTH,	"km",	1000.,		0.,	},	/* kilometer */

	{AREA,		"m2",	1.,		0.,	},	/* meter ** 2 */
	{AREA,		"cm2",	0.0001,		0.,	},
	{AREA,		"in2",	0.00064516,	0.,	},
	{AREA,		"ft2",	0.09290304,	0.,	},

	{VOLUME,	"m3",	1.,		0.,	},	/* meter ** 3 */
	{VOLUME,	"cc",	0.000001,	0.,	},
	{VOLUME,	"in3",	1.6387064e-05,	0.,	},
	{VOLUME,	"liter",.001,		0.,	},
	{VOLUME,	"cup",	0.000236588,	0.,	},

	{FORCE,		"N",	1.,		0.,	},	/* Newton */
	{FORCE,		"lbf",	4.4482216,	0.,	},
	{FORCE,		"lb",	4.4482216,	0.,	},

	{MASS,		"kg",	1.,		0.,	},	/* kilogram */
	{MASS,		"gm",	1000.,		0.,	},
	{MASS,		"lbm",	0.45359237,	0.,	},
	{MASS,		"lbs",	0.45359237,	0.,	},

	{PRESSURE,	"pascal", 1.,		0.,	},
	{PRESSURE, 	"mpa",	1000000.,	0.,	},
	{PRESSURE,	"psi",	6894.7573,	0.,	},
	{PRESSURE,	"atm",	101325.,	0.,	},
	{PRESSURE,	"bar",	100000.,	0.,	},

	{TEMPERATURE,	"K",	1.,		0.,	},	/* degrees K */
	{TEMPERATURE,	"C",	1.,		273.15,	},
	{TEMPERATURE,	"F",	0.55555556,	255.373,},

	{ENERGY,	"joule",1.,		0.,	},
	{ENERGY,	"J",	1.,		0.,	},
	{ENERGY,	"erg",	1e-07,		0.,	},
	{ENERGY,	"ftlb",	1.3558179,	0.,	},
	{ENERGY,	"cal",	4.1868,		0.,	},

	{VELOCITY,	"m/s",	1.,		0.,	},	/* meters/sec */
	{VELOCITY,	"kps",	1000.,		0.,	},
	{VELOCITY,	"mph",	0.44704,	0.,	},
	{VELOCITY,	"fps",	0.3048,		0.,	},
	{VELOCITY,	"in/sec",0.0254,	0.,	},

	{ACCELERATION,	"m/sec2",1.,		0.,	},
	{ACCELERATION,	"ft/sec2",.3048,	0.,	},
	{ACCELERATION,	"g",	9.80665,	0.,	},

	{DENSITY,	"kg/m3",1.,		0.,	},
	{DENSITY,	"g/cc",	1000.,		0.,	},
	{DENSITY,	"lb/ft3",16.018463,	0.,	},
	{DENSITY,	"lbm/ft3",16.018463,	0.,	},

	{MASSFLOW,	"kg/sec",1.,		0.,	},
	{MASSFLOW,	"lb/sec",0.45359237,	0.,	},
	{MASSFLOW,	"lbm/sec",0.45359237,	0.,	},

	{MASSFLUX,	"kg/sec/m/m",1.,	0.,	},
	{MASSFLUX,	"lb/sec/in/in",703.06958,0.,	},

	{TIME,		"sec",	1.,		0.,	},
	{TIME,		"s",	1.,		0.,	},

	{ANGLE,		"degree", .01745329252, 0.,	},
	{ANGLE,		"degrees",.01745329252, 0.,	},
	{ANGLE,		"radian",1.,		0.,	},

	{0,		 (char *)0, 1.,		0.,	},	/* END MARKER */
};

extern char *myname;
static int n_params;
static struct scio_input_parameter_s *param_p;
static int input_errors;

static double
u_conv(struct unit_s *up, double v)
{
	return v * up->multiply + up->add;
}

/* inverse conversion */
static double
i_conv(struct unit_s *up, double v)
{
	return (v - up->add) / up->multiply;
}

static double
get_val(struct ts_parsed_s *buffer, int which)
{
	double r;
	char *tail;

	r = strtod(buffer->words[which], &tail);
	
	if (tail == buffer->words[which] || *tail != '\0') {
		fprintf(stderr, "%s: parameter %s: \"%s\" is not a number\n",
			myname, buffer->words[0], buffer->words[which]);
		input_errors++;
	}

	return r;
}


void
scio_init(struct scio_input_parameter_s *sp, int n)
{
	int i;

	/*
	 * Save the input parameters
	 */
	n_params = n;
	param_p = sp;
	input_errors = 0;


	/*
	 * Zero out the number found.
	 */
	for (i = 0; i < n_params; i++, sp++) {
		/*
		 * If the requestor has not allocated space
		 * to hold the number found, then do so now.
		 */
		if (!(sp->nvp)) {
			sp->nvp = (int *)malloc(sizeof (int));
			if (!sp->nvp) {
				fprintf(stderr, "%s: cannot malloc %ld bytes\n",
					myname, sizeof (int));
				exit(1);
			}
		}
		*(sp->nvp) = 0;
	}
}

/*
 * Check for missing required parameters, free memory.
 */
void
scio_term()
{
	int i;

	for (i = 0; i < n_params; i++)
		if ((param_p[i].options & REQUIRED) &&
		     *(param_p[i].nvp) == 0) {
			fprintf(stderr, "%s: required parameter %s "
					"was not specified\n",
				myname,
				param_p[i].name);
			input_errors++;
		}
	if (input_errors) {
		fprintf(stderr, "%s: exiting on input errors.\n", myname);
		exit(1);
	}
}

/*
 * Any errors are reported.
 * When scio_term is called the program will exit if there were
 * any errors.
 */
void
scio_input_line(struct ts_parsed_s *buffer)
{
	int i, j, k;
	struct unit_s *up;
	struct scio_input_parameter_s *ip;
	double min, max, incr, fsteps;
	int isteps;

	/* ignore blank lines */
	if (!buffer->words[0])
		return;

	/* is the input line one of our parameters ? */
	for (i = 0, ip = param_p; i < n_params; i++, ip++)
		if (strcasecmp(buffer->words[0], ip->name) == 0)
			break;

	if (i >= n_params) {
		fprintf(stderr, "%s: unknown parameter name: %s\n",
			myname, buffer->words[0]);
		input_errors++;
		return;
	}

	/* count how many words in the input line. */
	for (j = 0; buffer->words[j]; j++)
		;

	/* the last word is the unit.  Specified?  Matches?  */
	if (ip->unit == NUMBER || ip->unit == STRING) {
		up = unit_defines;
		j++;		/* no unit specified or expected */
	} else {
		for (up = unit_defines; up->type; up++)
			if (ip->unit == up->type &&
			    strcasecmp(up->name, buffer->words[j-1]) == 0)
				break;	/* found */
		if (up->type == 0) {
			fprintf(stderr, "%s: parameter %s specified in unknown "
					"or incompatible unit %s\n", 
				myname, buffer->words[0], buffer->words[j-1]);
			input_errors++;
			return;
		}
	}

	/*
	 * At this point we are processing parameter #i, pointed to by ip.
	 * The input line has j words in it.
	 * The unit definition is valid and up points to it.
	 */

	if (*(ip->nvp) > ip->nv)
		return;	/* no sense in repeating error... */

	/*
	 * The input line can have one of three forms.
	 * In all cases the first word is the parameter name and the last
	 * is the unit name.  We've processed those.
	 * The middle is either a single number, a list of numbers, or
	 * an interated range specified as (lower, upper, STEP, increment).
	 * This form has exactly 4 tokens with the third the literal "STEP".
	 */
	if (ip->unit != STRING &&
	    j == 6 && strcasecmp(buffer->words[3], "step") == 0) {
		/* for-loop construct */
		min = get_val(buffer, 1);
		max = get_val(buffer, 2);
		incr = get_val(buffer, 4);
		fsteps = (max - min) / incr;
		isteps = (int)fsteps;
		if ((double)isteps == fsteps)
			isteps++;
		*(ip->nvp) += isteps;
		if (*(ip->nvp) > ip->nv) {
			fprintf(stderr, "%s: found %d values for "
					"parameter %s, limit is %d\n",
				myname,
				*(ip->nvp),
				buffer->words[0],
				ip->nv);
			input_errors++;
			return;
		}

		for (k = 0; k < isteps; k++)
			((double *)(ip->vp))[k + *(ip->nvp) - isteps] =
				u_conv(up, min + (double)k * incr);
	} else {
		/* simple list format */
		*(ip->nvp) += j - 2;
		if (*(ip->nvp) > ip->nv) {
			fprintf(stderr, "%s: found %d values for "
					"parameter %s, limit is %d\n",
				myname,
				*(ip->nvp),
				buffer->words[0],
				ip->nv);
			input_errors++;
			return;
		}
		for (k = 1; k < j - 1; k++)
		    if (ip->unit == STRING)
			((char **)(ip->vp))[k - 1 + *(ip->nvp) - (j - 2)] =
				ds_copy(buffer->words[k]);
		    else
			((double *)(ip->vp))[k - 1 + *(ip->nvp) - (j - 2)] =
				u_conv(up, get_val(buffer, k));
	}
}

/*
 * Convert a value to another unit.
 * Usually used for output formatting.
 */
double
scio_convert(double v, int unit_type, char *unit_name)
{
	struct unit_s *up;

	for (up = unit_defines; up->type; up++)
		if (up->type == unit_type &&
		    strcasecmp(up->name, unit_name) == 0)
			break;	/* found */

	if (up->type == 0) {
		fprintf(stderr, "%s: conversion specified in unknown "
				"unit %s\n", 
			myname, unit_name);
		exit(1);
	}

	return i_conv(up, v);
}

/*
 * Convert a number that has a unit to the appropriate internal value.
 */
double
scio_f_convert(double v, int unit_type, char *unit_name)
{
	struct unit_s *up;

	for (up = unit_defines; up->type; up++)
		if (up->type == unit_type &&
		    strcasecmp(up->name, unit_name) == 0)
			break;	/* found */

	if (up->type == 0) {
		fprintf(stderr, "%s: conversion specified in unknown "
				"unit %s\n", 
			myname, unit_name);
		exit(1);
	}

	return u_conv(up, v);
}
