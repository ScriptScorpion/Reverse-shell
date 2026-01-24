CC := g++
CCFLAGS := -pthread

main:
	$(CC) $(CCFLAGS) client.cpp -o cl
	$(CC) $(CCFLAGS) server.cpp -o sv
