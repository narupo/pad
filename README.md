# Pad

Pad is programming language.

# Build

How to build of the pad.

## UNIX & Windows

    $ git clone https://github.com/narupo/pad
    $ cd pad
    $ make init && make
    $ ./build/pad -h

On Windows, Need `ws2_32` dll.

# Syntax and feature

Bubble sort program.

```
{@
    // Bubble sort program
    arr = [3, 2, 4, 1]
    k = 0
    flag = true

    for flag:
        flag = false
        for i = 0; i < len(arr) - 1 - k; i += 1:
            if arr[i] > arr[i + 1]:
                tmp = arr[i]
                arr[i] = arr[i + 1]
                arr[i + 1] = tmp
                flag = true
            end
        end
        k += 1
    end

    for i = 0; i < len(arr); i += 1:
        puts(arr[i])
    end
@}
```

Structure and functions and methods.

```
{@
    struct Animal:
        name = nil

        def new(name):
            return Animal(name)
        end

        met getName(self):
            return self.name
        end
    end

    animal = Animal.new("Tama")
    puts(animal.getName())
@}
```

Functions and inject-block statement.

```
{@
    def base(title):
        block header: @}
            <h1>{: title :}</h1>
        {@ end

        block content:
        end
    end

    def index(title) extends base:
        inject content: @}
            <p>Welcome to my homepage!</p>
        {@ end

        super(title)
    end

    index("Pad")
@}
```

# Development policy

* Pad is interpreter Like C 
* Pad keep simple
* Pad needs many test cases
* Pad needs many standard libraries

