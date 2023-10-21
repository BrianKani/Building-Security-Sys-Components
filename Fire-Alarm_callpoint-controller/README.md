# Summary
Each fire alarm call-point controller is responsible for a single wall-mounted manual call-point, which is a little button on the wall that users can press during an emergency to activate the alarm system.

When the button is pressed, the controller will send a UDP datagram to the fire alarm unit telling it to signal the alarm. It will then continue sending these datagrams in a loop perpetually to ensure that they are received.

 

# Safety-critical component
As the call points are used to manually signal a fire emergency, the call-point controller is considered a safety-critical component and therefore must be implemented following appropriate safety-critical software standards. Make sure to include a block comment near the top of your program's source code documenting and justifying any deviations or exceptions.
