This README pertains to project two of Nachos
Group Members: Aaron Wells, Alex Powell, Cody Watson

#####################################################################################################################

Haiku

#ifdef RATED_PG
1)  Nachos you are
    a factory of sadness
    and some confusion
#else
2)  Nachos tries to fork
    but threads end up forking us
#ifdef REALLY_FORKED
    vasoline may help?
#else
    Berkely bastards
#endif   

#####################################################################################################################

Files that have been created / changed from original nachos2 release

    threads/main.cc
    threads/synch.cc
    threads/synch.h
    threads/system.cc 
    threads/system.h 
    threads/thread.cc 
    thread/thread.h

    test directory - Copied in given Tests and implementation of cp, cat, shell

    userprog/addrspace.cc 
    userprog/addrspace.h 
    userprog/bitmap.cc 
    userprog/exception.cc 
    userprog/fdset.cc 
    userprog/fdset.h 
    userprog/memorymanager.cc 
    userprog/memorymanager.h 
    userprog/processcontrolblock.cc 
    userprog/processcontrolblock.h 
    userprog/processmanager.cc 
    userprog/processmanager.h 
    userprog/synchconsole.cc 
    userprog/synchconsole.h
    userprog/syscall.h

#####################################################################################################################

Outline

    * Implementation of System Calls
        - Halt
        - Exit
        - Exec
        - Join
        - Create
        - Open
        - Read
        - Write
        - Close
        - Fork
        - Dup
    * Synchronous Access to the Console 
    * Implementation of Mulitprogramming with Time-Slicing
        - BitMap
        - Bring Data into Kernel
        - Virtual to Physical Address Translation
        - Timer Implementation
    * Implementation of cp and cat
    * Child/Parent Inheritance and Concurrency
    * Shell Execution

#####################################################################################################################

Implementation of System Calls

Sys Call:       Halt
How to Run:     Halt()
Requirements:   Run the halt system call in a C program which includes syscall.h
Explanation:    The halt system call traps to the kernel with value 0 in our implementation,
                this calls the halt function which stops Nachos and prints out performance statistics.

Sys Call:       Exit
How to Run:     Exit(int status)
Requirements:   The int status for a program that has completed successfully is 0, any other
                number indicates that the program did not complete a successful exit.
Explanation:    The exit system call gets the process control block for the child and the parent.
                If the parent is still alive, then the child can send its exit value to the parent.
                Lastly, the child will V a semaphore to let the parent know that the child has
                committed to the exit and has exited. The child then calls Finish() which will
                clean up the remnants of the child thread.

Sys Call:       Exec
How to Run:     Exec(char *name, char *args[])
Requirements:   To run the Exec command the first argument should be the name of the file the user
                wants to run. The second command should be the arguments to the exec'd file that
                the user wants to send.
Explanation:    The Exec call has two version, one without the args argument and the other with it.
                The Exec call without the args arguments is within ifdef CHANGED, the else is the
                Exec call that contains the args argument. We do not protect the user from entering 
                in an incorrect filename in either the name argument or the args[0] argument. The
                args[0] argument should also be the name of the program that should be executed.

                The Exec system call is the most extensive system call. The first step is the kernel
                getting the filename string from the user, which can be no longer than 42 characters.
                The next step is the kernel bringing in the second list of args of which there can be
                no more than 5 of 42 characters each. Each argument in args is brought into the kernel
                and stored in a data structure within the kernel. The exec call then continues by opening the file from
                the filename that the user provided. In our implementation, we hide the arguments
                "above" the stack of the exec'ing process. We get the stack pointer of the process
                and write the strings of the arguments right below it (since stack is written from
                the bottom up). Now, the pointers that are pointing to the arguments "above" the stack
                need to be stored. To do this, we assign the pointer to the argument, to a physical
                address beyond the stack pointer. Then the values of the number of arguments, the
                pointer to the arguments and where the new top of the stack will be, are all stored.

                In the Exec call within exception.cc there is a function called from the addrspace
                called Exec. This function is where the bulk of the work is done. This function takes
                the executable name and begins by clearing the page table for that child. We also need
                to zero out the address space, the data segment and the stack segment of the process.
                After this is zero'd out we need to copy the code and data segments into memory. We
                chose to do this a page at a time, constantly checking bounds to ensure we don't
                write more than a page worth of information before grabbing a new page. This results
                in two nasty for loops, one for the code segment and one for the data segment. Within
                the for loop we check to see if we are beginning to write in the middle of a page, if
                so we continue to write until that page is full. If we write only part of a page, we 
                keep track of the location we stopped writing so that the next segment can write 
                where we left off. The for loop for the data segment follows the exact same logic.
                When this function returns, we also call InitRegisters and RestoreState. These two
                functions work to initialize the registers for the child, and restore the state of
                the machine so the current address space can run. In order to initialize the registers
                of the child, we must put the PC counter at the "Start" of the program, we must tell
                the MIPS emulator where the next instruction will be, and we need to set the stack
                register to the end of the address space where we allocated the stack. However, on
                the allocation of the stack, we subtract 16 to ensure that we don't attempt to 
                reference something that is off the end of the stack. Finally, the exec call cleans up
                its data structures and begins running the user program. If the exec'd file cannot
                be opened, then the exec call immediately cleans up its data structures and returns
                a nonzero return value to register 2. After everything has been cleaned up, the
                failed exec call will increase the program counter and finish.

