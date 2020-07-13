# Empires Dawn of the Modern World - ZoomChanger

This tool changes the zoom distance for the games "Empires Dawn of the Modern World" by dynamically binary patching the executable.
The tool is built with C++ and Qt5.

## Infos

Since the rendering process seems to dynamically determine the zoom value based on various variables, we must redirect the program flow within the zoom distance computation code path. Therefore, the binary must be patched to include a new PE segment for the scaling code section. 

## How to Use
Steps to apply the zoom patch:
1. Load the binary. ("open File" button)
2. Patch the binary. ("patch loaded File" button)
3. Change values for zoom / cull.
4. Write changes on file to disk. ("Write values" button)

How to revert the changes:
1. Load the binary.
2. Revert the patch. ("unpatch loaded File" button)
3. Write changes on file to disk.
Now, the original binary should be restored. The sha256 hash should be `9a713d5034d0cffedadd2d50266f15769f905e298bdc4f87cb90ddf6caaf4ae3`. 


## Recommended Values
For 1920x1080 resolution, I recommend the following values:
- Zoom: 1.5
- Cull: 260.29
