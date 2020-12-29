
# ESPL_Tetris
=======
# Exercise 2 and 3 Solution by Andrew Elsayeh

 **Usage:** Use 'E' to switch between screens. For exercise 3, the number of times the buttons 'X' and 'Y' are counted, and 'Z' toggles the timer.

 **Git Username:** andrew-elsayeh

 Please note that the repository for exercises 2-4 is private. I have invited alxhoff and PhilippvK as collaborators. 
 
---

## Answer to therertical questions.

> 2.3.1 Make yourself familiar with the mouse interface. How does the mouse guarantee thread-safe
functionality?

The TUM Event Library uses a Mutex Semaphore to gaurd the the contents of the variables storing the mouse coordinates. 

>3.1 What is the kernel tick? Experiment with its frequency.

The kernel tick is the frequency at which the kernel checks which processes are running and which should be running. Using a slower kernel tick might cause tasks to not be scheduled properly since a task with a higher priority might be "Ready" but the kernel will only let it run when it checks at a tick

>3.1 What is a tickless kernel?

A tickless kernel doesn't have ticks running at a regular frequency ( for example every 100ns) but rather only as required. This can be useful in order to save power since the kernel doesn't have to keep the CPU running to check which tasks need to be scheduled. 

>3.2.2.5 Experiment with the task stack size. What happens if it is too low?

The stack can overflow, this can cause unexepcted behaviour and instability of the program.

>4.3 Play around with the priorities from exercise in Section 3.3.2 (e.g. set all priorities to the same value, inverse the priority order, etc.). What can you observe?

The Tasks are called in the order of their priorities except for tasks 3 which is always called 
