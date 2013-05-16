#/bin/bash

FILES=(`ls ours/`);
OURS=(`sed 's///g' ours/* | sed -n 's/Total Profit: \([0-9][0-9]*$\)/\1/p'`);
MDF=(`sed 's///g' solutions_MDF/* | sed -n 's/Total Profit: \([0-9][0-9]*$\)/\1/p'`);
ORG=(`sed 's///g' solutions_ORG/* | sed -n 's/Total Profit: \([0-9][0-9]*$\)/\1/p'`);

for (( i=0; i<${#OURS[@]}; i++ ));
do
	A=${OURS[$i]};
	B=${MDF[$i]};
	C=${ORG[$i]};
	DIFFB=`echo \($B-$A\)*100/$B | bc`;
	DIFFC=`echo \($C-$A\)*100/$C | bc`;
	echo "${FILES[$i]}: $DIFFB	| $DIFFC	($C)";
done;
