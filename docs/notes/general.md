## Megha Operating System V2 - General
## To POSIX or not to POSIX
---------------------------------------------------------------------------------------------------
_16 July 2021_

Well POSIX is one of a kind standard in the computer software field. In this world of proprietary 
softwares, POSIX makes it possible for a 'compliant' software to run on any other POSIX compliant
Operating System.

Several popular software, awk, ed etc. are already standardized and thus can be easily be ported to 
another POSIX Operating System.

### Advantages of being POSIX complaint

1. Career and being part of a group:
   Much better understanding of the popular operating systems, as most are POSIX complaint 
   themselves.

2. Portability of applications: 
   POSIX compliant applications can be ported and not have to be written from scratch. 
   Building tools like C compiler or assembler can be very time consuming to write, such 
   applications can be ported much quickly if MeghaOS is POSIX compliant.

   It is not a necessary to always port, but POSIX compliance will make it easy, if and when 
   required to port another application to MeghaOS.

3. Use Linux/GNU for prototypes:
   Because MeghaOS and Linux will both be POSIX compliant. I can build prototypes and test 
   algorithms on Linux first then build it for MeghaOS.

### Disadvantages of being POSIX complaint

1. Tied to a standard: 
   MeghaOS will become one of hundred of other POSIX compliant, UNIX-like operating systems. 
   But I also fear, that chasing the standard will make MeghaOS very boring to build.

   Say I go for POSIX 1.0, then I will have to build the following according to the standard.
       I/O, threading, File System, Process, Signals, Pipes, C Library 

   That means, I cannot really experiment with them a lot. Any change in them will break the POSIX 
   compliant applications.

2. Too much work:
   I want MeghaOS for experiments and use it to learn about operating systems, compilers, graphics 
   and other very basic softwares that we all take for granted these days.

   POSIX compliance is a whole other level of work and that will take me in a different path than 
   actually using the operating system for experiments.

3. Porting hell: 
   Being POSIX compliant will allow me to port standardized applications easily on my system. 
   But most applications today have a ton of dependencies. So that means to port all the 
   dependencies as well.

   If I port sometimes and not always, then there is a risk that:
   a. At the time of porting, its dependencies were not ported before, so need to ported now with 
      the main software.

   b. Duplicate work. I was not able to foresee the port and build another library with similar 
      features but is not POSIX compliant. That means now I will have to 
        * either break the POSIX compliance of the ported application and use the library I had 
          built. This will be very bad, in case we want to port the latest source later on 
          (say to get a fix).
        * or build a new POSIX compliant wrapper library just for the application.

4. Not able to lean the basics:
   Porting is good for software availability, but not so much about learning. I will be able to 
   learn to solve porting problems, but not be able to lean the implementation.

   If I want to lean Linux, then I will learn it on Linux, why will I build an Operating System 
   similar to Linux and learn from it. I mean Linux source code is available right.

5. Porting of bugs:
   When I port an application, I will most likely port it flaws as well. So lets say, I ported 
   'ssh' and there is a security bug that is discovered. What are my choices:

   a. Wait for the original author to fix it and then port the software again.
   b. Fix it myself. That now means, I will have to test it and maintaining the fix myself from now
      on.

In conclusion, I do not want MeghaOS to be completely POSIX complaint. 
I could have a POSIX layer between applications and kernel, which provides the POSIX functions
from a library and header files. This layer is going to implement specific POSIX functions when 
necessary on its own. The kernel syscalls, file system, process model, IPC will not be made with
the POSIX compliance in mind, so ideas could be taken from POSIX, DOS, WINDOWS as seen fit.

Need to be visited later on, after process model, file system is implemented.
