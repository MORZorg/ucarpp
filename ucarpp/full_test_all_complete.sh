#/bin/bash

# Tipi di algoritmi
TYPE=("VNS" "VND" "VNASD1" "VNASD2" "VNAASD1" "VNAASD2");

for i in ${TYPE[@]}
do
	echo -e "~~~ Risolvo con $i ~~~\r\n";
	./full_test_all.sh $i
done;
