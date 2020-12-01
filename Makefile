.PHONY: clean debug

M=main
MB=microbash
B=bin
S=src
FLAG=-Wall -O0 -pedantic -Werror

all: $(B)/$(MB)
debug: $(B)/$(MB)_debug

$(B)/$(MB): $(S)/$(M).c $(S)/$(MB).c
	gcc -o $(B)/$(MB) $(S)/$(MB).c $(S)/$(M).c -O0

$(B)/$(MB)_debug: $(S)/$(M).c $(S)/$(MB).c
	gcc -o $(B)/$(MB)_debug $(S)/$(MB).c $(S)/$(M).c $(FLAG)

clean:
	/bin/rm -f $(B)/$(MB) $(B)/$(MB)_debug

tgz: clean
	cd .. ; tar cvzf $(MB).tgz $(MB)
