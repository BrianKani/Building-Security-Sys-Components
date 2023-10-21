# Summary
Each door controller is responsible for a single secured door. The doors are operated remotely and may receive TCP messages requesting that they open or close - typically from the overseer, but in an emergency, from the fire alarm unit. Doors have four states: open, closed, and in the process of opening or closing.

Doors can come in two security configurations: fail-safe and fail-secure. Fail-safe doors will be instructed to open by the fire alarm unit during an emergency situation. During a security breach, fail-secure doors will be instructed to permanently close (until the building's systems are reset-- this event is outside the scope of this assessment.)

 

# Safety-critical component

As the doors may need to be opened in an emergency by the fire alarm unit, the door controller is considered a safety-critical component and therefore must be implemented following appropriate safety-critical software standards. Make sure to include a block comment near the top of your program's source code documenting and justifying any deviations or exceptions.
