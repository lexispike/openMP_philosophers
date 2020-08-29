# compile Q3.cpp
noodles: Q3.cpp
	g++ -std=c++11 -fopenmp Q3.cpp -o noodles

# run noodles with 5 philosophers
run_noodles: noodles
	./noodles 5

# clean the directory
clean:
	rm -f noodles
