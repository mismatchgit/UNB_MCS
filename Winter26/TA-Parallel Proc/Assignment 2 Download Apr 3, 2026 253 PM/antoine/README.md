# Compile Commands

If you're running mode 0:
```
make
./serial <func_id> <mode> <tol>
```

If you're running mode 1:
```
make
mpirun -np <P> ./static_mpi <func_id> <mode> <tol>
```

If you're running mode 2:

```
make
mpirun -np <P> ./dynamic_mpi <func_id> <mode> <tol>
```

