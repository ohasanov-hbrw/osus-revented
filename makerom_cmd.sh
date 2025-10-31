./bannertool makebanner -i meta/banner.png -a meta/banner.wav -o banner.bnr
./makerom -f cia -o osus-revented.cia -rsf meta/app.rsf -target t -elf osus-revented.elf -icon meta/icon.png -banner banner.bnr -v
