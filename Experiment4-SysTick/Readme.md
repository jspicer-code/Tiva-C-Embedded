# A Mock up report

## Names 
Initials if you dont want them to be public    
   
## Design Overview
This should be a general description.  It should make sense to your grandma / or a high school student.  It should explain what the project is, what it does, and how to set it up. Example:
<em> This device is a tool used in the machining industry to control rotational speed of the shaft of a lathe.  The device consists of a motor to rotate the lathe, a dial to control the speed manual, an input port to control the speed via a computer terminal, a screen to display the rotational speed, an encoder to measure the rotation, and two emergency stop switches for safety. 
    
## Hardware List
Motor - 240VAC  - Fleebl V2.0 - Norsten Mortaincs...        
Superphased Motor Encoder - 13 bit - Off-World encoders...    
...     
...    
Embedded web links, prices, etc.  

## Hardware Description
The Motor is connected to the encoder via JPF-23 cable, 3 wire, doubled shield.  The Encoder has a 13 bit resolution which allows the micrprocessor to control the motor to within N rpms, etc etc.  The schematic is show below.

![schematic](https://github.com/drnobodyphd/LAB_SETUP/blob/master/images/schem.jpg)    


## Software Description
C files added by users, special functions, important functions, software flow, etc.   Flowcharts are also fine.  

The file structure for this projects is as follows:

<pre><code>├── project_template
    |   ├── .gitignore                <em>(list of files to ignore in Repo)</em>
    |   ├── README.md                 <em>(a README file in markup language)</em>
    |   ├── project.uvprojx           <em>(the keil project file)</em>
    |   ├── project.c                 <em>(your main c file)</em>
    |   ├── project.h                 <em>(your main header)</em>
    |   ├── project.sct               <em>(scatter file)</em>
    |   ├── startup_rvmdk.S           <em>(startup file in asm)</em>
    |   ├── src                       <em>(Directory for your .c and .h )</em>
            ├── *.c files             <em>(*.c , the meat of the code)</em>
            ├── *.h files             <em>(*.h , the menu of the code)</em>
    |       └── ...  
    |   ├── inc                       <em>(Directory for precompiled libraries and headers)</em>
    |       ├── *.h files             <em>(*.h , the menu for micro controller)</em>
    |       ├── *.lib files           <em>(*.lib ,precompiled libraries)</em>
    |       └── ...  
    |   ├── driverlib                 <em>(Directory for driver libraries source)</em>
    |       ├── *.h files             
    |       ├── *.c files
    |       └── ...  
    └── </code></pre>

## Summary and Conclusions
Put something here that shows you learned something and / or you understand what you are talking about.  
