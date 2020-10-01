# Pad

Pad is programming language.

# Build

How to build of the pad.

## UNIX

    $ git clone https://github.com/narupo/pad
    $ cd pad
    $ make init && make
    $ ./build/pad -h


## Windows

Using MinGW's make (recommend TDM-GCC).

    $ git clone https://github.com/narupo/pad
    $ cd pad
    $ mingw32-make init && mingw32-make
    $ ./build/pad -h   

# Syntax and feature

```
{@
    for i = 0; i < 4; i += 1:
        puts("Hello", i)
    end
@}
```

