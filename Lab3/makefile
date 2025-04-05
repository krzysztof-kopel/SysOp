all: clean_text flipper

test: clean flipper

.PHONY: main clean 

flipper: flipper.c
	gcc flipper.c -o flipper

clean_text:
	rm -fr *.txt outputs

clean: clean_text
	rm -f flipper


