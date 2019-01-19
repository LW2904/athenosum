### athen`osu!`m

_The [Athenaeum](https://en.wikipedia.org/wiki/Athenaeum) for osu!._

A collection of code that I wrote while working on a number of osu! related projects.

I wouldn't dare call any of this stable, but the codebase is reasonably clean and consistent. Currently, most of the contents were migrated over here from [maniac](https://github.com/LW2904/maniac), with some cleanup for code style and minor bugs (and to make [clang-tidy](http://clang.llvm.org/extra/clang-tidy/) happy).

All of the code can be compiled on Linux, where it depends on X11, and the more recent Windows versions.

#### Installation

```
mkdir build && cd build
cmake ..
sudo make install
```

You will find the (shared) library and all public headers in their respective default directories. As should be obvious by the instructions above, [CMake](https://cmake.org/) is the only supported way of building this library.

#### Current State

__This is a work in progress.__ The code here should be considered pre-alpha, and this should only be used by tinkerers or hackers at this point.

Anything requiring `Xtest` is currently __broken__, this includes keypress simulation.

#### Usage

You will find that all functions declarations are following a comment block, describing their use and usage.

Many functions require OS-specific setup to be performed before they can be run, which is currently done with `do_setup()`, from the `game.h` header. Said functions have a note declaring this dependency in their usage comment.

#### Features

_This is not a comprehensive list and should simply serve as an overview._

`beatmap.h`

- Beatmap search in the default osu! directory given a partial name (for example from the osu! window title)

Currently focused on osu!mania beatmaps:

- Parsing of metadata and hitpoints from osu!mania beatmaps
- Parsing of hitpoints into actions, each representing a keypress
- Basic humanization of actions (read: adding a randomised delta to the timings of each)

`game.h`

- Keypress simulation

`process.h`

- Pattern scanning to fetch the address of the game time in an osu! process
- Game time reading from an osu! process

`window.h`

- Window searching
- Window title fetching

#### TODO

These are features which are planned, the code contains a number of TODO comment blocks which detail things that should be improved in the current implementations.

- Unit tests
- Better osu!standard integration
- Replay parsing (migration from [mimic](https://github.com/LW2904/mimic))
