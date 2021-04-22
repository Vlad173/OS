all: control counting

control:
	g++ control.cpp -lzmq -o control  --std=c++17 -L /usr/local/include 

counting:
	g++ counting.cpp -lzmq -o counting --std=c++17 -L /usr/local/include

clean:
	rm -rf control counting