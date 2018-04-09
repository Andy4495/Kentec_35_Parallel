Kentec Parallel <Update with correct full name from vendor>
====================================

Ported version from Energia built in library <reference>

Based on SPI version of library, changed low-level interface functions.


Usage
-----
_See the sketch included in the `examples` folder._

Works same way as built-in SPI version.

<Do you need to copy it to Energia libraries folder, or will it work as-is in user directory library folder?>

<Data lines are always set to output mode, so make sure read signal is always high to avoid bus contention.>

<Slow operation -- may try to create fast mode for F5229 direct register access>

References
----------
+ [Kentec user guide][1]
+ [Driver chip user guide (from Kentec)][2]
= [Rei Vilo review][3]
