CC = gcc
CFLAGS = -O3 -g

all: dataset_generator castr
	rm -f frames/*.ppm

dataset_generator: src/dataset-generator/main.c
	$(CC) $(CFLAGS) -o bin/dataset-generator src/dataset-generator/main.c

clean:
	rm -f bin/*
	rm -f frames/*.ppm

CASTR_OBJ := bin/automatons_main.o bin/automatons_cycle.o bin/ffmpeg_write.o
castr: $(CASTR_OBJ)
	$(CC) $(CFLAGS) -o bin/castr $(CASTR_OBJ)

bin/automatons_main.o : src/automatons/main.c
	$(CC) $(CFLAGS) -o bin/automatons_main.o -c src/automatons/main.c

bin/automatons_cycle.o: src/automatons/cycle.c
	$(CC) $(CFLAGS) -o bin/automatons_cycle.o -c src/automatons/cycle.c

bin/ffmpeg_write.o: src/image-to-video/converter.c
	$(CC) $(CFLAGS) -o bin/ffmpeg_write.o -c src/image-to-video/converter.c
