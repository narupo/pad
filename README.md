Cap is simple snippet manager for programmer.

Usage:

    cap command [arguments]

The commands are:

    help    display usage
    cat     display cap file
    ls      display cap file list
    cd      display or set current directory path
    edit    edit cap file
    editor  display or set editor path
    deploy  deploy files from directory
    make    display and make by cap's make roule
    path    display normalized path of cap file
    run     run script


Usage:

    cap cat [name]

The options are:

    -h, --help     display usage
    -[0-9]         key number of replace
    -s, --separate display by separate name
    -d, --debug    debug mode

The options details:

    -[0-9]
        Replace @cap{} braces to number's value.
        If has not replace number then case of @cap{0:default-value} to "default-value".
        Else to "replace-value".

        CapFile:

            @cap{0:default-value} @cap{1:default-value}

        $ cap cat -0 "replace-value" -1 "more" capfile

    -s, --separate
        Separate CapFile by separate name.

        CapFile:

            @cap sep my-sep-0
            @cap sep my-sep-1
            @cap sep my-sep-2

        $ cap cat -s "my-sep-1" capfile


Usage:

    cap make [make-name] [options]

The options are:

    -h, --help display usage
    -d,        debug mode

The cap syntax:

    @cap brief [string]   brief
    @cap mark [mark-name] mark for goto
    @cap goto [mark-name] goto mark

The cap syntax commands:

    @cap cat  like a "cap cat" command
    @cap make like a "cap make" command
    @cap run  like a "cap run" command