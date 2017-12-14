.PHONY:clean

#obj=displayjpeg.o
obj=v4l2_operation_lib.o main.o pic.o
exe=main

CC=arm-linux-gcc

#CC=arm-linux-gnueabihf-gcc
CFLAGS=-I./jpeglib/include	
LDFLAGS= -L./jpeglib/lib -ljpeg 

$(exe):$(obj)
	@$(CC) -static $^ -o $@ $(LDFLAGS)

%.o:%.c %.h
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@-rm -v *.o $(exe)
