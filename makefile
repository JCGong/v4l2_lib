.PHONY:clean

obj=v4l2_operation_lib.o main.o
exe=v4l2_operation_test

#CC=arm-linux-gcc

CC=arm-linux-gnueabihf-gcc
LFLAGS=-c
LDFLAGS=

$(exe):$(obj)
	$(CC) $(LDFLAGS) $^ -o $@
	
%.o:%.c %.h
	$(CC) $(LFLAGS) $^
	
clean:
	-rm -v *.o $(exe)

