The overseer and other components are designed so that they can be plugged into a system as long as it provides the shared memory interface to the building's mechanisms.

For testing purposes, however, since there is no actual building to test on, we need to provide a simulator. The simulator will pretend to be a collection of card readers, doors, elevators, cameras, temperature sensors etc. and communicate with your components via that shared memory.

# RUN
To compile the simulator sourefile run the following command:
		gcc -o simulator simulator.c -lpthread
Which allows the compiler to link with the pthread library


