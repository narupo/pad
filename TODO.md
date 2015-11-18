# BUG

## realpath says:

	==5931== Syscall param getcwd(buf) points to unaddressable byte(s)
	==5931==    at 0x4127221: getcwd (getcwd.c:80)
	==5931==    by 0x408B485: realpath@@GLIBC_2.3 (canonicalize.c:88)
	==5931==    by 0x804ADFA: file_solve_path (file.c:26)


# File Structure

## File Name Format

[language name]-[name].[cap suffix]
[language name]-[name]-[name].[cap suffix]

c-array.cap
c-include-guard.cap

## File Content Format

Plain text

# Directory Structure

cap/c-array.cap
cap/c-include-guard.cap

or

cap/c/array.cap
cap/c/include-guard.cap


# Convert name roules

snake <-> camel
etc

array_new_size <-> Array_new_size
array_new_size <-> Array_NewSize

# Install System

$ cap install c-array

# Command View

$ cap c-include-guard MYNAME
#ifndef MYNAME_H
#define MYNAME_H

#endif



