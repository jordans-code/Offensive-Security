# This can make folders and files with illegal characters (Files/folders that end in a space, or only have a space for a name, or have illegal characters in general)
# This can be useful for some TTPs like UAC bypass techniques and tricking windows into thinking it's in the real C:\Windows folder when it's in "C:\Windows \"
mkdir "\\?\C:\temp\trailing space \"
Remove-Item -LiteralPath "\\?\C:\temp\trailing space \"
