### Commands running order
 To run :

    make 
    ./a.out

        
## Assumptions :

1. In the log file I'm storing the erroneous commands(except the erroneous commands containing log).

2. When a foreground process runs for more than 2 seconds I have used an array of structs fg_processes that keeps track of all foreground processes and prints them in the next prompt with their running times times separated by ';' and then the original prompt is restored.

3. I used a global variable total_fg_time that keeps track of the total time taken by foreground processes that run for more than 2 seconds and prints this in the prompt.

4. If there is no previous directory when '-' flag is enabled then I'm printing an error statement  "No Previous directory".

5. User implemented commands like hop,reveal,seek will not be preceeded immediately an &.

6. If the exit command is run as a background process (e.g., exit &), it will fork, print the PID, exit, and print a message indicating that the process exited normally.

7. In log commands like cmd , cmd "spaces" and cmd 
"tabspaces" are taken to be different commands.

8. <,>,>> will always be preceeded and succeeded by  atleast one space.

9. The maximum number of bg_processes,fg_processes that can be stored per session is 1000,4096.

10. A spawned process is either sopped or running.

11. In my log file I'm storing commands from oldest to newest.While printing them I'm printing them in the oldest to newest order.

12. The maximum length of input string is 4096.

13. Aliases execution is possible only when it is the only command present in the input.Same goes for the mk_hop() and hop_seek() functions.

14. If a command is erroneous then it also redirected into the output file.

15. When a series of piped commands are running only the last piped command excutes in background. 

16. In ping dump signals like SIGTRAP(signal 5),SIGILL(signal 4),etc.. are not sent.


### Colour Coding:

When an error occurs, it will be printed in red to make it stand out. Here are the color codes used:



      GREEN "\033[92m"   for files other than directories
      BLUE "\033[34m"    for directories
      WHITE "\033[97m"  
      RESET "\033[0m" 
      RED "\033[31m"      for Error messages
      YELLOW "\033[1;36m" for the shell prompt
      PURPLE "\033[1;35m" for printing the paths obtained by hopping

