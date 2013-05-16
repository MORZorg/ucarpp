#/bin/bash

FILES=(`ls ours/`);
OURS=(`sed 's/
MDF=(`sed 's/
ORG=(`sed 's/

for (( i=0; i<${#OURS[@]}; i++ ));
do
	A=${OURS[$i]};
	B=${MDF[$i]};
	C=${ORG[$i]};
	DIFFB=`echo \($B-$A\)*100/$B | bc`;
	DIFFC=`echo \($C-$A\)*100/$C | bc`;
	echo "${FILES[$i]}: $DIFFB	| $DIFFC	($C)";
done;