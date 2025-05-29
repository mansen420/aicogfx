in the include/window.h file

1. what does extern do

Let me drop some knowledge on you.

The simple answer: It says "This symbol is declared here, but defined somewhere else” to the compiler.

The real asnwer:

`extern` will mark a variable as being externally linked. So what does *that* mean? 
To understand that, you need to understand linking and the ODR (One Definition Rule).
In C/C++, we take 3 steps from source-code to binary executable:
    1. Pre-processing. This is the pre-processor resolving all the commands that start with `#` in your code. We call the pre-processed file the "translation unit" (TU). It is this TU that is fed into the compiler as input. 
    For example, if your code `#include`s `stdio.h`, the pre-processor will go ahead and paste the entire contents of that file (given that it can locate it/it actually exists), and the compiler will see the result of *that*.
    2. Compilation. This is the compiler. It is extremely important to understand that the compiler only ever compiles *one* TU at a time. I.e., your compiler cannot see definitions outside the immediate TU it is currently processing. 
    The output of the compiler is object code. Object code is binary, but with some unresolved symbols.
    3. Linking. The linker will receive all the object codes of all your TUs.
    Now, the reason we even *have* to link is that the compiler treats each TU as a completely independent C/C++ program, but it is unreasonable to expect programmers to put everything in one file (hello 20KLoC .c file?).
In addition to all of the above, C/C++ has the One Definition Rule, which states that language constructs may not be defined more than once (for obvious reasons), but, crucially, *can* be declared multiple times.
In other words
    ```C
        void foo(); //declaration
        void foo(); //ok
    ```
Is allowed, but 
    ```C
        void foo(){}; //definition
        void foo(){}; //not ok
    ```
is not.
Now that we have covered the ODR and linking, let's discuss how C/C++ devs decided to solve the problem: How can I share definitions across c/cpp files?
Header files. They are headers because you include them at the beginning of your TU.
These header files (in theory) include declarations of various language constructs you want to share across files, the compiler will not complain that a function is not defined, as long as it can see the declaration.
In other words:
    ```C
        //math.h
        float sqrt(float); //declare function that takes and returns a float.
        [...]

        //test.cpp 
        #include "math.h"
        [...]
        result = sqrt(x); //compiler can't see sqrt definition, but that's fine.

        //math.cpp
        #include "math.h"
        
        //sqrt defined here, the linker will direct sqrt from test.cpp to this definition.
        //there is only *one* definition of sqrt, but possibly many declarations.
        float sqrt(float x)
        {
            [...]
        }
    ```
Now, what you saw above *is* external linkage, in the case of functions.
In the case of plain variables, such as `int x`, the declaration and definition are one and the same, i.e., `int x;` is *both* a declaration and a definition of `x`.
So, how can we share variables across TUs? Simple, allow
    ```C
        extern int x; //this is only a declaration. 
    ```
Now, `int x` may be defined (once, and only once), in some TU. 

The above explained external linkage, as a bonus, there is a different type of linkage, called internal linkage. 
By default, functions are externally linked, variables need `extern`, but both functions and variables can be declared with `static`, a la `static int x;` or `static void foo();`. 
So, what does internal linkage mean? It means that a variable or function definition is *only* visible inside the TU.
What that means is that, if you have `static int x;` in a header file, and include that header file in A.cpp and B.cpp, both files will have their own `int x`, and these are completely independent. Same for functions.

2. whats INIT = 1 << 0
    `INIT` is an integer given the value `1 << 0`, which is 1 (binary 00..001) being shifted 0 times to the left. So what's the big deal? Well, once we add more flag bits, we'll do this
    ```C
        //example 
        SHUTDOWN = 1 << 1;
    ```
This is the binary 00..001 shifted once to the left, so 00..010. 
This *should* answer your question, but I'm sure you're still confused as to what we're doing with these binary numbers. 
Remember: We have the bitset `engine_flags`, this is a binary number where every bit corresponds to an engine state (e.g., the first bit tells you if the engine is ON or OFF), and we want to be able to easily check that state, so we do something like 
    ```C
        if(engine_flags & engine_flag_bits::INIT) //equivalent to engine_flags & 00..01 
            //engine is ON
    ```
We might also do 
    ```C
        engine_flags = engine_flags | engine_flag_bits::INIT; //set the INIT bit
    ```

3. what are the "@" symbols for example, line 42 "@class" and line 43 "@brief" what is brief?
    Documentation symbols for Doxygen, a standard doc generator or smth. I just use an automatic tool that generates doc symbols for me and I fill them out, it's called Neogen.

4. honestly this entire structure i don't get, what is it doing? all i know is that structs have public by default opposed to the class' private default
    As far as structs v.s. classes go, yeah, that's the only difference. But I get why you don't understand what the `wndctx` struct is doing.

in the src/window.cpp file

1. line 9 "aicogfx::opres aicogfx::init()", is "opres aicogfx" a namespace or are they two separate entities?
    Let's be clear, `aicogfx::opres` is the return type of the function named `aicogfx::init`.

2. why is everything in a struct? starting from line 27?
    This is the definition of the struct `_impl` which contains the private implementation details of `wndctx`, more on that later.

3. lines 54,55: "glfwSwapBuffers" and "glfwPollEvents" i have no idea what they do
    swapbuffers will swap the front and back buffer of the system window. Look, we only render to the back buffer, and what you see on the monitor is the front buffer, so once we finish the rendering, we flip them (i.e., update the screen). This is called double-buffering. Look it up.
    As for pollevents, afaik it just collects OS events like key presses or mouse movement. You can then check those using other functions.

4. line 62: what's the colon operator? in the middle of "wndctx([...])" and "implptr[...]" 
    This is not an operator. This basically initializes class member variables before the constructor body. So, in line 62 I'm initializing `implptr` before the body. This is just a C++ way of initializing member variables.

note: [...] means that there are stuff there that i didn't bother writing.

in the test/window_test.cpp file

1. what is mainctx (this is the first time it has ever been mentioned i think maybe)
    Ah, this one will require some explanation. Basically it is a structure I made to control the program flow. It basically takes control of the thread that calls it, this gives us some nice guarantees once we start calling OpenGL functions. Basically we're making a whole class of bugs impossible in the code.
