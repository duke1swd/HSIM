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
#include <strings.h>
#include <stdlib.h>
#include "rsim.h"

extern char *myname;

char *
ds_copy(char *s)
{
	int n;
	char *p;

	n = strlen(s) + 1;
	p = malloc((unsigned) n);
	if (!p) {
		fprintf(stderr, "%s: malloc of %d bytes failed.\n",
			myname, n);
		exit(1);
	}

	strcpy(p, s);
	
	return p;
}
