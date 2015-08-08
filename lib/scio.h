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
 * This module provides I/O for scientific calculations.
 * The primary function is to manage unit conversions.
 */

/*
 * These are the types of physical quanitites supported.
 */
#define	NUMBER		1	/* unitless */
#define	LENGTH		2
#define	AREA		3
#define	VOLUME		4
#define	FORCE		5
#define	MASS		6
#define	PRESSURE	7
#define	TEMPERATURE	8
#define	ENERGY		9
#define	VELOCITY	10
#define	ACCELERATION	11
#define	DENSITY		12
#define	MASSFLOW	13
#define	TIME		14
#define	MASSFLUX	15
#define	ANGLE		16

#define	STRING		100

#define	REQUIRED	0x01

/*
 * This structure defines a requested input.
 * An array of such structures is handed to the input processing routine.
 */
struct scio_input_parameter_s {
	char *name;		/* Name of the parameter */
	int  unit;		/* One of the above types */
	int  options;		/* bitwise OR of the option flags */
	void *vp;		/* Where to stash the value. */
	int nv;			/* maximum number of such values allowed */
	int *nvp;		/* locn to place the number found */
};

/*
 * Main input routine.
 *
 * Input lines are handed in, as parsed by ts_parse.c
 *
 * Each input line contains a name, a value (or range of values)
 * and then a unit specifier.
 *
 * The range of values might be a single number,
 * A list of numbers, or 2 numbers, the word "step" and an increment
 *
 * Each number, word, or whatever is separated by whitespace.
 *
 */

/*
 * Define the acceptable input parameters.
 */
void
scio_init(struct scio_input_parameter_s *sp, int n);

/*
 * Process an input line.
 * On error an error message is printed.
 * Returns true if an error was encountered.
 */
void
scio_input_line(struct ts_parsed_s *buffer);

/*
 * Done parsing.
 * Perform last few error checks.  Die if any errors have happened.
 */
void
scio_term();

/*
 * Convert a value to another unit.
 */
double scio_convert(double v, int unit_type, char *unit_name);
double scio_f_convert(double v, int unit_type, char *unit_name);
