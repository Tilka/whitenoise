# whitenoise documentation makefile
all:	manual.pdf manual.html post-build-cleanup

manual.pdf: manual.tex
	pdflatex manual.tex
	pdflatex manual.tex

manual.html: manual.tex
	hevea manual.tex

post-build-cleanup: manual.pdf manual.html
	rm -f *.aux *.log

clean:
	rm -f *.aux *.log *.pdf *.html

# arch-tag: DO_NOT_CHANGE_e9e0f24a-d119-4648-bcb5-79994ce891dd 
