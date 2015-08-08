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
 * Read in a line from a comma-separated-value file.
 *
 * Parameters:
 *	input	An open stdio stream
 *	buffer	a character buffer with enough room to read the line
 *	size	sizeof(buffer)
 *	ptrs	an array of pointers to the delimited strings
 *	n	number of points
 *
 * Returns:
 *	number of values pointed to by the pointers
 *	0 on EOF
 *	-1 on any error.  Errors are printed to stderr.
 */


int csv_read(FILE *input, char *buffer, int size, char **ptrs, int n);


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
 *
 * The context routine returns an interpolation context for use
 * by the actual interpolation routine.
 */

void *interpolate_1d_context(double x_array[], double y_array[], int n);

int interpolate_1d(double x, double *y, void *context);

/*
 * Dynamic string copy.
 */
char *
ds_copy(char *s);

