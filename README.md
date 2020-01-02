# miniOS
Project for university course. It implements the followings:
    - Defining paging tables  
    - Activates paging  
    - Switch to Long Mode: 64bits  
    - Program PIC. Add interrupt handlers. Dump trap frame 
    - Program timere  
    - Program keyboard  
    - Implementation of a console  
    - Read sectors of a disk using PIO mode ATA
    - Memory management: Allocates physical, virtual and heap memory  

# Prerequisites
-Bosch: Download from http://bochs.sourceforge.net/getcurrent.html and save it to:  
./utils/Bochs-2.6.8  
-Nasm: Downlaod from https://www.nasm.us/ and move the .exe file to:  
./utils/nasm.exe  
-Python3: Install it from https://www.python.org/downloads/  
-Visual Studio  

# How to run it
-On the root directory run:  
    python3 utils/makeDisk.py  
-Load the project in Visual Studio  
-Build it in x64 Release 
-Start the OS runing the script q.cmd in terminal  

