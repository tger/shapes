SAXON = @SAXON@
SHAPES = $(top_builddir)/source/shapes
SHAPESFLAGS = --resources=no \
              --fontmetricspath ${top_srcdir}/resources/fontmetrics \
              --needpath ${top_srcdir}/resources/extensions \
              --tp=no \
              --tmpdir /tmp


include ${top_builddir}/source/SHAPES-VERSION-FILE

DATE_PATTERN='[0-9]\{4\}-[0-9]\{2\}-[0-9]\{2\}'
MORE_SSIFLAGS=-dMODIFICATION_DATE=$(shell echo $(VERSION_DATE) | \
	             sed -e 's!.*\('${DATE_PATTERN}'\).*!\1!' | \
               grep -e ${DATE_PATTERN} 2> /dev/null || echo --- )

# Setting BUILDDIR=^ means that paths will be relative to the current working directory.
SSIFLAGS=-dEXAMPLES=example/ -dBASE= -dBUILDDIR=^/ $(MORE_SSIFLAGS)
SSIFLAGS_EXTENSIONS=-dEXAMPLES=../example/ -dBASE=../ -dBUILDDIR=^/ $(MORE_SSIFLAGS)
SSI=${top_builddir}/doc/tools/ssi/ssipp

html_DATA = man.html tutorial.html syntax.html algo-tol.html bindings.html dynamic.html states.html state-types.html types.html index.html \
extensions/index.html extensions/blockdraw_wfo.html  extensions/centering.html  extensions/circle.html

htmldir = ${pkgdatadir}/doc/html

docdir=${top_srcdir}/doc/parts

dist_noinst_DATA = ${docdir}/man/index.sxml ${docdir}/formats/html.xsl \
                   ${docdir}/formats/man.xsl ${docdir}/man/formats/man.xsl \
                   ${docdir}/algo-tol/tolerances.sxml

extensions/%.xml : ${docdir}/extensions/%.sxml ${DEPDIR}/extensions/%.ssidep
	install -d extensions
	if ${SSI} --in $< ${SSIFLAGS_EXTENSIONS} > extensions/$*.Txml; \
	then \
		mv extensions/$*.Txml $@; \
	else \
		${RM} extensions/$*.Txml; \
		exit 1; \
	fi

%.xml : ${docdir}/%/index.sxml ${DEPDIR}/%.ssidep ${docdir}/formats/html.xsl
	if ${SSI} --in $< ${SSIFLAGS} > $*.Txml; \
	then \
		mv $*.Txml $@; \
	else \
		${RM} $*.Txml; \
		exit 1; \
	fi

${DEPDIR}/extensions/%.ssidep : ${docdir}/extensions/%.sxml
	install -d ${@D}
	if $(SSI) --in $< $(SSIFLAGS_EXTENSIONS) --head $*.xml --deps > ${DEPDIR}/extensions/$*.Tssidep; \
	then \
		mv ${DEPDIR}/extensions/$*.Tssidep $@; \
	else \
		${RM} ${DEPDIR}/extensions/$*.Tssidep; \
		exit 1; \
	fi

${DEPDIR}/%.ssidep : ${docdir}/%/index.sxml
	install -d ${@D}
	if $(SSI) --in $< $(SSIFLAGS) --head $*.xml --deps > ${DEPDIR}/$*.Tssidep; \
	then \
		mv ${DEPDIR}/$*.Tssidep $@; \
	else \
		${RM} ${DEPDIR}/$*.Tssidep; \
		exit 1; \
	fi

example/%.shape : ${top_srcdir}/examples/%.shape
	install -d $(@D)
	install -m 444 $< $@

%.css : ${top_srcdir}/doc/styles/html/%.css
	install -m 444 $< $@

%.pdf %.stdout: %.shape
	$(SHAPES) $(SHAPESFLAGS) --in $< --out $*.pdf > $*.stdout

%_3.pdf %.stdout: %.shape
	$(SHAPES) $(SHAPESFLAGS) -vs1.3 --in $< --out $*_3.pdf > $*.stdout

%_y_big.jpg: %.pdf
	convert -density 150 -resize x150 $< $@

%_y_small.jpg: %.pdf
	convert -density 50 -resize x50 $< $@

%_x_big.jpg: %.pdf
	convert -density 150 -resize 150x $< $@

%_x_small.jpg: %.pdf
	convert -density 50 -resize 50x $< $@

extensions/%.html : extensions/%.xml ${docdir}/extensions/formats/html.xsl ${docdir}/formats/html.xsl
	$(SAXON) $< ${docdir}/extensions/formats/html.xsl | \
	  ${top_builddir}/doc/tools/postproc/postproc-html > $@
	if $(SAXON) $@ ${top_srcdir}/doc/tools/xhtml-deps/filter.xsl > ${DEPDIR}/extensions/$*.Thtmldep ; \
	then \
		echo "html-BUILD html-INSTALL : $$( sed  -e 's!\([^ ]\{1,\}\)!extensions/\1!g' < ${DEPDIR}/extensions/$*.Thtmldep | tr ' ' '\n' | sort -u | tr '\n' ' ' )" > ${DEPDIR}/extensions/$*.htmldep ; \
		${RM} ${DEPDIR}/extensions/$*.Thtmldep ; \
	else \
		${RM} ${DEPDIR}/extensions/$*.Thtmldep ; \
		exit 1; \
	fi

%.html : %.xml ${docdir}/%/formats/html.xsl ${docdir}/formats/html.xsl
	$(SAXON) $< ${docdir}/$*/formats/html.xsl | \
	  ${top_builddir}/doc/tools/postproc/postproc-html > $*.html
	if $(SAXON) $@ ${top_srcdir}/doc/tools/xhtml-deps/filter.xsl > ${DEPDIR}/$*.Thtmldep ; \
	then \
		echo "html-BUILD html-INSTALL : $$( tr ' ' '\n' < ${DEPDIR}/$*.Thtmldep | sort -u | tr '\n' ' ' )" > ${DEPDIR}/$*.htmldep ; \
		${RM} ${DEPDIR}/$*.Thtmldep ; \
	else \
		${RM} ${DEPDIR}/$*.Thtmldep ; \
		exit 1; \
	fi

%.htmldep : %.ssidep
	@touch $@

.PRECIOUS : $(addprefix ${DEPDIR}/, $(html_DATA:.html=.ssidep)) $(html_DATA:.html=.xml)
.PHONY : html-tmp

clean :
	-rm -rf *.xml *.html .deps extensions example \#*

include $(addprefix ${DEPDIR}/, $(html_DATA:.html=.ssidep))
include $(addprefix ${DEPDIR}/, $(html_DATA:.html=.htmldep))

all-am : html-am

html-am : ${html_DATA} shapes.css
	${MAKE} html-BUILD

install-am : html-am
	${MAKE} html-INSTALL

html-INSTALL : ${html_DATA} shapes.css
	for f in $^; \
	do \
		dir="${htmldir}/$$( dirname $$f )" ; \
		test -d  $$dir || install -d -m755 $$dir ; \
		install -m444 $$f $$dir ; \
	done