Sys Call:       Join
How to Run:     Join(SpaceId id)
Requirements:   Run the join system call which requires the id of the program that has finished
Explanation:    Join gets the id value from register 4 and from the child's process control block
                gets the pid of the child. The process that calls join, now P's a semaphore and 
                hangs until the child finishes. Once the child has finished, the child is deleted,
                the return value is written to register 2 and the PC counter is increased.

Sys Call:       Create
How to Run:     Create(char *name)
Requirements:   Running the Create system call has one argument for the name of the file that will
                be created. 
Explanation:    The create system call gets the string provided by the user which it will use to
                name the file. The file is then created and the PC is incremented. If the file 
                fails to create then we return nothing since Create is a void function.

Sys Call:       Open
How to Run:     OpenFildId = Open(char *name)
Requirements:   Running the Open system call has one argument for the name of the file that is
                desired to be open.
Explanation:    Open() syscall starts out similar to Create by fetching the filename from the user
                program and reading it into the kernel.  At this point we introduce the 
                ProcessControlBlock and FDSet objects.  We modified the addrspace class to contain
                a ProcessControlBlock object upon instantiation. The constructor takes in 3 parameters:
                the ProcessControlBlock of the parent process, the FDSet of the parent process, and
                the process id.  The FDSet is a custom class created for the sake of easy management
                of open files in the file system.  At the heart of the FDSet is an array of structs each
                containing an int and an OpenFile object.  The FDSet class contains methods like
                AddFD(), GetFile(), and DeleteFD().  To add a file descriptor to the FDSet we simply
                iterate through the array until the first null value is reached, then the new file
                descriptor becomes the index of the first null block found.  GetFile() and DeleteFD()
                behave similarly.  The important thing to note here is that a lock is placed around all
                of these methods to prevents any problems associated with simultaneous adds or deletes. 
                For example, in order for a process to add a file descriptor to the FDSet, it must first
                acquire the lock before iterating through the array, and then release the lock before
                returning.

Sys Call:       Read
How to Run:     Read(char *buffer, int size, OpenFileId id)
Requirements:   Read requires an empty buffer to put the data, the size of bytes you want to read, and
                the open file to read from
Explanation:    The Read() system call is very similar to write.  Arguments and user string are fetched
                in a similar fashion.  The difference is that when the OpenFileId is 1, indicating Console
                input we perform a synchConsole() read, but do nothing on ConsoleOutput.  On any other
                positive integer value we invoke a file->Read() method call.  Again, we don't want to halt
                on bad input so we simply do nothing if the returned OpenFile is null.  Since the Read()
                syscall has the requirement of returning the number of bytes read, this value is loaded
                into R2 before exiting. 

Sys Call:       Write
How to Run:     Write(char *buffer, int size, OpenFileId id)
Requirements:   Write requires a buffer which holds the data we will write. The size argument will
                indicate how many bytes to write and the last argument is the file to be written to.
