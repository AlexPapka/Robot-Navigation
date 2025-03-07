# Robot-Navigation

EEL-4742L Advanced Microprocessor Based System Design Lab Report





Section No:
0001


Lab Instructor:
Mr. Ayodeji Ogundana


Lab No:
10


Lab Title:
Robot Navigation


Name:
Alex Papka


Partner’s Name:
Zachary Fogleman


Date Performed:
11/17/22


Date Delivered:
12/1/2022





Contents
EEL-4742LAdvanced Microcontroller Based Systems Design Lab Report	1
1.	Introduction	3
2.	Design Requirements	3
3.	Theoretical Design	3
4.	Synthesized Design	4
5.	Experimental Results	4
6.	Summary	4
7.	Lessons Learned	5


Introduction
In this lab, students were tasked with using what the learned in the previous labs, to navigate their Roomba through a figure 8 track, within the time limit of 70 seconds.  

Design Requirements
Step 1, turn on design
All LED toggle once
Device enters standby mode.
In standby mode, the LED1 flashes, and the LED2 display’s color based on the number of sensors active. The table below shows LED colors based on the number of sensors active.
If BMP0 is pressed, the device goes to tracking mode.
In tracking mode, LED1 is solid, and LED2 display’s color based on the number of sensors active. The table below shows LED colors based on the number of sensors active.
The wheels move based on the number of sensors active, in the same pattern as the LED2.
If in Tracking mode, the device loses the Line, the design will go to Lost mode, and LED1 will turn off, and LED2 will turn white. If the line is re-found, the device will return to Tracking mode
If any BMP is pressed while in tracking mode or lost mode, the device will return to  standby mode.
The wheels move in reverse in Tracking mode, as to attempt to find the track again, after 10 seconds the design retuurns to tracking mode.
   
   
Theoretical Design
This section should summarize the theory of operation behind the design and present the design itself.  This section should include the following parts:
Top-level design: 
 Read sensor function
Switch state
If standby{
If LED not white{
set LED2
}
If LED white{
LED1 off, Set LED2 to white
}
}
If tracking{
Set LED1 on, set LED2
}
If lost{
Set LED1 off, set LED2 white
}
Robot control function()
delay


Functional description of modules: 
 Read sensor function{
Read in sensor values for P7.0-P7.7
Count how many left sensors are 1
Count how many right sensors are 1
Run sensor resolve function(Left val, right val)
}
Sensor resolve{
If state is lost, it is now tracking
If Left == right == 0
Color is white
If Left == right != 0
Color is green
If Left > Right+1 {
If Right == 0 color is red
Else color is yellow
}
If Right > Left+1{
If Left == 0 color is Blue
Else color is Cyan
}
}
Robot control function{
Switch (mode)
	Case power up,  toggle robit LEDS
	Case stand by, set wheels to coast, turn off robot LEDS
	Case tracking, check LED2 enum
	If red, Lreverse, Rforward
	If yellow, Lcoast, Rforward
	If green, Lforward, Rforward
	If cyan, Lforward, Rcoast
	If blue, Lforward, Rreverse
	For all colors, toggle forward robot LEDS
	Case Lost, reverse both wheels, turn on back robot LEDs
}
 


Lab report questions.  
Did we change our design from week 8 to week 10?
We did change our design slightly, we needed the design to be slightly dumber, so we made it so that left and right turns, to slightly turn, needed 2 more sensors on a side, rather than just 1. This made the design less sensitive to the crossing in the middle of the figure 8. 


Experimental Results
Certification sheet is attached to this file on canvas.  
 

Summary
7 passed
0 failed
100% passed
42.99s on track   
Lessons Learned

We learned sometimes, a complecated solution isn’t always the best
The sensors on the Roomba, can be too sensitive if you have a significant number of them
Sometimes, failure can be a better teacher than success.
This lab went very well, I waited and watched what the flaws in our lab 8 design were for the track, then upon implementing them, we had a minor typo, that when resolved, had our Roomba successfully navigate the track with an ~100% success rate.



	
