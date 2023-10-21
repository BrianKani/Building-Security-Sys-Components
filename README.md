# Building Security Systems Components
This GitHub repository houses a collection of software components designed for managing and securing building access, fire alarms, and security systems. These components work in tandem, communicating via a combination of TCP and UDP over IPv4 to achieve comprehensive building security. This project is highly versatile and can be utilized in various building management scenarios.

# Components Included

Overseer: The overseer acts as the central control unit, communicating with all other components and governing the entire building's security operations. There is exactly one overseer in the system.

Card Reader Controller: Card reader controllers communicate with the overseer and report successful card scans. A system can have 0 or more card reader controllers.

Door Controller: Door controllers manage the state of security doors, controlling whether they are open or shut. A system can have 0 or more door controllers.

Fire Alarm Unit: The fire alarm unit connects with the overseer, security doors, and a temperature sensor mesh network. It activates fire alarms and opens specific doors in response to manual call-point activation or elevated temperature readings. There is exactly one fire alarm unit in the system.

Temperature Sensor Controller: Temperature sensor controllers collect local temperature readings from the building and establish a mesh network with other sensors, the overseer, and the fire alarm system. A system can have 0 or more temperature sensor controllers.

Fire Alarm Call-Point Controller: These controllers allow manual activation of the fire alarm. A system can have 0 or more fire alarm call-point controllers.

Elevator Controller: Elevator controllers manage elevator doors and motion, transporting authorized users between floors based on their access permissions. A system can have 0 or more elevator controllers.

Destination Select Controller: These controllers enable users to select their desired floor after scanning their access card. They connect to the overseer, which then instructs the elevator controller for the selected route. A system can have 0 or more destination select controllers.

Security Camera Controller: Security camera controllers read pixel data from cameras and can control camera direction. If motion is detected, they send a report to the overseer, which may trigger a security alarm and close secure doors. A system can have 0 or more camera controllers.

Simulator: The simulator reads scenario files, spawns controllers and the fire alarm unit, and simulates events such as people entering the building, card scans, camera events, and fire simulations. There is a single simulator in the system.

Safety-Critical Systems
Some components (door controllers, fire alarm unit, and fire alarm call-point controllers) are considered safety-critical and must be developed to appropriate standards, with all deviations and exceptions well-documented.

Getting Started
Please refer to the documentation and instructions in each component's subdirectory for specific details on usage, deployment, and standards compliance for safety-critical components.

# License
This project is open-source and available under the MIT License. Feel free to use, modify, and contribute to the repository to enhance building security systems.

We encourage collaboration and welcome contributions from the community to improve the reliability and security of building access, fire alarm, and security systems.
