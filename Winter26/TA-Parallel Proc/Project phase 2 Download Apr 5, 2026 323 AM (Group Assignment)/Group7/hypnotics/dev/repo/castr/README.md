# CASTR - Cellular Automaton Streams

A simple Cyclical Cellular Automaton engine.


## Dependencies

castr depends on ffmpeg for aggregating frames into a video file, but if you do not have ffmpeg, or cannot install it, you can still view the individual frames generated in any image viewer that supports PPM files

## How to run it.

For a good pattern run:

```bash
    make && ./bin/castr data/pattern mov.mp4 300 2
```

The first argument is a data file, you can find more in the data directory or by creating a new one using ./bin/dataset-generator.

The second is the output file of the movie.

The third is the number of steps or loops of the simulation to run, the movie will be rendered at 60 frames per second, so pick a sufficiently large number,

The forth is the threshold, or how many adjacent squares are required to trigger an increment.

