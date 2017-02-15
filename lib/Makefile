#
# This file is a portion of Hsim 0.1
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
CFLAGS=-Wall

librsim.a:	ts_parse.o scio.o csv.o interpolate.o dscopy.o cfgets.o
	-rm librsim.a
	ar rc librsim.a ts_parse.o scio.o csv.o interpolate.o dscopy.o cfgets.o

scio_test: scio_test.c librsim.a
	gcc -Wall -o scio_test scio_test.c librsim.a

csv_test: csv_test.c rsim.h librsim.a
	gcc -Wall -o csv_test csv_test.c librsim.a

cfgets_test: cfgets_test.c rsim.h librsim.a
	gcc -Wall -o cfgets_test cfgets_test.c librsim.a
