# nanorc-mngr
Manager for nano syntax highlighting (.rc files) for C++

There are two usage modes: parsing code files to extract probable keywords
automatically (doesn't always work perfectly) and specifying keywords to be
added manually. 

The former is:

```
nanorc [options] [mode] [keywords]
```

with 

```
Options
    -v, --verbose
        Verbose general debugging information
    
    -l, --lexverbose
        Verbose debugging output for the lexing
    
    -x, --ctxverbose
        Verbose debugging output for 
        
    -u, --user (default)
        Modify the user-defined set of keywords
        
    -l, --lib
        Modify the set of library-derived keywords
        
    -b, --builtin
        Modify the set of builtin keywords
        
    -r, --recursive
        Search directories recursively when parsing keywords from files
    
    -y, --no-confirm
        Disable confirmation before write
        
Mode
    --add
        Add new keywords
    
    --remove
        Remove existing keywords
        
    --ignore
        Sets keywords to be ignored by future parsing but not if they 
        are added manually via the [add] mode.
```
