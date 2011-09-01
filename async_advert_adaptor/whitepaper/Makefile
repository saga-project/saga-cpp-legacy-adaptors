MF=	Makefile

LC=	pdflatex

LFLAGS=

PDF=	saga-advert.pdf

SRC= \
	saga-advert.tex
	

.SUFFIXES:
.SUFFIXES: .tex .pdf

TOC=	saga-advert.toc

IMG=	

TMP=	$(SRC:.tex=.tex~) core saga-advert.aux saga-advert.log

all:    $(PDF)

$(PDF):	$(TOC)
	$(LC) $(LFLAGS) $(SRC) $@ 

$(TOC):	$(MF) $(IMG)

clean:
	rm -f $(PDF) $(TOC) $(TMP)

thesis.toc  : $(SRC)
	$(LC) $(LFLAGS) $(SRC)
