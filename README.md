# UIDRESP

minimal CLI tool to simulate UID responses in AISG device discovery.

## BEHAVIOR

takes a list of full UIDs as arguments, then reads input lines from stdin:

- if exactly one UID matches → prints it
- if multiple match → prints a generated garbage collision
- if no match → prints nothing

## MATCH RULE

an input string is split into:

- first two characters (`left`)
- remaining characters (`right`)

a UID matches if:

```
uid.startswith(left) && uid.endswith(right)
```

example:  
`input = "12xx56"` matches `uid = "12abc56"`

## BUILD

```bash
make        # build uidtool
make test   # run unit tests
```

## USAGE

```bash
./uidtool 12341234 12349875976 12340870987076
```

then type input lines interactively (e.g. `1234`, `129076`, etc.)

## TESTS

unit tests cover:

- `UidResponder::matches`
- `UidResponder::generateCollision`

framework: GoogleTest, see `Makefile`.

## author

crazybrake <crazybrake -sobaka- gmail dot com>, 2025

