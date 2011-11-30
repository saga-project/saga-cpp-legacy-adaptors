
#####################################################################
# project name

BASE        = saga-irods-spec

#####################################################################
# executeables

TEX         = latex
COL         = | cat | awk -- '$(AWK_COLOR)'
PDFTEX      = pdflatex
SPELL       = ispell
PSBOOK      = psbook
DVIPS       = dvips
BIBTEX      = bibtex
MAKEINDEX   = makeindex
MAKE_IND    = $(MAKEINDEX) -s gind.ist
PSNUP_PAT   = '4:0L@.7(21cm,0)+1L@.7(21cm,14.85cm),2R@.7(0,14.85cm)+3R@.7(0,29.70cm)'
PSNUP       = pstops $(PSNUP_PAT)

AWK_COLOR   = # /^\(/       { next }
AWK_COLOR  +=   /^ *$$/     { next }
AWK_COLOR  += # /^:/        { next }
AWK_COLOR  += # /^\)/       { next }
AWK_COLOR  += # /^Chapter/  { next }
AWK_COLOR  += # /^\\OT/     { next }
AWK_COLOR  += # /^ *\[/     { next }
AWK_COLOR  += # /^ *</      { next }
AWK_COLOR  += # /^\.$/      { next }
AWK_COLOR  += # /^Append/   { next }
AWK_COLOR  += #
AWK_COLOR  += /^LaTeX Font Warning:/   { next }
AWK_COLOR  += #
AWK_COLOR  += /Warning/   { print "\033[1;34m" $$0 "\033[0m" ; next}
AWK_COLOR  += /Error/     { print "\033[1;31m" $$0 "\033[0m" ; next}
AWK_COLOR  += /Overfull/  { print "\033[1;31m" $$0 "\033[0m" ; next}
AWK_COLOR  += /Underfull/ { print "\033[1;35m" $$0 "\033[0m" ; next}
AWK_COLOR  += #
AWK_COLOR  += { print }

AWK_FILTER  = /^- ?(.*)/ { next                           }
AWK_FILTER += /^+ ?(.*)/ { print " " substr($$0, 2); next }
AWK_FILTER += /^! ?(.*)/ { print " " substr($$0, 2); next }
AWK_FILTER +=            { print                   ; next }

GVIM        = /Applications/System/MacVim.app/Contents/MacOS/Vim -g

FIG2DEV     = fig2dev
PS2EPSI     = ps2epsi
EPS2PDF     = epstopdf
TXT2TEX     = ./pp -DTEXMODE
XDVI_EDITOR = "xdvi_edit XDVI-$(BASE) %f %l"
XDVI_OPTS   = 
XDVI        = xdvi -editor $(XDVI_EDITOR) $(XDVI_OPTS)
XDVI_LOCK   = .xdvi_lock
ACRO        = xpdf -z page
ACRO_LOCK   = .acro_lock
RM          = rm -f
TOUCH       = touch
LPR         = lpr
ENV         = /bin/env DVIFILE=$(BASE).dvi DVINAME=XDVI-$(BASE)
EDITOR      = $(GVIM) -o --servername XDVI-$(BASE)

#####################################################################
# files

TEXDIR      = .
BIBDIR      = .
PICDIR      = ./pics/
FIGDIR      = ./pics/

