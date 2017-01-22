# argsplus

C++ argument parsing library, essentially a rewrite of args allowing a better interface

# What does it do?

Nothing yet. Stay Tuned.

# What will it do?

* lets you handle flags, flag+value, and positional arguments simply and
  elegently, with the full help of static typechecking.
* allows you to use your own types in a pretty simple way.
* lets you use count flags, and lists of all argument-accepting types.
* Allow for required arguments and validated groups of arguments, with pretty
  output in validation failures.  Groups may be possibly nestable.
* Generates pretty help for you, with some good tweakable parameters.
* Lets you customize all prefixes and most separators, allowing you to create
  an infinite number of different argument syntaxes
* Lets you parse, by default, any type that has a stream extractor operator for
  it.  If this doesn't work for your uses, you can supply a function and parse
  the string yourself if you like.
* Lets you decide not to allow separate-argument value flags or joined ones
  (like disallowing `--foo bar`, requiring `--foo=bar`, or the inverse, or the
  same for short options).


# What will it not do?

* Allow one value flag to take a specific number of values (like `--foo first
  second`, where --foo slurps both arguments).  You can instead split that with
  a flag list (`--foo first --foo second`) or a custom type extraction (
  `--foo first,second`)
* Allow you to intermix multiple different prefix types (eg. `++foo` and
  `--foo` in the same parser), though shortopt and longopt prefixes can be
  different (longopt prefixes will take precidence, so make sure the longopt
  prefix doesn't match the beginning of the shortopt prefix, or you won't be
  able to use shortopts).
* Allow you to have value flags only optionally accept values
* Allow you to make flags sensitive to order (like gnu find), or make them
  sensitive to relative ordering with positionals.  The only orderings that are
  order-sensitive are:
    * Positionals relative to one-another
    * List positionals or flag values to each of their own respective items

# It might in the future

* Allow you to create subparsers somewhat like argparse, through the use of
  kick-out arguments (check the gitlike.cxx example program for a simple sample
  of this)

# How do I install it?

```shell
sudo make install
```

Or, to install it somewhere special (default is `/usr/local`):

```shell
sudo make DESTDIR=/opt/mydir install 
```

You can also copy the file into your source tree, if you want to be absolutely
sure you keep a stable API between projects.

## I also want man pages.

```shell
make doc/man
sudo make installman
```

This requires Doxygen

## I want the doxygen documentation locally

```shell
doxygen Doxyfile
```

Your docs are now in doc/html
