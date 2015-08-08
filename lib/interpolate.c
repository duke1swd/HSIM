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
 * Interpolates and extrapolates.
 *
 * caluclates *y = f(x), where f(x) is defined by an array of x and y values.
 *
 * The arrays are sorted in ascending x[i] order.
 * There are n entries in the arrays.
 *
 * Returns 0 on success, -1 on extrapolation to low x and -2 on extrapolation
 * to high x.
 */

#include <stdio.h>
#include <stdlib.h>

extern char *myname;

struct i_context_s {
	int n;
	double *x_array;
	double *y_array;
};

void *
interpolate_1d_context(double x_array[], double y_array[], int n)
{
	struct i_context_s *ip;

	if (n < 2) {
		fprintf(stderr, "%s: interpolate call error\n",
			myname);
		exit(1);
	}

	ip = (struct i_context_s *)malloc(sizeof (struct i_context_s));

	if (ip == (struct i_context_s *)0) {
		fprintf(stderr, "%s: cannot allocate %ld bytes for "
				"interpolation context\n",
			myname, sizeof (struct i_context_s));
		exit(1);
	}

	ip->n = n;
	ip->x_array = x_array;
	ip->y_array = y_array;

	return (void *)ip;
}

int
interpolate_1d(double x, double *y, void *context)
{

	int i;
	int r;
	double x1, x2, y1,y2;
	struct i_context_s *ip;

	ip = context;

	r = 0;

	if (x < ip->x_array[0]) {
		i = 0;
		r = -1;
	} else if (x > ip->x_array[ip->n-1]) {
		i = ip->n - 2;
		r = -2;
	} else 
		for (i = 0;  x >= ip->x_array[i]; i++)
			;

	/*
	 * x is in the range (x_array[i], x_array[i+1])
	 */

	x1 = ip->x_array[i];
	x2 = ip->x_array[i+1];
	y1 = ip->y_array[i];
	y2 = ip->y_array[i+1];

	*y = y1 + (x - x1)/(x2 - x1) * (y2 - y1);
	return r;
}
