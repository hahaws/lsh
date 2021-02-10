objects = lsh.o lsh_dict.o lsh_config.o lsh_parser.o

lsh : $(objects)
	gcc -o lsh $(objects)

lsh.o : lsh.h
lsh_dict.o : lsh_dict.h
lsh_config.o : lsh_config.h
lsh_parser.o : lsh_parser.h

.PHONY : clean

clean:
	-rm -f $(objects)