Explanation:    There are three arguments for the Write() syscall, so these are transferred to the kernel
                by reading from register 4, 5 and 6.  Next the buffer string is fetched from user memory
                as before.  At this point there are several cases.  The OpenFileId is obtained via the
                file descriptor in R6.  If this id is 1 or NULL, no action is taken.  A value of 1 implies
                ConsoleInput and we don't want to write anything to the input.  If it is null-valued this
                implies some error has occured when opening the file, so again, no action is taken.  We
                don't want to perform any actions that would halt the machine on bad user input.  If the
                OpenFileId is 2 we write to the synchConsole and if it is any other positive integer we
                invoke the file->Write() method.  Finally, the program counter is incremented and memory
                is freed where necessary.

Sys Call:       Close
How to Run:     Close(OpenFileId id)
Requirements:   Close requires an open file which the user wants to close. The id provided will indicate
                which file we need to close since the user is done reading and writing to it.
Explanation:    Close is a system call that changes the FDSet object which holds all of the open file
                descriptors. Each FDSet object is contained within the aforementioned process control block
                of each thread. So we access this and delete the file descriptor for that particular thread
                to the given file.

Sys Call:       Fork
How to Run:     SpaceId = Fork()
Requirements:   The fork system call returns a value indicating to status of the creation.
Explanation:    Fork creates a clone (child) of the calling process. The parent will receive the SpaceId
                of the child as the return value of the fork. The child will receive a 0 as the return 
                value, if the parent receives a -1, it means the child was not successfully forked. Our
                fork system call begins by calling NewProcess() which gives us a PID. We then create a new
                thread with a new address space. The child will be an exact clone of the parent. To do this
                we need to copy the parents pages to the child's pages. This is a fairly straightforward
                process once the pages have been allocated to the child. This allocation is done through
                the bitmap which tells us which pages are free to be allocated, and which ones are taken.
                After the child is copied then the parent puts the child in it's own process control block.
                This way it can keep track of how many children it has, the child will also possess a pointer
                it its parents process control block so that it can send the return value back to its parent.
                The PC counter is then incremented and finally the new thread is ready to run and the parent
                gets the PID of the child when the child writes it to R2. The function Fork() possessed by
                the thread object is different from the system call but it allocates the child's stack and
                puts the child onto the scheduler.

Sys Call:       Dup
How to Run:     Dup(OpenFileId fd)
Requirements:   Copies the file descriptor of a given OpenFileId so that Id should be valid. If not the
                call will fail.
Explanation:    Dup is fairly straightforward, the fd given is read from R4. The thread calling dup then
                calls another function within the FDset.cc file which is also called dup. This function
                goes into the FDSet for the given thread, finds the first available spot in the array
                and creates a duplicate of the given file descriptor. If there is no available space
                then the function returns -1. The Dup system call was not previously given as a known
                syscall, thus we needed to create the assembly language stub. This was done in start.s
                in the test directory. This writes the Dup system call code, 10, into register 2 so that
                the exception can be properly handled by the system. Once the system reads the value 10,
                it will call the Dup code in exception.cc which was previously explained.

#####################################################################################################################

