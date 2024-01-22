# A Compiler for the Grace Progamming Lanuage 📈

## Compiler Features 👀
- optimising 💀
- llvm backend 🔨
- the integer type is implemented in 64-bits only (its 2024)
- strings can contain unicode characters 😂👌

## About the Grace Language 🥵
- it's like pascal and C
- it has nested functions and call by reference
- it has interesting comments
- it supports strings and has some built in functions to manage them (included in the runtime library)
- look at the [spec](https://https://courses.softlab.ntua.gr/compilers/2023a/grace2023.pdf) to find out more 🖖
- look bellow for example programs to get the idea of the language (or to use the compiler 🤭😳)

## Repositories Containing Programs Written in Grace 🔢
- https://github.com/kostis/ntua_compilers/tree/master/grace/programs 🔫
- https://github.com/giannosr/grace_programs

## More Resources for Grace 🤓
- a plug in for vscode which includes syntax highlighting among other resources can be found [here](https://github.com/kostis/ntua_compilers/tree/master/grace)

## About the Runtime Library 🚀
- it was based on [this](https://github.com/tomkosm/ntua-grace-runtime-lib) 🧎‍♀️
- im not sure its makefile works correctly 🤔
- it relies on C's library
- it uses C's string functions directly

## Installation 🔥
```shell
make
```
then
```shell
make clean
```
removes all files produced but not need to run the executable (i.e. object files etc.) 😇

### To uninstall 🥺
```shell
make distclean
```

## Usage 🤯
```shell
./grc.py [flags] program.grc
```
produces the executable ```program``` (files are not required to end in ```.grc``` 📄)

❗ if flags ```-i``` or ```-f``` are not set, then two more files are produced:
- ```program.ll``` - contains llvm ir
- ```program.s```  - contains assembly

### Flags 😏
- ```-O``` *optimise* 💀
- ```-f``` *final code* 🤖 - read prorgam from stdin and put final code in stdout
- ```-i``` *intermediate code* 👽 - read prorgam from stdin and put intermediate code in stdout
