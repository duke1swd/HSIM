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

#include <stdio.h>
#include "rsim.h"

int
main()
{
	int i, j;

	char buffer[256];
	char *ptrs[20];

	i = csv_read(stdin, buffer, sizeof buffer, ptrs, 20);

	printf("csv read returns %d\n", i);

	for (j = 0; j < i; j++)
		printf("%2d: %s\n", j, ptrs[j]);
	return 0;
}
