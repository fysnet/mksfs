# mksfs
make an SFS image file

Source code to create an image file, filled with specified files, formatted to the Simple File System (SFS) file system.

See top of mksfs.c for licensing

See floppy.txt and harddrive.txt for examples

Usage:
  mksfs floppy.txt
or
  mksfs harddisk.txt

See sfs.pdf for the file system's specification file.

Here is the post I made to http://forum.osdev.org/viewtopic.php?f=11&t=32405&p=279764#p279764

"The .EXE is compiled with DJGPP for a DPMI environment, however I hope I have made the source code mostly portable so it should compile for other environments as-is.  However, please let me know if it doesn't and please show me the additions/modifications you made to get it to compile on your platform.

I ask that someone with Linux and other unix platform, compile it and see what modifications need to be made.

Okay, now for some notes:
The two example resource files, floppy.txt and harddisk.txt, create a 1.44meg floppy image and a 5Meg hard drive image (respectively).  To make either one, use:

   mksfs floppy.txt
or
   mksfs harddisk.txt

Within the .txt file are a few parameters you need to initialize:

  imgfile=  D:\fysos\main\filesys\images\sfs_f.img

This is the target path and filename of the image file to create.  It will truncate any existing file, so only create new images or know that any existing will be destroyed.

  mbrfile=D:\Books\fysos\main\syscore\source\mbr\mbr.bin

This is the path and filename of the MBR boot sector code to put at the first of the image.  Your code to create mbr.bin should parse the partition entries, load the active one, then jump to 0x07C00. (Note that this parameter should not be used for floppy images, though if given, will still work just fine)

  bootfile= D:\Path\to\your\boot\source\code\sfs_f.bin

This is the path and filename of the boot sector code to put at the first of the volume.  Note that it should be a multiple of 512 (BLOCK SIZE) and can be one (1) or more blocks in size.

base_lba= 0         # LBA = 0
tot_sects= 2880     # Total number of sectors in partition
param0= 0           # Ignored
param1= 0           # Ignored
heads= 2            # heads per cylinder
spt= 18             # sectors per track
These need to be initialized to the specs shown above.  The tot_sects field is the total count of 512-byte sectors used in the image, not the volume size.  For example, if you are creating a hard disk image and give 123,456 as the tot_sects parameter and then 63 as the base_lba parameter, the volume size will be 123,456 - 63 sectors.  (Note that the code may add sectors to pad to the nearest ending cylinder or track.)

 D:\Path\goes\here\kernel.sys, kernel.sys, 0
 * , system, 1            # make the system folder (note, must put something in path field. '*' works just fine)
 
The end of the file is a list of paths and files to add to the image.  The first parameter (before the first comma) is the path and filename of the file on the host machine to copy from.

The second parameter is the path and filename of the file to place onto the image.  Note that it will write the path and file name exactly as you have it in this parameter.  There is no error checking to see if the path and filename is valid.

The last parameter is a 0 or a 1.  If 0, it is a file, write to file's contents to the image as well as adding the entry.  If the parameter is a 1, it is a folder, only add the entry to the Index Data Area.  (Note that there must be all three parameters, even though the folder doesn't need a first parameter, and each parameter must have something in it, hence the '*' character.

The code is not written to check errors and assumes that all input is valid.  I did not test it as much as I probably should have, so please let me know if you find any errors.

I have also included two pre-made images, so that you can test your code without having to make mksfs.  Each image contains minimal files for http://www.fysnet.net/fysos.htm.  Each image will boot with valid SFS boot code, though note that the GUI will not load since I didn't include a complete SFS driver for FYSOS yet.

I am not included source code to boot SFS since this is suppose to be your job :-).

Thank you,
Ben
