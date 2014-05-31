# Whitespace interpreter

This is a whitespace language interpreter written in C++. The whitespace language is an esoteric programming language whose specifications are defined [on the whitespace page](http://compsoc.dur.ac.uk/whitespace/). 

Right now not all the instructions have been written, but it can run count.ws and count2.ws

Next goals :

 - Make it interpret the whole instruction set. That's 24 instructions, it seems doable.
 - Turn the interpreter into a compiler. Yes mam'.

## Compilation

Nothing fancy actually, this is just some quick and dirty code with everything in the same file.

	g++ main.cpp -o white

## Running a whitespace program

	./white examples/count2.ws

## But... why ?!?

Because it's fun !

## Credits

All the whitespace code samples in the example directory come from [the whitespace page](http://compsoc.dur.ac.uk/whitespace/)