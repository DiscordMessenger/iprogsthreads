# iProg's Win32 Thread Implementation

This is the repository for iProgramInCpp's Win32 thread implementation.  It aims to loosely follow
the STL in regards to interfacing with `thread`, `mutex` and `condition_variable` objects. It is
designed to be compatible with most Windows versions. (It works for sure on NT4 and older)

## Background

MinGW 6.3.0 is the toolchain that [Discord Messenger](https://github.com/DiscordMessenger/dm) uses
to target versions of Windows older than XP SP3.  It does not come with a `thread` and `mutex`
implementation unless I give it `gthreads`, whatever that is.  I thought of using it, but it might
be the case that `gthreads` ends up using an API that is not implemented in earlier versions of
Windows, so writing this was my best choice.

## License

This project is licensed under the MIT license. See the license file for details.
