#
# This file is a portion of Hsim 0.2
#
# Hsim is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published
# by the Free Software Foundation; either version 2 of the License,
# or (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

CFLAGS=-Wall -I../lib
TESTS=n2o_test tank_test fuel_test injector_test chamber_test chem_test
PROGRAMS: hsim report createNzr fuel.csv n2orifice water ${TESTS}

#
# Programs
#
sim_main.o: linkage.h fuel.h state.h ../lib/scio.h ../lib/rsim.h ../lib/ts_parse.h

hsim: sim_main.o state.o libhybrid.a ../lib/librsim.a
	gcc ${CFLAGS} -o hsim sim_main.o state.o libhybrid.a ../lib/librsim.a

report: report.o state.o ../lib/librsim.a libhybrid.a
	gcc ${CFLAGS} -o report report.o state.o libhybrid.a ../lib/librsim.a

createNzr: createNzr.c cpp.h libhybrid.a ../lib/librsim.a
	gcc -Wall -o createNzr createNzr.c libhybrid.a ../lib/librsim.a

n2orifice.o: linkage.h state.h ../lib/scio.h ../lib/rsim.h ../lib/ts_parse.h

n2orifice: n2orifice.o libhybrid.a ../lib/librsim.a state.o
	gcc ${CFLAGS} -o n2orifice n2orifice.o state.o libhybrid.a ../lib/librsim.a

fuel_gen: fuel_gen.c
	gcc -Wall -o fuel_gen fuel_gen.c

expand: expand.o linkage.h 
	gcc ${CFLAGS} -o expand expand.o state.o libhybrid.a ../lib/librsim.a

water: water.o linkage.h
	gcc ${CFLAGS} -o water water.o state.o libhybrid.a ../lib/librsim.a

#
# Simulator routines
#

OBJS=chamber.o chem.o fuel.o tank.o sim.o injector.o constants.o \
	record_data.o n2o_thermo.o vent.o errors.o rocksim.o \
	license.o fuel_data.o liquid.o liquid_data.o \
	liquid_injector.o

libhybrid.a: ${OBJS}
	-rm libhybrid.a
	ar rc libhybrid.a ${OBJS}

chem.o: chem.c state.h linkage.h cpp.h
chamber.o: chamber.c state.h linkage.h
fuel.o: fuel.c state.h linkage.h fuel.h
injector.o: injector.c state.h linkage.h
tank.o: tank.c state.h linkage.h
sim.o: sim.c state.h linkage.h
constants.o: constants.c state.h linkage.h
record_data.o: linkage.h state.h
n2o_thermo.o: linkage.h
vent.o: vent.c linkage.h state.h
state.o: state.c state.h
errors.o: errors.c state.h linkage.h
rocksim.o: rocksim.c rocksim.h
license.o: license.c
fuel_data.o: fuel_data.c fuel.h
liquid.o: liquid.c liquid_fuel.h state.h
liquid_data.o: liquid_data.c liquid_fuel.h
liquid_injector.o: liquid_injector.c state.h

#
# Test programs
#

n2o_test.o: linkage.h state.h ../lib/scio.h ../lib/rsim.h ../lib/ts_parse.h
tank_test.o: linkage.h state.h ../lib/scio.h ../lib/rsim.h ../lib/ts_parse.h
fuel_test.o: linkage.h state.h ../lib/scio.h ../lib/rsim.h ../lib/ts_parse.h
injector_test.o: linkage.h state.h ../lib/scio.h ../lib/rsim.h ../lib/ts_parse.h
chamber_test.o: linkage.h state.h ../lib/scio.h ../lib/rsim.h ../lib/ts_parse.h
chem_test.o: linkage.h state.h ../lib/scio.h ../lib/rsim.h ../lib/ts_parse.h

n2o_test: n2o_test.o state.o libhybrid.a ../lib/librsim.a
	gcc ${CFLAGS} -o n2o_test n2o_test.o state.o libhybrid.a ../lib/librsim.a

tank_test: tank_test.o state.o libhybrid.a ../lib/librsim.a
	gcc ${CFLAGS} -o tank_test tank_test.o state.o libhybrid.a ../lib/librsim.a

fuel_test: fuel_test.o state.o libhybrid.a ../lib/librsim.a
	gcc ${CFLAGS} -o fuel_test fuel_test.o state.o libhybrid.a ../lib/librsim.a

injector_test: injector_test.o state.o libhybrid.a ../lib/librsim.a
	gcc ${CFLAGS} -o injector_test injector_test.o state.o libhybrid.a ../lib/librsim.a

chamber_test: chamber_test.o state.o libhybrid.a ../lib/librsim.a
	gcc ${CFLAGS} -o chamber_test chamber_test.o state.o libhybrid.a ../lib/librsim.a

chem_test: chem_test.o state.o libhybrid.a ../lib/librsim.a
	gcc ${CFLAGS} -o chem_test chem_test.o state.o libhybrid.a ../lib/librsim.a

# CSV file
fuel.csv: fuel_gen
	./fuel_gen > fuel.csv
