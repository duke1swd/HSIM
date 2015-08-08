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
 * This program runs cpropep for us.
 *
 * This program is run by system(CREATENZR...
 * First argument is the file name, second is the NZR
 * Third is the fuel
 *
 * This program is invoked by the functions in chem.c
 * when they cannot find the requested data.
 *
 * If the first argument is "-e", then cpropep is directly invoked
 * (rather than through the runcp shell script).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include "fuel.h"
#include "liquid_fuel.h"

#define	DATACREATE
#include "cpp.h"

char *myname;
char *fuel_long_name;
int verbose = 1;
int exec_mode = 0;
int fuel_cpropep;
int nitrous_cpropep;

/*
 * Call the shell script which actually runs cpropep.
 */
static void
cpropep_system(double Nzr, char *fuel)
{
	int i, j;
	struct cpp_s *p;

	int r;
	FILE *f;

	char cmd[1024];
	char tbuf[64];


	sprintf(cmd, "%s \"", RUNCMD);

	for (i = 0; i < N_OF; i++) {
		sprintf(tbuf, " %.0f", OFvector[i] * 10.);
		if (strlen(tbuf) + strlen(cmd) >= sizeof(cmd) - 10) {
			fprintf(stderr, "%s: cmd line too long\n", myname);
			exit(1);
		}
		strcat(cmd, tbuf);
	}

	strcat(cmd, "\" \"");

	for (i = 0; i < N_CP; i++) {
		sprintf(tbuf, " %.0f", CPvector[i]);
		if (strlen(tbuf) + strlen(cmd) >= sizeof(cmd) - 32) {
			fprintf(stderr, "%s: cmd line too long\n", myname);
			exit(1);
		}
		strcat(cmd, tbuf);
	}

	sprintf(tbuf, "\" %f %s %d %d",
		Nzr, fuel, fuel_cpropep, nitrous_cpropep);
	strcat(cmd, tbuf);

	f = popen(cmd, "r");
	if (f == NULL) {
		fprintf(stderr, "%s: cannot run: %s\n",
			myname, cmd);
		exit(1);
	}

	for (i = 0; i < N_OF; i++)
	    for (j = 0; j < N_CP; j++) {
	    	p = &(data[i][j]);

		p->OF = OFvector[i];
		p->Cp = CPvector[j];
		p->Nzr = Nzr;

		r = fscanf(f, "%lf%lf%lf%lf",
			&p->Isp,
			&p->Cs,
			&p->Cf,
			&p->Ep);

		p->Cs *= 100. / 2.54 / 12.;

		if (r != 4) {
			fprintf(stderr, "%s: cannot parse return from: %s\n",
				myname, cmd);
			exit(1);
		}
	    }

	fclose(f);

}

/*
 * Directly call cpropep, bypassing the shell script.
 */

/*
 * remove up to and including the last space, then parse the number.
 */
static double
getnum(char *s)
{
	char *p;

	for ( ; *s; s++)
		if (*s == ' ')
			p = s + 1;
	return atof(p);
}

/*
 * Find a string
 * Must be at least 2 spaces in front of it and at least 2 behind it
 */
static char *
getstr(char *s)
{
	char *p;

	for ( ; *s; s++)
		if (s[0] == ' ' && s[1] == ' ' && s[2] != ' ')
			goto start;
	return (char *)0;

    start:
	s += 2;
	p = s;

	for ( ; *s; s++)
		if (s[0] != ' ' && s[1] == ' ' && s[2] == ' ') {
			s[1] = '\0';
			break;
		}

	return p;
}

static int
parse_data_return(FILE *datareturn, struct cpp_s *p)
{
	int errors;
	int pluck;
	char lbuf[256];

	errors = 0;

	p->Isp = 0.;
	p->Cs = 0.;
	p->Cf = 0.;
	p->Ep = 0.;

	pluck = 0;

	while (fgets(lbuf, sizeof lbuf, datareturn) == lbuf) {
		if (--pluck >= 0) {
			fuel_long_name = getstr(lbuf);
			if (!fuel_long_name)
				continue;
			if (strcmp(fuel_long_name, "Name") == 0)
				continue;
			if (strcmp(fuel_long_name, "NITROUS OXIDE") == 0)
				continue;
			fprintf(stderr, "%s: Using %s as the fuel\n",
				myname, fuel_long_name);
		}

		if (!fuel_long_name && strstr(lbuf, "Propellant composition"))
			pluck = 3;
		if (strstr(lbuf, "Isp/g"))
			p->Isp = getnum(lbuf);
		else if (lbuf[0] == 'C' && lbuf[1] == '*')
			p->Cs = getnum(lbuf);
		else if (lbuf[0] == 'C' && lbuf[1] == 'f')
			p->Cf = getnum(lbuf);
		else if (strstr(lbuf, "Pressure") == lbuf)
			p->Ep = getnum(lbuf);
	}
	
	if (p->Isp == 0.) {
		fprintf(stderr, "%s: missing Isp from cpropep return data\n",
			myname);
		errors++;
	}
	
	if (p->Cs == 0.) {
		fprintf(stderr, "%s: missing Cs from cpropep return data\n",
			myname);
		errors++;
	}
	
	if (p->Cf == 0.) {
		fprintf(stderr, "%s: missing Cf from cpropep return data\n",
			myname);
		errors++;
	}
	
	if (p->Ep == 0.) {
		fprintf(stderr, "%s: missing Ep from cpropep return data\n",
			myname);
		errors++;
	}

	return errors;
}

static void
cpropep_once(struct cpp_s *p, char *fuel)
{
	int r;
	int errors;
	int status;
	pid_t pid;
	int fds[2];
	FILE *cpi;
	FILE *datareturn;
	char filename[256];

	/*
	 * First, create the cpropep input file
	 */

	sprintf(filename, "%s/%s", CPRODIR, CPROTEMP);
	cpi = fopen(filename, "w");
	if (cpi == NULL) {
		fprintf(stderr, "%s: cannot open %s for writing\n",
			myname, filename);
		exit(1);
	}

	fprintf(cpi,
			"Propellant\n"
			"+%d	%d g	#nitrous\n"
			"+%d	10 g	#%s\n"
			"\n"
			"FR\n"
			"+chamber_pressure %d psi\n"
			"+supersonic_area_ratio %f\n"
			"\n",
		nitrous_cpropep,
		(int)(p->OF * 10),
		fuel_cpropep, fuel,
		(int)(p->Cp),
		p->Nzr);
	fclose(cpi);

	/*
	 * Now fork off CPROPEP.
	 */
	r = pipe(fds);  /* a pipe for cpropep's result. */
	if (r < 0) {
		fprintf(stderr, "%s: cannot create pipe to talk to cpropep\n",
			myname);
		perror("pipe");
		exit(1);
	}

	pid = vfork();
	if (pid == -1) {
		fprintf(stderr, "%s: cannot vfork to run cpropep\n", myname);
		perror("vfork");
		exit(1);
	}

	if (pid == 0) {
		/* child */
		/* make fds[1] into stdout. */
		close(fds[0]);
		dup2(fds[1], 1);
		r = chdir(CPRODIR);
		if (r != 0)
			_exit(10);
		execl("cpropep", "cpropep", "-f", CPROTEMP, (char *)0);
		_exit(11);
	}
	/* parent */
	close(fds[1]);

	datareturn = fdopen(fds[0], "r");
	if (datareturn == NULL) {
		fprintf(stderr, "%s: cannot fdopen the return pipe\n", myname);
		exit(1);
	}

	errors = parse_data_return(datareturn, p);

	fclose(datareturn);

	r = waitpid(pid, &status, 0);
	if (r == -1) {
		fprintf(stderr, "%s: waitpid error while waiting for cpropep\n",
			myname);
		perror("waitpid");
		errors = 1;
	}
	if (!WIFEXITED(status)) {
		fprintf(stderr, "%s: cpropep terminated abnormally\n",
			myname);
		errors = 1;
	} else
	    switch(WEXITSTATUS(status)) {
	    case 0:
		break;	/* all is well */
	    case 10:
		fprintf(stderr, "%s: couldn't chdir to %s for cpropep\n",
			myname, CPRODIR);
		errors = 1;
	    case 11:
		fprintf(stderr, "%s: could not exec cpropep in %s\n",
			myname, CPRODIR);
		errors = 1;
	    default:
		fprintf(stderr, "%s: cpropep returned abnormal status %d\n",
			myname, WEXITSTATUS(status));
		errors = 1;
	    }

	if (errors) {
		fprintf(stderr, "\n\tcpropep input file is %s\n", CPROTEMP);
		fprintf(stderr, "\tcpropep directory is %s\n", CPRODIR);
		exit(1);
	}
}

static void
cpropep_exec(double Nzr, char *fuel)
{
	int i, j;
	struct cpp_s *p;

	for (i = 0; i < N_OF; i++)
	    for (j = 0; j < N_CP; j++) {
	    	p = &(data[i][j]);

		p->OF = OFvector[i];
		p->Cp = CPvector[j];
		p->Nzr = Nzr;

		cpropep_once(p, fuel);

		p->Cs *= 100. / 2.54 / 12.;

	    }
}

static void
usage()
{
	fprintf(stderr, "Usage: %s <output file> <NZR> <fuel>\n",
		myname);
	exit(1);
}

int
main(int argc, char **argv)
{
	int fd;
	double Nzr;
	char *filename;
	char *fuel;
	struct fuel_data_s f;
	struct liquid_fuel_data_s l;

	myname = *argv;
	fuel_long_name = (char *)0;

	if (argc > 1 && strcmp(argv[1], "-e") == 0) {
		exec_mode = 1;
		argc--;
		argv++;
	}

	if (argc != 4)
		usage();

	filename = *++argv;
	Nzr = atof(*++argv);
	fuel = *++argv;

	switch (fuel_data(fuel, &f)) {
	case 0:
		fuel_cpropep = f.cpropep;
		break;
	case 1:
		usage();
	case 2:
		if (liquid_fuel_data(fuel, &l))
			usage();
		fuel_cpropep = l.cpropep;
		break;
	default:
		fprintf(stderr, "%s: unknown return from fuel_data\n", myname);
		usage();
	}

	if (fuel_data("nitrous", &f)) {
		fprintf(stderr, "%s: cannot get cpropep linkage for nitrous\n",
			myname);
		exit(1);
	}
	nitrous_cpropep = f.cpropep;

	fd = open(filename, O_WRONLY | O_CREAT, 0666);
	if (fd < 0) {
		fprintf(stderr, "%s: cannot open %s for writing\n",
			myname, filename);
		exit(1);
	}

	if (exec_mode)
		cpropep_exec(Nzr, fuel);
	else
		cpropep_system(Nzr, fuel);

	if (write(fd, &data, sizeof data) != sizeof data) {
		fprintf(stderr, "%s: could not write data to %s\n",
			myname, filename);
		perror("write");
		exit(1);
	}
	close(fd);
	return 0;
}
