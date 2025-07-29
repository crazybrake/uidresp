# UIDRESP and UIDSCAN

minimal CLI tools to simulate and scan UID responses in AISG device
discovery.

## COMPONENTS

### uidresp

simulates AISG device responses to UID patterns.

#### behavior

takes a list of full UIDs as arguments and reads input lines from stdin:

- if exactly one UID matches the pattern → prints it
- if multiple match → prints a collision string (garbage-like output)
- if no match → prints nothing

#### match rule

an input string is split into:
- first two characters (`left`)
- remaining characters (`right`)

a UID matches if:

```c++
uid.startswith(left) && uid.endswith(right)
```

example:  
`input = "12xx56"` matches `uid = "12abc56"`

#### build

```bash
./bootstrap.sh 
./configure
make
make check
```


#### special commands

when running interactively, `uidresp` accepts the following control commands:

- `SETADDR:<UID>` — manually assign address to a UID (e.g. for simulation)
- `RESETADDR:<UID>` — remove previously assigned address from UID
- `RESETALL` — clear all assigned addresses

#### run

```bash
src/uidresp AB12345678901234567 AB02345678901234567
```

then type pattern lines interactively, e.g.:
```
AB
AB3
AB567
```

### uidscan

actively discovers UIDs using pattern refinement based on collision
responses.

- iteratively refines the search by expanding the pattern inwards
- collects and mutes discovered UIDs
- supports automatic scanning over `socat`

#### example (socat)

you can run `uidresp` and `uidscan` in pair using `socat`:

```bash
socat exec:"src/uidresp AB12345678901234567 AB02345678901234567" \
  exec:"src/uidscan AB"
```

this will run a full scan for all devices that match prefix `AB` and
follow the expected UID format.

## tests

unit tests cover:
- `UidResponder::matches`
- `UidResponder::generateCollision`

use `make check` to run tests.  
for more detailed output:

```bash
src/test_uidresp
```

framework: GoogleTest

## author

crazybrake <crazybrake -sobaka- gmail dot com>, 2025
