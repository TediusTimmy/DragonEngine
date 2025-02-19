Dragon Engine
=============

I'm writing an engine for re-implementing Dragon Quest / Dragon Warrior. It will probably be a mash of the two.


Notes
-----
The grammar of Backwards has a bug in it in that one cannot store a function in an array and then call it by
accessing the array.  
This is broken:
```
   array[index]()
```
This version of Backwards fixes this bug.
