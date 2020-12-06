CC = clang
LFLAGS =  -L/usr/local/lib -lhdf5
CFLAGS += -I src -I /usr/local/include -Wall
LIBS =  -L/usr/local/lib -lhdf5

all: libgrandlib.a to_hdf5

libgrandlib.a: grand_hdf5lib.o grand_misc.o grand_binlib.o
	ar -r $@ $^

to_hdf5: to_hdf5.o libgrandlib.a
	$(CC) -o $@ $(CFLAGS) $(LFLAGS) -L. -lgrandlib $(LIBS) $<

%.o: %.c %.h Makefile 
	${CC} $(CFLAGS) -c $<
