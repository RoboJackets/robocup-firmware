Note: Any place where <variable> is used should be replaced by the user with the appropriate variable.

## Introduction
For debugging C and C++ knowing how to use GDB (GNU Debugger) will be invaluable. For that reason we have  assembled this tutorial to get you started with basic features of GDB as well as a few features which we found invaluable to new members.

## Installing gdb
To install gdb use the following
```
sudo apt install gdb
```

## Starting gdb
To start gdb, type "gdb <filename>" where <filename> is the name of the compiled file you wish to debug (the name you type to run your program).
```
$ gdb ./soccer
```

If this is successful you should now see the gdb prompt:
```
$ (gdb)
```

## GDB Commands Reference
Below are some other common gdb commands along with their abbreviated versions. All these commands can be directly replaced with their abbreviated versions.
```
run       r
break     b
print     p
next      n
step      s
list      l
continue  c
info      i
disable
delete
quit      q
```

### Run
To actually run the loaded program use the run command (which can be shorthanded to r)
If we run the program without any breakpoints set it will either run to completion or an exception will occur and execution will stop.
```
$ (gdb) run
```

### Break
Breakpoints will stop the execution of the code when they are activated. You can set a breakpoint to trigger on the code reaching an any function (by name) or line number.
To set a breakpoint on a function:
```
$ (gdb) break <function_name>
```

To set a breakpoint on a line number:
```
$ (gdb) break <filename>:<line_number>
```

### Print
After you reach a particular point in your code with a breakpoint you undoubtedly want to see what is going on there. Print allows us to print the values of variables currently defined (in-scope). To print the value of a variable speed use:
```
$ (gdb) print speed
```

### Next
After encountering a breakpoint you will probably want to look around at nearby code. To step forward one line of the program we use step or next. When using step the debugger will go to the next instruction the flow of the program including stepping into function calls. On the other hand next will only go to the next line in the current function (stack frame) and thus avoid function calls.

To step once:
```
$ (gdb) step
```

To next once:
```
$ (gdb) next
```

To step or next multiple times you can append the number of times you would like to step to the end of the line as follows:
```
$ (gdb) step 10
```

### List
If you have stepped forward many times or been printing out a large amount of information you may forget where you are in the program or the surrounding code. List (with its default usage) will list 5 lines above and below the one the debugger is currently at.
To use list:
```
$ (gdb) list
```

### Continue
If you have multiple breakpoints you can resume execution until the next breakpoint(or error) with continue.
To continue:
```
$ (gdb) continue
```

### Info
Info is main command for getting info about the current state of the computer, program and debugger. Try using the default info command:
```
$ (gdb) info
```

You should now see a help menu describing all the various information info can provide you with including stack contents, register contents, etc. For starters the most useful info will be on breakpoints:
```
$ (gdb) info breakpoints
```

This will print a numbered list of your current break points as well as whether or not they are disabled.

### Disable
To temporarily prevent a breakpoint from stopping execution when we pass it we can disable it. This is done with the disable command followed by the index of the break point as given by info breakpoints:
```
$ (gdb) disable 2
```

### Delete
To permanently remove a breakpoint you can use delete followed by the index of the break point as given by info breakpoints:
```
$ (gdb) delete 2
```

### Define
For frequently used operations, especially prints, GDB allows you to a sequence of commands with the define command.
```
$ (gdb) define mydump
```
gdb will reply with the following message:
```
Type commands for definition of "mydump".
End with a line saying just "end".
```
After which we might define our desire function. For instance a function that prints the length of an array of pointers ptrs[] as well as the value of:
```
print i
print ptrs[i]
print *ptrs[i]
end
```
