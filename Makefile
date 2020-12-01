.PHONY: clean debug

M=main
N=microbash
B=bin
S=src
FLAG=-Wall -O0 -pedantic -Werror

all: $(B)/$(N)
debug: $(B)/$(N)_debug

$(B)/$(N): $(S)/$(M).c $(S)/$(N).c
	gcc -o $(B)/$(N) $(S)/$(N).c $(S)/$(M).c -O0

$(B)/$(N)_debug: $(S)/$(M).c $(S)/$(N).c
	gcc -o $(B)/$(N)_debug $(S)/$(N).c $(S)/$(M).c $(FLAG)

clean:
	/bin/rm -f $(B)/$(N) $(B)/$(N)_debug

tgz: clean
	cd .. ; tar cvzf $(N).tgz $(N)
