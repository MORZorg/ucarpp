#/bin/bash

# Percorsi utili
INSTANCE_PATH="../instances&Results/instances/";
GRAPH_PATH="../progressive_output/";
RESULTS_PATH="../instances&Results/";
OURS_PATH="../instances&Results/ours_";
SOLUTION_FILES=$RESULTS_PATH"results.MDF "$RESULTS_PATH"results.ORG";
EXE="./full_test.sh";
INSTANCES=(`ls $INSTANCE_PATH`);

# Funzioni statistiche (assumendo mescolanza di valori positivi e negativi)
function average { total=0; for i in ${VALUES[@]}; do ((total+=$i)); done; echo $((total/${#VALUES[@]})); }
function varianc { total=0; for i in ${VALUES[@]}; do ((total+=($i-average) * ($i-average))); done; echo $((total/${#VALUES[@]})); }
function maximum { max=-100; for i in ${VALUES[@]}; do [ $i -gt $max ] && max=$i; done; echo "$max"; }
function minimum { min=0100; for i in ${VALUES[@]}; do [ $i -lt $min ] && min=$i; done; echo "$min"; }
function bbeated { total=0; for i in ${VALUES[@]}; do [ $i -lt 0 ]  && ((total+=1)); done; echo "$total"; }

# Compilo il programma
make
mv ucarpp ucarpp_stable

START_TIME=$(date +%s%N);
# Ciclo sulle istanza da risolvere 
for i in ${INSTANCES[@]}
do
	# Lancio lo script su ogni istanza
	echo -e " ..:: Risolvo l'istanza $i ::..\r\n";
	$EXE $i $1
done;
END_TIME=$(date +%s%N);

# Sposto tutte le soluzioni create in una cartella con la data corrente
CURRENT_DATE=(`date '+%y-%m-%d_%H-%M-%S'`);
RESULTS_DIR=$RESULTS_PATH"results/$1_$CURRENT_DATE/";
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

# Statistiche
TEMPO=$((($END_TIME-$START_TIME)/1000000000));
TOT_ISTANZE=$((${#INSTANCES[@]} * 6));
TEMPO_MEDIO=$(($TEMPO/$TOT_ISTANZE));
echo "Tempo impiegato: $TEMPO secondi per $TOT_ISTANZE istanze (media $TEMPO_MEDIO)" >> $RESULTS_PATH"results.TOT";
for i in $SOLUTION_FILES
do
	VALUES=(`sed 's/^M//g' $i | sed -n 's/[a-zA-Z0-9.: 	]*=>	\(-*[0-9][0-9]*\)%	([0-9]*).*/\1/p'`);
	echo $i >> $RESULTS_PATH"results.TOT";
	echo Media:		$(average) >> $RESULTS_PATH"results.TOT";
	echo Varianza:	$(varianc) >> $RESULTS_PATH"results.TOT";
	echo Massimo:	$(maximum) >> $RESULTS_PATH"results.TOT";
	echo Minimo:	$(minimum) >> $RESULTS_PATH"results.TOT";
	echo Battuti:	$(bbeated) >> $RESULTS_PATH"results.TOT";
done;
mv $RESULTS_PATH"results.TOT" $RESULTS_DIR;

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
