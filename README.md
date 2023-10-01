# osakaOS v1.0
<h2>THE operating system of all time.</h2>

osakaOS is a 32 bit, singletasking, megalithic operating system that is made with a focus on unconventional/ridicoulous design choices. It uses a text-based command line to edit files using the <b>Osaka FileSystem</b> and create small programs using the <b>AyumuScript</b> scripting language. 

It also features small programs called 'modes' for small games and entertainment. The commands include the ability to read and write directly to memory, disk sectors, and most importantly, print ASCII art of Osaka :)


<h2>How to run/compile</h2>

Included is a pre-compiled bootable iso image that you can use in any normal virtualization software.
To compile from source do 'sudo make run' and pray for the best. The binary will be built and QEMU will boot from the virtual disk.

You will probably need the following software packages: g++, binutils, libc6-dev-i386, qemu-system-x86_64 grub-legacy, grub2, xorriso.

<h2>Extra</h2>

This project is partly based on the <a href="https://github.com/AlgorithMan-de/wyoos">WYOOS</a> tutorial series. Although many things from the tutorial have been changed or removed for various reasons.

There are quite a number of glitches that I've yet to fix. 
<ul>
  <li>Deleting a file that was not the most recently created might cause files to be listed incorrectly.</li>
  <li>Every 4th LBA won't save data correctly on it's last sector.</li>
  <li>Recent versions of QEMU have deprecated the 'pcspk' option. It might take some work to get speaker emulation working.</li>
  <li>Other stuff that I haven't found yet or are too insignificant to mention in the README.</li>
</ul>

Yes there are graphics. Currently they're in a very sorry state but I do have plans to develop them further. There are also implementations for things like multitasking and networking but they're also underutilized currently.
