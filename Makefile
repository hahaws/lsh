objects = lsh.o lsh_dict.o lsh_config.o

lsh : $(objects)
	gcc -o lsh $(objects)

lsh.o : lsh.h
lsh_dict.o : lsh_dict.h
lsh_config.o : lsh_config.h

.PHONY : clean

clean:
	-rm $(objects)
