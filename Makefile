objects = lsh.o lsh_dict.o

lsh : $(objects)
	gcc -o lsh $(objects)

lsh.o : lsh.h
lsh_dict.o : lsh_dict.h

.PHONY : clean

clean:
	-rm $(objects)