Synchronous Access To the Console

    In our implementation we added a file synchconsol.h and synchconsol.cc this class helps by adding 
    functions that provide the abstraction of synchronous access to the console. The functions that exist
    within the synchconsol.cc file are, staticReadAvail, staticWriteDone, SynchConsole, GetChar, Read,
    PutChar and Write. When a synchconsole object is created it creates a console, one semaphore for
    reading from the console, one semaphore for writing to the console and a console lock.

    staticReadAvail and staticWriteAvail both create a pointer to a SynchConsole then the static ReadAvail 
    function calls a ReadAvail() which will V the reading semaphore indicating that the console has data
    to read. staticWriteAvail calls a function named WriteDone() which will V the writing semaphore. This
    indicates that the console has data and a process has written something to the console.

    The console has two ways to gather data from the user. The first is through GetChar() which acquires a lock,
    waits on a P'd semaphore and when there is something to read, it reads the character and releases the lock.
    The function Read of the other hand reads an entire word or sentence from the console. It does this by 
    reading characters and putting them in a buffer, the size of how many bytes were read will be returned.

    For writing we have similar functions as those to reading. PutChar is a function that locks the console and
    outputs a character to the console, we then hang on a semaphore until the write has been completed. Similar
    to Read, Write outputs a buffer of length "size". The Write call outputs every character in a buffer to the
    console, waits for the writing to be completed (on a P'd semaphore) and then releases the lock so the console
    is free again.

#####################################################################################################################

Implementation of Multiprogramming with Time Slicing

    BitMap  
    
        In the explanation of the system calls we referred to a bitmap whenever we mentioned getting pages for
        memory. The way we implemented bitmap was with an array of bits. Each bit can be turned on or off
        which corresponds to if a page is being used or not. In our implementation, the bitmap is implemented
        as an array of unsigned ints. We then use modulo arithmetic to find the bit we are looking for. 

        Our bitmap class has functions Mark and Clear which do what you would intuitively think. Mark, sets a bit
        and marks that page as taken, likewise, Clear clears a bit which marks the corresponding page as free.
        Another function we needed to to find the first available page for a user process to use. For this we
        have the function Find, this finds the first bit that has not been set and marks it as in use. If there
        is no free bit, then the function returns a -1. NumClear is a function that returns the number of bits
        that have been unallocated. The last two important functions are FetchFrom and WriteBack. FetchFrom is 
        a function that initializes the contents of a bitmap from a nachos file, then the file is the place to
        read the bitmap from. WriteBack stores the contents of a bitmap to a nachos file meaning the file is
        where we write the bitmap to. 

        This is important for multiprogramming because we need a way to allocate free pages to multiple processes.
        There is no guarantee that a process will have sequential pages and therefore we need a map to tell us
        which pages are taken, and which pages are available.

    Bringing Data in and out of the Kernel

        In order to correctly implement multiprogramming, we needed to be able to bring information from userland
        into the kernel as well as from the kernel back into userland. During the checkpoint this was a strait
        forward process. Addressing was a 1 to 1 function, meaning the virtual to physical address was a 1 to 1
        translation. This made bringing in information fairly easy. However, we now have to have a page table
        per process which tells us the mapping from virtual to physical address. We will talk about the assignment
        of pages and creating the page table in the next section.

        We have three functions that gather information from the user. The first is getStringFromUser which takes
        a virtual address, a string and that string's length as arguments. We then read byte by byte from that
        virtual address into the kernel. The address translation of this function is done in the addrspace file
        rather than here. We end the function by placing a null byte at the end of the char * to terminate the
        array. Another function, getDataFromuser also takes a virtual address and length as arguments, however, it
        also takes a buffer rather than a string of data. This function follows the same logic as getStringFromUser
        except there is no null byte at the end since we are dealing with data and not a string. The last function
        we have to get information from userland is getPointerFromUser. This function gets a pointer from the user
        to a specific structure or object. We assume a 4 byte pointer as is custom to little endian. We then must
        do some bitwise shifting since memory is stored as characters. To access this pointer, we will need to read
        these four characters and turn them into a number. We can then return this number to access the pointer.

        The last two functions we have in this area return information from the kernel back to userland.
        writeDataToUser takes a virtual address, a char *buffer and the length of the data to be written. We then
        write the data byte by byte from the buffer to the virtual address of that data within the thread we are
        communicating with. Likewise writeCharToUser has the exact same logic as writeDataToUser except we are only
        writing a single byte. 

    Virtual to Physical Address Translation

        This was one of the more complicated aspects to code up. When we create an address space for a process we need
        to copy all of the information including the code and data into memory pages. To do this we first consult
        the bitmap to see what pages are free. We then get that page and write data to it in page modulo form. 
        Essentially we write as much on one page as possible, check the bounds to ensure we don't write off the page,
        then grab another page and write until that page is taken up. If the code information ends in the middle of a
        page, the data section begins writing in the middle of the page where the code section left off. However, we
        do not assign half a page to once process and half to another. If the end of the data section of a process
        only takes up half a page, the entire page is still dedicated to that process. 
        
        During this memory allocation is where the page table is created. This page table is the translation function
        between the virtual page and the physical page. We begin the translation with getting the virtual page by
        taking the virtual address and dividing by the page size. Now we need the offset, which we get by taking the
        virtual address modulo page size. We then have a separate function which gets the physical page by the page
        table which maps the virtual page to the physical page. Finally we return the physical page number multiplied
        by the page size, plus the offset which gets us the full physical address.

        Translating virtual to physical addresses happens often within system calls. Anytime information is brought
        in or out of the kernel, we have to translate the processes virtual address to find exactly where that piece
        of data is in memory. This translation is also pertinent to the Fork and Exec calls which require the loading
        and copying of memory into physical pages. It's important to note that the page table is contained within the
        addrspace object. Thus every thread has its own page table, however, the bitmap is not shared and is a 
        global data structure which is therefore lock protected.

    Timer Implementation

        In order to implement the time slicing we create a timer object in system.cc. We do not initialize the random
				number generator with any specific value so it will be generated with default values. This timer object will
				post interrupts that will yeild the current thread causes it to leave the cpu, even if it did not put itself 
				to sleep.

#####################################################################################################################

Implementation of cp and cat

    The implementation of cat is contained within the test directory. The test directory holds all of the test
    cases that were given to us in order to ensure things in Nachos were working properly. Here cat checks to make
    sure the arguments given are at least 2, if they are less than that we return -1 and exit. If the file we are
    displaying the cat of is -1 then we exit with a value of -1. If all qualifications are met then we read in a 
    character at a time from the file and write that same character on the console output.

    For the implementation of cp we are copying the contents of the first argument into the second argument. To do
    this we first check to make sure we have 3 or more arguments, if we don't we exit -1. If we pass this check,
    then we proceed to open the first file and then create the second file with the name given in the second argument
    If either the input is not a file we can open or the create fails on the second file we exit -1. After this
    has passed we read in one character at a time from the first file and write to the second file that same
    character. After this completes we close both files.

#####################################################################################################################

Child/Parent Inheritance and Concurrancy

    When children are forked from their parents, they are a copy of their parent meaning they have the same address 
    space as their parent process. This means that they inherit their parents file descriptor set and the parents
    information stored in the address space. However, the parent and the child are completely separate processes. 
    Once the child is forked it has its own FDSet that matches its parent, as well as its own address space which
    will match the parent. The child will also have its own Process Control Block. This is a data structure which
    holds, the processes PID, a list of child processes, a lock and contains the FDSet. This information allows
    parents to keep track of their children, and children to return their own PID to their parents.

    Concurrency is a tricky situation due to writing into files. If a parent and their two children all want to write
    to the same file, there needs to be some sort of regulations on who can manipulate that file. For example, we
    don't want one child to be outputting 5 As and then the second child outputs 2 or 3 Bs right in the middle
    of the first childs As. To do this we need a global data structure of every open file. Each file will now
    possess a lock upon creation that will be taken by the process that wants to write to this file. That way no more
    than one process at a time is able to write to a given file. We implement this in filemanager.cc. Every time a
    process wants to write to a file, it must first require the lock for that file. That lock is gotten by calling
    the GetLock function which will only allow that process to write to that file. The process will then release
    the lock so that another process can access that same file.

#####################################################################################################################

Shell Execution

    The shell was given to us and we added additional functionality to run our system calls. The shell will take a
    maximum of 4 arguments of 60 characters each. The shells main functionality is to fork a clone of itself to run
    the command the user types into the shell. The first commands we support are the redirection commands < and >.
    These commands redirects stdin and stdout from the console to a file. In the case of redirecting stdin, if the
    file cannot be opened or does not exist then the shell execs the command without any redirection and waits for
    input from console input. In the case of redirecting output, if the file we are redirecting to does not exist,
    then the command will execute and stdout will not be redirected and will continue to be console output. If the
    file that the command is working on does not exist then no redirection is possible. We will still try to exec
    that command with the given arguments and let the command handle the errors of the files not existing. In our
    system, the redirection calls must be single space delimited with no trailing spaces. Formatting the arguments
    and commands in any other way is undetermined behavior. Ex. "cat file1 > file2" Bad Ex. "cat file1 > file2  ".
    In order to test the shell's functionality as well as the tests given for this project, you must run these files
    from the test directory.

