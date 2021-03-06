# This file is part of Shapes.
#
# Shapes is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# any later version.
#
# Shapes is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Shapes.  If not, see <http://www.gnu.org/licenses/>.
#
# Copyright 2008 Gustaf Hendeby, Henrik Tidefelt

# === How to setup a new test ===

# It is actually very simple:

# Just add your .shape file and a corresponding .correct file.  The
# .correct file should be the output expected on stdout, like this:
#   %.correct : %.out
#   mv $< $@
# If the test is expected to fail, simply add a .correct file such
# that a failing test will not be able to create it, for instance by
# copying the .correct file from another test.  See below for how to
# indicate that a test is expected to fail.

# The following variable must be edited by hand to include every test
# that is expected to fail.
XFAIL_TESTS = test_bad

SHAPES = ${top_builddir}/source/shapes
SHAPESFLAGS = --resources=no \
              --fontmetricspath ${top_srcdir}/resources/fontmetrics \
              --tmpdir "/tmp/shapes.${USER}" --tmp*=yes \
              --needpath ${top_srcdir}/resources/extensions

TEST_BASE = $(notdir $(basename $(wildcard ${srcdir}/*.shape)))
TESTS = ${addprefix test_, ${TEST_BASE}}
check_DATA = ${addsuffix .out, ${TEST_BASE}}
dist_check_DATA = testTEMPLATE \
                  ${addsuffix .shape, ${TEST_BASE}} \
                  ${addsuffix .correct, ${TEST_BASE}}
CLEANFILES = ${addprefix test_, ${TEST_BASE}} \
             ${addsuffix .out, ${TEST_BASE}} \
             ${addsuffix .pdf, ${TEST_BASE}} \
             ${addsuffix .log, ${TEST_BASE}}

test_% : testTEMPLATE
	@sed -e 's/FNAME/$*/g' < $< > $@ && \
  chmod a+rx $@

%.out : ${srcdir}/%.shape ${SHAPES}
	-@$(SHAPES) ${SHAPESFLAGS} $< > $@ 2> /dev/null
