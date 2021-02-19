# PIDMotorControl
Firware developed for arduino due, that reads data from an enconder through 2 external interrupts on pins D20(A) and D21(B), and with the help of a timer interrupt it calculates the speed of the rotor and its position considering the value of the reduction ratio of the gear.
This data is send to a user interface throug a serial port, that also has a interruption to enable the interface to properly control the system, through this interface the user can select a controller type to implement and change the input signal and the controller's gains to see how it will behave through a series chart of the input, the system's response and the error, and through a bar chart the user can see the voltage that is beeing applyed to the system. Besides that the user can use the interface to just turn on/off the motor, change its direction and the voltage input.

This project needs to be used with the https://github.com/igorberaldo/PIDMotorControlGUI
