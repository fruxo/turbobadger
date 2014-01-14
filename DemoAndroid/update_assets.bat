
mkdir assets\language
mkdir assets\font
mkdir assets\skin
mkdir assets\demo_skin
copy /Y "..\resources\language\*.txt" "assets\language\*"
copy /Y "..\resources\default_font\*" "assets\font\*"
copy /Y "..\resources\default_skin\*.png" "assets\skin\*"
copy /Y "..\resources\default_skin\*.txt" "assets\skin\*"
copy /Y "..\Demo\demo01\skin\*.png" "assets\demo_skin\*"
copy /Y "..\Demo\demo01\skin\*.txt" "assets\demo_skin\*"
