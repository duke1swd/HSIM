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
 * Print out the warnings.
 */

#include <stdio.h>
#include <stdlib.h>
#include "ts_parse.h"
#include "scio.h"
#include "linkage.h"
#include "state.h"

void
errors_init()
{
	warn_n2o_flux = 0;
	warn_n2o_flux_value = .0;
	warn_core_throat_ratio = 0;
	warn_core_throat_ratio_value = WARN_CORE_THROAT_RATIO_2;
	warn_injector_pressure_drop_value = 1e8;
	warn_supply_pressure = 0;
	warn_negative_vent_to_fill = 0;
}

void
print_errors(FILE *output)
{
	int errors;

	errors = 0;

	if (warn_n2o_flux) {
		errors++;
		fprintf(output, "Error: N2O Flux (%.1f) is higher than "
			"threshold (%.1f)\n",
			scio_convert(warn_n2o_flux_value,
				MASSFLUX, "lb/sec/in/in"),
			scio_convert(WARN_N2O_FLUX_LIMIT,
				MASSFLUX, "lb/sec/in/in"));
	}

	if (warn_core_throat_ratio == 1) {
		errors++;
		fprintf(output, "Error: Core to Throat area ratio (%.1f) "
			"less than threshold (%.1f)\n",
			warn_core_throat_ratio_value,
			WARN_CORE_THROAT_RATIO_1);
	}
	if (warn_core_throat_ratio == 2) {
		errors++;
		fprintf(output, "Warning: Core to Throat area ratio (%.1f) "
			"less than threshold (%.1f)\n",
			warn_core_throat_ratio_value,
			WARN_CORE_THROAT_RATIO_2);
	}

	if (warn_injector_pressure) {
		errors++;
		fprintf(output, "Error: Injector pressure drop (%.1f) "
			"less than %.1f times chamber pressure (%.1f)\n",
			scio_convert(warn_injector_pressure_drop_value,
				PRESSURE, "psi"),
			(double)WARN_INJECTOR_RATIO,
			scio_convert(warn_injector_pressure_drop_chamber_value,
				PRESSURE, "psi"));
	}

	if (warn_supply_pressure) {
		errors++;
		fprintf(output, "WARNING: Supply tank to flight tank "
			"pressure drop (%.0f psi) is less than %.0f psi\n",
			scio_convert(warn_supply_pressure_drop_value,
				PRESSURE, "psi"),
			scio_convert(WARN_SUPPLY_PRESSURE_DROP,
				PRESSURE, "psi"));
	}

	if (warn_negative_vent_to_fill) {
		fprintf(output, "WARNING: N2O Mass Vented to Fill (%.2f kg) "
			"is negative.\n",
			scio_convert(vent_mass, MASS, "kg"));
	}
}

void
error_exit(int code)
{
	fprintf(stderr, "\n\n");
	exit(code);
}
