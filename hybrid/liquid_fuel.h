/*
  This file is a portion of Hsim 0.4
 
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

struct liquid_fuel_data_s {
    	double fuel_density;
	int cpropep;	/* what line in propellant.dat ? */
};

/*
 * Returns 0 on success, non-zero on error.
 *
 * All errors have already generated a message to stderr.
 *
 * On successful return the structure pointed to by fp has been filled in.
 * 
 */
int liquid_fuel_data(char *liquid_fuel_name, struct liquid_fuel_data_s *fp);
