
mkdir assets\language
mkdir assets\font
mkdir assets\skin
mkdir assets\demo_skin
copy /Y "..\resources\language\*.txt" "assets\language\*"
copy /Y "..\tinkerbell\default_font\*" "assets\font\*"
copy /Y "..\tinkerbell\default_skin\*.png" "assets\skin\*"
copy /Y "..\tinkerbell\default_skin\*.txt" "assets\skin\*"
copy /Y "..\Demo\demo01\skin\*.png" "assets\demo_skin\*"
copy /Y "..\Demo\demo01\skin\*.txt" "assets\demo_skin\*"
