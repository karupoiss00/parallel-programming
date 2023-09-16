@echo off
for /l %%x in (1, 1, 16) do (
   call bmb-blur.exe input.bmp output.bmp %1 %%x
)