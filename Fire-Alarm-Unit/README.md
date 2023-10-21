# Fire alarm unit

## Summary
The fire alarm unit is a separate system to the overseer, and its job is to detect a fire emergency (either triggered by manual call-points or detected from temperature sensors) and to signal the building's fire alarms as well as to open the security doors marked fail-safe in the event of one. The system is kept separate from the overseer to ensure a higher level of redundancy.

The fire alarm unit will register itself with the overseer via TCP in the same way that many of the components do. The overseer will then send UDP datagrams to the fire alarm unit giving the IPv4 addresses and ports of fail-safe doors, which will need to be opened during an emergency. The fire alarm unit will respond with a UDP datagram telling the overseer that the message was received (otherwise it will be sent again).

 

## Safety-critical component
Due to the fire alarm unit's vital role in handling fire emergencies, this is considered a safety-critical component and therefore must be implemented following appropriate safety-critical software standards. Make sure to include a block comment near the top of your program's source code documenting and justifying any deviations or exceptions.
