all:
	xelatex tad-thesis.tex
	-bibtex  tad-thesis.aux
	xelatex tad-thesis.tex
	xelatex tad-thesis.tex
clean:
	find . -name '*.aux' -print0 | xargs -0 rm -rf
	rm -rf *.lof *.log *.lot *.out *.toc *.bbl *.blg *.thm
depclean: clean
	rm -rf *.pdf
