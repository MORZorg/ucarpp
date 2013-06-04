#/bin/bash

# Percorsi utili
INSTANCE_PATH="../instances&Results/instances/";
GRAPH_PATH="../progressive_output/";
RESULTS_PATH="../instances&Results/";
OURS_PATH="../instances&Results/ours_";
SOLUTION_FILES=$RESULTS_PATH"results.MDF "$RESULTS_PATH"results.ORG";
EXE="./full_test.sh";
INSTANCES=(`ls $INSTANCE_PATH`);

# Compilo il programma
make
mv ucarpp ucarpp_stable

# Ciclo sulle istanza da risolvere 
for i in ${INSTANCES[@]}
do
	# Lancio lo script su ogni istanza
	echo -e " ..:: Risolvo l'istanza $i ::..\r\n";
	$EXE $i $1
done;

# Sposto tutte le soluzioni create in una cartella con la data corrente
CURRENT_DATE=(`date '+%y-%m-%d_%H-%M-%S'`);
RESULTS_DIR=$RESULTS_PATH"results/"$CURRENT_DATE"/";
mkdir $RESULTS_DIR;

# Copio i grafici generati
GRAPHS_DIR="Graphs";
mkdir $RESULTS_DIR$GRAPHS_DIR;
for i in `ls $GRAPH_PATH`
do
	if [ ${i:(-1)} != "m" ]
	then
		mv $GRAPH_PATH$i $RESULTS_DIR$GRAPHS_DIR
	fi
done;

# Copio i confronti con i benchmark
mv $SOLUTION_FILES $RESULTS_DIR
# Copio i risultati
TYPE[0]="ORG";
TYPE[1]="MDF";
for i in ${TYPE[@]}
do
	mkdir $RESULTS_DIR$i
	for j in `ls $OURS_PATH$i`
	do
		mv $OURS_PATH$i"/"$j $RESULTS_DIR$i"/"
	done;
done;
