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

/*
 * This is the interface to CPROPEP.
 *
 * DYNAMIC INPUTS
 *	chamber_pressure
 *	n2o_flow_rate
 *	fuel_flow_rate
 *
 * STATIC INPUTS
 *	nozzle_throat_area
 *	nozzle_exit_area
 *
 * OUTPUTS
 *	c_star
 *	exit_pressure
 *	nozzle_cf
 */


#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "state.h"
#include "linkage.h"

#define	DATACREATE
#include "cpp.h"

extern char *myname;

static int Nzrx = -1;

static void
init(double Nzr)
{
	int lNzrx;
	int input;
	int r;
	int status;
	pid_t pid;
	char Nzrbuf[32];
	char filename[256];
	char command[512];

	lNzrx = Nzr * 1000. + .5;

	/*
	 * If the data is already loaded, then we are done.
	 */
	if (lNzrx == Nzrx)
		return;

	/*
	 * Validate input parameter within legal range.
	 */
	if (Nzr < 1. || Nzr > 8.) {
		fprintf(stderr, "%s: Nozzle Ratio %.3f is "
				"smaller than 1. or greater than 8.\n",
			myname, Nzr);
		error_exit(1);
	}

	if (0 && Nzrx > 0) {
		fprintf(stderr, "%s: Warning: Changing Nozzle Ratio to %.4f\n",
			myname, Nzr);
	}

	Nzrx = lNzrx;
	
	/*
	 * Find the data, creating it if necessary.
	 */
	sprintf(filename, "%s/%s.Nzr.%d", CPROPEPDATA, fuel, Nzrx);
	if ((input = open(filename, O_RDONLY)) < 0) {

		/*
		 * Data not found.  Create it.
		 */
		switch (ok_to_create_nzr) {
		    case NZR_CREATE_SYSTEM:
			errno = 0;
			fprintf(stderr, "%s: Warning: Creating Nozzle Ratio "
				"data for %.4f (%s)\n",
				myname, Nzr, filename);
			sprintf(command, "%s %s %f %s",
				CREATENZR, filename, Nzr, fuel);
			system(command);
			break;

		    case NZR_CREATE_EXEC:
			fprintf(stderr, "%s: Warning: Creating Nozzle Ratio "
				"data for %.4f (%s)\n",
				myname, Nzr, filename);

			sprintf(Nzrbuf, "%f", Nzr);

			pid = vfork();
			if (pid == -1) {
				fprintf(stderr, "%s: cannot vfork\n", myname);
				error_exit(1);
			}
			if (pid == 0) {
				/* child */
				execl(CREATENZR,
					CREATENZR,
					"-e",
					filename,
					Nzrbuf,
					fuel,
					(char *)0);
				fprintf(stderr, "%s: execl of %s failed\n",
					myname, CREATENZR);
				perror("execl");
				exit(1);
			}

			/* parent */
			r = waitpid(pid, &status, 0);
			if (r == -1) {
				fprintf(stderr, "%s: waitpid failed\n",
					myname);
				perror("waitpid");
				error_exit(1);
			}

			if (!WIFEXITED(status) || WEXITSTATUS(status)) {
				fprintf(stderr,
					"%s: %s failed to run normally\n",
					myname, CREATENZR);
				if (WIFEXITED(status))
					fprintf(stderr, "\tExit status was %d",
						WEXITSTATUS(status));
				error_exit(1);
			}

			break;


		    case NZR_CREATE_NONE:
		    default:
			fprintf(stderr, "%s: Need Nozzle Ratio data file %s\n",
					myname, filename);
			fprintf(stderr, "\tRerun with -N to create.\n");
			error_exit(1);
		}

		/* File should be there now, try again. */

		if ((input = open(filename, O_RDONLY)) < 0) {
			fprintf(stderr,
				"%s: Nozzle Data create failed\n",
				myname);
			perror("open");
			error_exit(1);
		}
	}
	
	/*
	 * Read the data.
	 */
	if (read(input, &data, sizeof data) != sizeof data) {
		fprintf(stderr, "%s: reading data from %s failed.\n",
			myname, filename);
		perror("read");
		error_exit(1);
	}

	/*
	 * Done.
	 */
	close(input);
}

/*
 * See Nummerical Recipes, page 105.
 */
static double
interpolate(double of, double cp, double (* value)(int j, int k))
{
	int j, k;
	double t, u;
	double y1, y2, y3, y4;
	
	if (of < OFvector[0])
		of = OFvector[0];
	if (of > OFvector[N_OF - 1])
		of = OFvector[N_OF - 1];

	if (cp < CPvector[0])
		cp = CPvector[0];
	if (cp > CPvector[N_CP - 1])
		cp = CPvector[N_CP - 1];

	for (j = 1; j < N_OF-2; j++)
		if (of < OFvector[j])
			break;
	j--;

	for (k = 1; k < N_CP-2; k++)
		if (cp < CPvector[k])
			break;
	k--;

	t = (of - OFvector[j]) / (OFvector[j+1] - OFvector[j]);
	u = (cp - CPvector[k]) / (CPvector[k+1] - CPvector[k]);

	y1 = (*value)(j, k);
	y2 = (*value)(j+1, k);
	y3 = (*value)(j+1, k+1);
	y4 = (*value)(j, k+1);

	return (1 - t) * (1 - u) * y1 +
		t * (1 - u) * y2 + 
		t * u * y3 +
		(1 - t) * u * y4;
}

#ifdef notused

static double
Isp_value(int i, int j)
{
	return data[i][j].Isp;
}

static double
cpp_Isp(double OF, double Cp, double Nzr)
{
	init(Nzr);
	return interpolate(OF, Cp, &Isp_value);
}

#endif

static double
Cs_value(int i, int j)
{
	return data[i][j].Cs;
}

static double
cpp_Cs(double OF, double Cp, double Nzr)
{
	init(Nzr);
	return interpolate(OF, Cp, &Cs_value);
}

static double
Cf_value(int i, int j)
{
	return data[i][j].Cf;
}

static double
cpp_Cf(double OF, double Cp, double Nzr)
{
	init(Nzr);
	return interpolate(OF, Cp, &Cf_value);
}

static double
Ep_value(int i, int j)
{
	return data[i][j].Ep;
}

static double
cpp_Ep(double OF, double Cp, double Nzr)
{
	init(Nzr);
	return interpolate(OF, Cp, &Ep_value);
}

void
cpropep()
{
	double cp, of, nzr;

	/* inputs */
	nzr = nozzle_exit_area / nozzle_throat_area;
	of = n2o_flow_rate / fuel_flow_rate;
	cp = chamber_pressure * 0.00014503774;	// convert from pascal to psi.

	/* outputs */
	c_star = cpp_Cs(of, cp, nzr);
	c_star *= 0.3048;		/* convert from ft/sec to m/sec */

	exit_pressure = cpp_Ep(of, cp, nzr);
 	exit_pressure *= 101325.;	/* convert from ATM to Pascal */

	nozzle_cf = cpp_Cf(of, cp, nzr);
	/* unitless */
}
