# nm

## A network modeling tool

Given a network and traffic demands described in csv format (for examples see network.csv and traffic.csv) show the shortest path that traffic takes. 

Simulate N number of link failures and see how that effects traffic and link load.

See sample output at out.md

### compile
```
clang++ main.cpp -o nm -O3
```

### run
```
./nm -n network.csv -t traffic.csv -s 2
```