TEXFILES    = $(wildcard $(TEXDIR)/*.tex)
BIBFILES    = $(wildcard $(BIBDIR)/*.bib)
AUXS        = $(wildcard *.aux)
PICS        = $(wildcard $(PICDIR)/*.{eps,ps,jpeg,jpg,gif,png,pdf})
FIGS        = $(wildcard $(PICDIR)/*.fig)

TEX_FIGS    = $(addsuffix .eps,$(basename $(FIGS)))
TEX_FIGS   += $(addsuffix .png,$(basename $(FIGS)))
PDF_FIGS    = $(addsuffix .pdf,$(basename $(FIGS)))

TEX_FINAL   = $(addsuffix .final,$(TEXFILES))

TXTFILES    = $(wildcard $(TEXDIR)/*.txt)
# INCTEX      = $(addsuffix .tex,$(basename $(TXTFILES)))

#####################################################################
# special targets

.SECONDARY:   $(TEX_FIGS) $(PDF_FIGS)

.SUFFIXES:    .tex .dvi .eps .ps  .ps2 .pdf \
              .bib .bbl .fig .idx .ind

.PHONY:       clean distclean figclean \
              show   s                 \
              again  a                 \
              print  p                 \
              print2 p2                \
              spell                    \
              touch  t                 \
              edit   e                 \
              fine                     \
              bib bbl ind              \
              ps ps2 pdf dvi

#####################################################################
# implicit rules

%.tex.final:  %.tex
%.tex.final:  %.tex $(TEXFILES)
	            @echo "cat $< | awk --    AWK_FILTER   > $@"
	            @      cat $< | awk -- '$(AWK_FILTER)' > $@

%.dvi:        %.tex
%.dvi:        %.tex $(TEXFILES) $(INCTEX) $(PICS) $(TEX_FIGS)
	            @echo "$(TEX) $<"
	            @$(TEX) $< $(COL)

%.pdf:        %.tex
%.pdf:        %.tex $(TEXFILES) $(INCTEX) $(PICS) $(PDF_FIGS)
	            $(PDFTEX) $<

%.ps:         %.dvi
%.ps:         %.dvi
	            $(DVIPS) $< -o $@

%.ps2:        %.ps
%.ps2:        %.ps
	            $(PSNUP) $< $@
	
%.bbl:        %.bib
%.bbl:        %.bib
	            $(if $(wildcard $(BASE).aux), $(BIBTEX) $* )

%.ind:        %.idx
%.ind:        %.idx
	            $(MAKE_IND) -o $@ $<

%.eps:        %.fig
%.eps:        %.fig
	            $(FIG2DEV) -Leps $< $@

%.ps:         %.fig
%.ps:         %.fig
	            $(FIG2DEV) -Lps $< $@

%.png:        %.fig
%.png:        %.fig
	            $(FIG2DEV) -Lpng -S 4 $< $@

%.pdf:        %.fig
%.pdf:        %.fig
	            $(FIG2DEV) -Lpdf $< $@

%.eps:        %.ps
%.eps:        %.ps
	            $(PS2EPSI) $< $@

%.pdf:        %.eps
%.pdf:        %.eps
	            $(EPS2PDF) $<

# %.tex:        %.txt
# %.tex:        %.txt
# 	            $(TXT2TEX) < $< > $@

#####################################################################
# short targets

ifdef PRIVATE_MAKE_STUFF
s:            show
d:            slides
f:            figs
b:            bib
a:            again
t:            touch
e:            edit
p:            print
p2:           print2
endif

pdf:          $(BASE).pdf
ps:           $(BASE).ps
ps2:          $(BASE).ps2
dvi:          $(BASE).dvi
tex:          $(BASE).tex
ind:          $(BASE).ind
bbl:          $(BASE).bbl
aux:          $(BASE).aux

#####################################################################
# targets

figs: $(TEX_FIGS) $(PDF_FIGS)

spell:
	            @$(SPELL) $(TEXFILES)

print:        $(BASE).ps
	            @$(LPR) $(BASE).ps

touch:
	            @$(TOUCH) $(TEXFILES) $(BIBFILES)

edit:
	            @$(ENV) $(EDITOR) $(BASE).tex &
	            @echo "name: $(EDITOR)"

final: $(TEX_FINAL)
	@echo "========================================"
	@echo ""
	@echo " Attention:"
	@echo ""
	@echo " you need to enable \sagafinal in "
	@echo " saga_core.tex to get a clean version!"
	@echo ""
	@echo " after 'make final' run 'make again'"
	@echo ""
	@echo "========================================"

#--------------------------------------------------
# if an xdvi starts, it creates an XDVI_LOCK file.
# if this exists, it does not start a new process.
# if xdvi finishes, it removes the XDVI_LOCK file.
#
ifdef PRIVATE_MAKE_STUFF
show:         $(BASE).dvi
	            @if [ ! -r $(XDVI_LOCK) ]; then   \
	              ($(TOUCH) $(XDVI_LOCK) ;        \
	               $(ENV)   $(XDVI) $(BASE).dvi ; \
	               $(RM)    $(XDVI_LOCK) ;        \
	              )&                              \
	            fi
	            -@$(GVIM) --servername XDVI-$(BASE) --remote-send "<ESC>:call XDVI_jump()<CR>i"

slides:       $(BASE).pdf
	            @if [ ! -r $(ACRO_LOCK) ]; then \
	              ($(TOUCH) $(ACRO_LOCK) &&     \
	               $(ACRO)  $(BASE).pdf  &&     \
	               $(RM)    $(ACRO_LOCK) ;      \
	              )&                            \
	            fi
endif


again:
	            $(MAKE) -W $(BASE).tex $(filter-out again, $(MAKECMDGOALS))
	
fine:
	            @$(MAKE) distclean
	            @$(MAKE) dvi again
	            @$(MAKE) ind
	            @$(MAKE) bib
	            @$(MAKE) again dvi
	            @$(MAKE) again dvi
	            -@$(MAKE) again pdf
	            @$(MAKE) again pdf
	            @$(MAKE) ps

clean: # figclean
	            @$(RM) $(INCTEX) $(BASE).m* \
	              *.{aux,blg,bbl,log,toc,err,idx,bak,ilg,ind,lof,lot,out,lod} \
	              *.{bmt,loe,glo} \
                .*.sw[po] *~ \
                *.pdf

distclean:
	            @$(MAKE) clean
	            @$(RM) $(BASE).{bak,dvi,ps,pdf,tex.bak}
	            @$(RM) $(XDVI_LOCK) $(ACRO_LOCK) makelog
	            @$(RM) *.tex.final

figclean:
	            @$(RM) $(TEX_FIGS) $(PDF_FIGS)

bib:
	            -@for file in $(AUXS) ; do $(BIBTEX) $$file ; done

#####################################################################
# dependencies

$(BASE).aux:  $(BASE).dvi
$(BASE).log:  $(BASE).dvi
$(BASE).toc:  $(BASE).dvi
$(BASE).idx:  $(BASE).dvi

test:
	@echo INCTEX: $(INCTEX)

log:
	@cvs log | grep lines | sort -n

