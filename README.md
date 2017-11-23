# emudore, a Commodore 64 emulator

![basic](/pics/basic.gif "basic")

# What's this?

emudore is a [Commodore 64 emulator](https://en.wikipedia.org/wiki/Commodore_64), 
fully written from the scratch and built on the top of the following technologies:

 * [C++11](https://en.wikipedia.org/wiki/C%2B%2B11)
 * [SDL2](https://www.libsdl.org/download-2.0.php)

emudore builds and runs on Linux, Windows (Visual Studio), OSX and even runs in
your browser thanks to Emscriptem, it should theoretically work on any platform 
supported by SDL2 provided a C++11 compiler is available.

# JavaScript and WebAssembly builds

If you wish to play a bit around without having to go through the hassle of 
compiling it you can try the web build:

* [emudore (boot into CBM BASIC)](http://marioballano.github.io/emudorejs/)
* [emudore (load Monopole PRG)](http://marioballano.github.io/emudorejs/?load=samples/monopole.prg)

A [WebAssembly](https://en.wikipedia.org/wiki/WebAssembly) build of emudore is also available, 
please note that a browser with WebAssembly support is required to run these demos:

* [emudore (boot into CBM BASIC)](http://marioballano.github.io/emudorewa/)
* [emudore (load Monopole PRG)](http://marioballano.github.io/emudorewa/?load=samples/monopole.prg)

# Why am I writing another C64 emulator?

Long story short: to learn a bit more about computer architecture, graphics, 
C++, etc..  while having some fun!

The Commodore 64 is regarded as one of the most epic 8-bit computers, it was actually 
the first computer I ever laid hands on - thanks to my dad :) - and it seemed like a 
sound choice to write an emulator for.

# Does this even work?

Well, yeah, mostly... 

The BASIC ROM runs just fine, and most simple programs run without issues, however, many 
of the more advanced games written in ML do not yet play well due to unimplemented hardware 
features, writing an emulator is a tough task and after all my goal wasn't to write a full 
perfect emulator but to learn in the process of making a simple one, here follow some facts 
you might be interested in knowing about emudore's implementation:

**emulation is instruction-exact**

That's right, on every emulation cycle a single instruction is fetched and interpreted, 
the number of CPU cycles the instruction has taken to execute is used to synchronize the 
rest of the chips in a C64 board. 

Unfortunately, this is not the best approach, other emulators are able to execute a single 
instruction over multiple iterations of their emulation loop and mimic more accurately 
the behaviour of the real hardware, however, this is an easier approach to implement 
and does not impede the emulation from being relatively accurate.

**illegal opcodes not supported yet**

As many other architectures the MOS 6510 features a number of undocumented opcodes,
most of these are thought to be unintended and usually perform a mix of other opcodes 
operations:

[NMOS 6510 Unintended Opcodes - PDF](http://csdb.dk/getinternalfile.php/135165/NoMoreSecrets-NMOS6510UnintendedOpcodes-20142412.pdf)

Nevertheless, some of these unintended opcodes have proven to be useful and are often
used in games and demos, emudore will need to support these in the future if we ever 
intend to emulate serious games and demos.

**single-threaded**

emudore has been written as a single-threaded program, everything (including graphics) is 
handled within the same thread, again, this approach has possibly some drawbacks, especially 
in terms of performance, but it greatly simplifies the architecture: things like 
synchronization of the mainboard chips become easier to implement.

**hardware acceleration and vertical refresh sync**

The screen is refreshed once at the end of every frame, when the video raster reaches the 
last visible scanline, this way we're not constantly writing to the host video memory.

Also, to speed things up a bit I implemented hardware acceleration, we use an accelerated 
renderer and streaming textures, unfortunately, we need to keep the rendered video frame 
within emudore's memory and upload the texture to the GPU on every frame, direct pixel-access
is not doable straight on the GPU memory.

I also implemented vertical refresh synchronization, at the end of every frame we check 
whether we are ahead of time compared to a real C64 computer, if that's the case we sleep 
for a bit and wake up at the point a real C64 would have finished rendering the frame, this
effectively locks the screen refresh down to ~50Hz (PAL).

There are two main benefits of implementing vsync: it helps with performance since GPU 
operations are costly and after all we don't want to run at the limit of fps our GPU can 
handle; also, and more importantly, by doing this we emulate the speed of the real C64 
computer synce the CPU and other chips are synchronized and run within the same thread, if 
we are on a fast computer visual effects won't look accelerated and games become playable :) 

**VIC-II chip**

The vic-ii is a relatively complex chip, my implementation is not yet complete whatsoever,
and certain features are more than likely still buggy, for now four out of the five official 
graphic modes are supported:

 * standard character mode 
 * multicolor character mode 
 * standard bitmap mode 
 * multicolor bitmap mode

Smooth scrolling, sprites and raster interrupts have also been implemented and badlines
are also emulated.

Some things that are left to implement include: sprite double height/width mode, sprite 
collision interrupts, etc.

A simple approach was taken to emulate the raster beam drawing, pixels drawn to the 
screen surface are computed at the end of each scan line, this might result in 
certain graphic effects being badly emulated, bear in mind that timing is of the essence 
in the c64, well-versed programmers master and exploit it to put together amazing
effects that otherwise wouldn't be feasible.

**beware**

Due to some of the aforementioned facts, expect things to fail, don't even dream
the emulation is going to be pixel-exact, certain effects are likely to get badly 
emulated, specially if you're running things like a demo.

# Are you bleedin' serious radare2 is supported?

Indeed, for now it's only suppported on Linux and OSX (debug) builds, you can grab 
a fresh copy of radare from [github](https://github.com/radare/radare2)

Then fire the emulator up and connect with radare:

    r2 -w -a 6502 rap://localhost:9999//

For now radare can just read and modify memory, further support might be coming
down the line once [this feature](https://github.com/radare/radare2/issues/4173) 
gets implemented in radare.

Some pictures of radare in action:

![r2mem](/pics/r2_mem.png "r2mem") 
![r2dis](/pics/r2_dis.png "r2dis")

I hope this feature will come in useful to retrodev reverse engineers :)

# So now .. what?

There are many features of the currently emulated chips that I haven't implemented 
yet, time permitting my plan is to keep working and learning about other aspects of
the C64 technology, also, at some stage I'd love to get the time to work on:

  * Sound support: [SID](https://en.wikipedia.org/wiki/MOS_Technology_SID)
  * Floppy drive support: [VIC-1541](https://en.wikipedia.org/wiki/Commodore_1541)

# Alright, enough of your jibba-jabba, how do I run this?

If you are running a Linux Debian-based distro:

    sudo apt-get install g++ cmake libsdl2-dev

Then simply compile and run:

    make release
    cd build
    ./emudore

# What C64 programs can I load into this?

For the time being emudore can load PRGs, you can test: 

    ./emudore assets/prg/monopole.prg

emudore can also type BASIC listings for you (special keys not supported yet):

    ./emudore assets/bas/10print.bas 
    (then type RUN at the emulator window)

# Got some more screenshots?

Sure.. 

![parallax](/pics/parallax.png "parallax") ![mario](/pics/mario.png "mario") ![pacman](/pics/pacman.png "pacman") 
![hitmen](/pics/hitmen.png "hitmen") ![montezuma](/pics/montezuma.png "montezuma") ![ghostbusters](/pics/ghostbusters.png "ghostbusters") 


# References

If you are interested in computer archeology, and particularly in the C64, among
others, the following resources came in handy to develop (and get inspiration) while 
developing emudore:

 * [The C64 Wiki](http://www.c64-wiki.com/)
 * [Programming the Commodore 64: The Definitive Guide](http://www.amazon.co.uk/Programming-Commodore-64-Definitive-Guide/dp/0874550815/)
 * [Commodore: A Company on the Edge (book)](http://www.amazon.co.uk/Commodore-Company-Edge-Brian-Bagnall/dp/0973864966/)
 * [The Ultimate Commodore 64 Talk](https://www.youtube.com/watch?v=ZsRRCnque2E)
 * [Behind the scenes of a C64 demo](https://www.youtube.com/watch?v=So-m4NUzKLw)
 * [Dustlayer's blog](http://dustlayer.com/blog/)
 * [The MOS 6567/6569 video controller (VIC-II) and its application in the Commodore 64](http://www.zimmers.net/cbmpics/cbm/c64/vic-ii.txt)

# Forks 

Some people have forked emudore and experimented with it, some notable forks include:

* [Chris Frantz's C64 and NES emulator](https://github.com/cfrantz/emudore)
* [Scott Hutter's bare metal x86-64 emulator](https://github.com/xlar54/emudore64)

# License

I don't think anybody will ever dare to use this for an actual useful purpose, but just in case, 
the project is licensed under the [Apache 2.0 license](http://www.apache.org/licenses/LICENSE-2.0)
