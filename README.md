# minikeyg

faster minikey generator for Casascius keys

**Please read all this document before to use the program**

## Download

To clone the repository:

```
git clone https://github.com/albertobsd/minikeyg.git
```

don't forget change to the minikeyg directory

`cd minikeyg`


## How to build

Just compile:

```
make
```

# usage

This program only have a few possibles arguments

```
./minikeyg -u 
```

the `-u` option is for generate keys without the byte verification, `u` stand for `unverified` keys,  if you don't specify the keys generated will be always valid with the byte verification.

```
./minikeyg -s 22
```

the `-s 22` or `-s 30` option is for generate only keys of the specified size, `s` stand for `size`, if you don't specify this option the default value is 22

```
./minikeyg -m SECUN34uVQKk3UkMBAiTLSoLTUWfwS
```
the `-m` option is for specify a star key this string need to fit in 22 or 30 characters `m` stand for `minikey`, if you don't specify this option the star value is chossen randomly

```
./minikeyg -a 23456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz
```

the `-a` option is for specify the alphabet to be use I hear that the Series 2 Coin don't have number 1 in his minikey, if you don't specify this optin the defaul alphabet is `123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz` 

```
./minikeyg -t 2
```

the `-t` option is for specify the thread number, default number of threads is 1.

```
./minikeyg -r
```

the `-r` option is to generate just random keys without any special order, `r` stand for `random`. This option is OS dependent and only works with `/dev/urandom`

Of course you can use all the option together to fit you needs.

## Just a key generator

This program is just a generator, this generator is already build in keyhunt

You need redirect the output og this program to brainflayer please check https://github.com/ryancdotorg/brainflayer

## Free Code

This code is free of charge, see the licence for more details. https://github.com/albertobsd/minikeyg/blob/main/LICENSE

This is a hobby for me but is still a lot of work.

## ¡Beta!

This version is still a **beta** version, there are a lot of things that can be fail. And absoluly there are some bugs 

# Donations

- BTC: 1Coffee1jV4gB5gaXfHgSHDz9xx9QSECVW
- ETH: 0x6222978c984C22d21b11b5b6b0Dd839C75821069
- DOGE: DKAG4g2HwVFCLzs7YWdgtcsK6v5jym1ErV

All the donations will be use only for two things:

- Buy coffee to develop from dusk till dawn
- Get an affordable desktop computer with decent GPU, not highend, just to start the GPU version of all my programs
