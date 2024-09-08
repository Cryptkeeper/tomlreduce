# tomlreduce

A tool for reducing TOML data into its compacted format.

## Motivation

I work with a large amount of TOML files that are frequently edited by various people, resulting in a mix of formats. [Taplo](https://taplo.tamasfe.dev) and other TOML formatting tools are used to mitigate this volatility. However I have found many TOML formatting tools mostly only support (what I refer to as) TOML's "expanded" form:

**Expanded Form**

```
[[contacts]]
name = "Bob John"
address = "123 Example St"

[[contacts]]
name = "Alice Sally"
address = "456 Example Dr"
```

In many cases, I would prefer the array to be reduced to a inline assignment, or if required, a "compacted" form.

**Inline Assignment**

```
contacts = [{name = "Bob John", address = "123 Example St"}, {name = "Alice Sally", address = "456 Example Dr"}]
```

**Compact Form**

```
contacts = [
    {name = "Bob John", address = "123 Example St"},
    {name = "Alice Sally", address = "456 Example Dr"}
]
```

tomlreduce achieves this by loading the TOML data using [tomlc99](https://github.com/cktan/tomlc99) and then recursively re-printing the data in a reduced/compacted format, where possible. Large or overly complex data structures will be printed in the typical "expanded" form.

## Disclaimer

You should probably not use this. I use this tool daily for various workflows, but it isn't a "fully supported" formatter and it may unexpectedly rewrite or corrupt your TOML files. This tool serves a purpose, but it is something which should instead be offered natively by formatting tools instead. In summary: it works, but it probably is less of a solution and more of a fun project.

## Compiling

1. Clone the repository and its dependency submodules using `git clone --recursive https://github.com/Cryptkeeper/tomlreduce`
2. Build the project using CMake: `cmake -B build`
3. Compile the project using `cmake --build build`
4. Your `tomlreduce` binary is available at `build/tomlreduce`

## Usage

tomlreduce accepts a single parameter, which is the TOML file you wish to reduce/rewrite. It will parse the full file into memory, and write it back out to the same filepath.

`tomlreduce my-file.toml`
