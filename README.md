# libNumHop

A simple string parsing nummerical calulation library written in C++.
It inteprets strings with mathematical expressions and allows creation and reading of internal or external variables.

Internal variables are local within the library, external variables are such variables that may be pressent in some other code using this library.

The library supports the following operators: = + - * / ^ and expressions within ()
Scripts can have LF CRLF or CR line endings


## Usage examples

```
# The Hash character is used to comment a line
a = 1; b = 2; c = 3    # Multiple expressions can be writen on the same line separated by ;
d = a+b*c              # d Should now have the value 7
d                      # evaluate d (show value of d)
d = (a+b)*c            # d Should now have the value 9
d = d / 3              # d will now have the value 3
```

```
a=1/2/3/4/5            # Will be transformed into 1/2*1/3*1/4*1/5
```

For more example see the included "test" executable

## Build instructions
The library uses qmake and QtCreator as the build system, but the library is plain C++ and Qt is technically not required.
The library files can be directly included in an external project or the library can be built as a shared or static library.

## Implementation details
The library builds a binary tree from the expressions, each operator will have a right and left hand side.
Each operator will represnt a node in the tree, the leaves are the nummerical values or variables that needs to be evaluated.

The variable storage can be extended with access to external variables by overloading members in a class for this purpos.
This way you can access your own variables in your own code to set and get variable values.

See the doxygen docuementation for further details.
