> ⚠️ Running this will destroy your computer
>
> Star my repo!

This works by writing new data to the first 512-byte sector of a hard drive which replaces the boot code and partition table with new info.

This renders a pc unbootable because MBR is essential for locating the bootloader. Tou can use fixmbr to fix this.
