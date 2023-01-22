# Two methods of downloading files through certutil:

certutil.exe -urlcache -split -f "url"
certutil.exe -verifyctl -f -split "url"


#Encode/Decode files:
certutil.exe -encode inputfilepath/calc.exe outputfilepath/Sly