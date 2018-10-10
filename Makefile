CFLAGS = -std=gnu99 
LIBS = -lpthread -lrt -lm
SOURCES = project2.c my_memory.c 
OUT = out
default:
	gcc $(CFLAGS) $(SOURCES) $(LIBS) -o $(OUT)
debug:
	gcc -g $(CFLAGS) $(SOURCES) $(LIBS) -o $(OUT)
all:
	gcc $(SOURCES) $(LIBS) -o $(OUT)
clean:
	rm $(OUT)
