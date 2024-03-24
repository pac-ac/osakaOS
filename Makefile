#sudo apt-get install g++ binutils libc6-dev-i386 mtools
#sudo apt-get install qemu-system-x86_64(any qemu stuff ngl) grub-legacy grub-mkrescue grub2 xorriso


GPPPARAMS = -m32 -Iinclude -fno-use-cxa-atexit -nostdlib -fno-builtin -fno-rtti -fno-exceptions -fno-threadsafe-statics -fno-leading-underscore -Wno-write-strings
ASPARAMS = --32
LDPARAMS = -melf_i386

objects = obj/loader.o \
	  obj/gdt.o \
	  obj/memorymanagement.o \
	  obj/art.o \
	  obj/drivers/driver.o \
	  obj/hardwarecommunication/port.o \
	  obj/hardwarecommunication/interruptstubs.o \
	  obj/hardwarecommunication/interrupts.o \
	  obj/multitasking.o \
	  obj/drivers/amd_am79c973.o \
	  obj/hardwarecommunication/pci.o \
	  obj/drivers/keyboard.o \
	  obj/drivers/mouse.o \
	  obj/drivers/vga.o \
	  obj/drivers/ata.o \
	  obj/drivers/pit.o \
	  obj/drivers/speaker.o \
	  obj/gui/widget.o \
	  obj/gui/desktop.o \
	  obj/gui/window.o \
	  obj/gui/button.o \
	  obj/gui/sim.o \
	  obj/net/etherframe.o \
	  obj/net/arp.o \
	  obj/net/ipv4.o \
	  obj/net/icmp.o \
	  obj/filesys/ofs.o \
	  obj/cli.o \
	  obj/app.o \
	  obj/script.o \
	  obj/math.o \
	  obj/mode/piano.o \
	  obj/mode/snake.o \
	  obj/mode/file_edit.o \
	  obj/mode/space.o \
	  obj/mode/bootscreen.o \
	  obj/kernel.o

obj/%.o: src/%.cc
	mkdir -p $(@D)
	g++ $(GPPPARAMS) -o $@ -c $<

obj/%.o: src/%.s
	mkdir -p $(@D)
	as $(ASPARAMS) -o $@ $<

osakaOS.bin: linker.ld $(objects)
	ld $(LDPARAMS) -T $< -o $@ $(objects)

install: osakaOS.bin
	sudo cp $< /boot/osakaOS.bin

osakaOS.iso: osakaOS.bin
	mkdir iso
	mkdir iso/boot
	mkdir iso/boot/grub
	cp osakaOS.bin iso/boot/osakaOS.bin
	echo 'set timeout=0' >> iso/boot/grub/grub.cfg
	echo 'set default=0' >> iso/boot/grub/grub.cfg
	echo '' >> iso/boot/grub/grub.cfg
	echo 'menuentry "osakaOS" {' >> iso/boot/grub/grub.cfg
	echo '	multiboot /boot/osakaOS.bin' >> iso/boot/grub/grub.cfg
	echo '	boot' >> iso/boot/grub/grub.cfg
	echo '}' >> iso/boot/grub/grub.cfg
	grub-mkrescue --output=osakaOS.iso iso
	rm -rf iso
	
	qemu-img create -f qcow2 Image.img 1G
	dd if=osakaOS.iso of=Image.img	

#run: osakaOS.bin
run: osakaOS.iso
	qemu-system-x86_64 -enable-kvm \
		-boot menu=on \
		-drive id=disk,file=Image.img,format=raw,if=none \
		-device piix4-ide,id=piix4 -device ide-hd,drive=disk,bus=piix4.0 \
		-cpu 486 -smp 1 -m 8M \
		-vga virtio
		#-audiodev pa,id=pa0,server=unix:/tmp/pulse-socket \
		#-machine pcspk-audiodev=pa0
		
		#only use '-soundhw' option if your qemu is 
		#old and doesnt support '-audiodev'
		
		#if neither works then just remove everything 
		#after '-vga virtio' and start without sound


.PHONY: clean
clean:
	rm -rf obj osakaOS.bin osakaOS.iso

