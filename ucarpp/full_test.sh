#/bin/bash

# Percorsi utili 
INSTANCE_PATH="../instances&Results/instances/";
OURS_PATH="../instances&Results/ours_";
SOLUTION_PATH="../instances&Results/solutions_";
GRAPH_PATH="../progressive_output/";
SOLUTION_FILE="../instances&Results/results.";
EXE="./ucarpp_stable";

# Eseguo il programma per risolvere il problema associato al file ricevuto come parametro
TYPE[0]="ORG";
TYPE[1]="MDF";
VEHICLE[0]=2;
VEHICLE[1]=3;
VEHICLE[2]=4;

# Ciclo sul tipo di istanze da risolvere
for i in ${TYPE[@]}
do
	# Ciclo sul numero di veicoli possibili
	for j in ${VEHICLE[@]} 
	do
		# Eseguo il programma con i parametri correnti
		echo "Risolvo $1 $i con $j veicoli.";
		FILENAME="Detailed_Sol_$1_$j.txt";
		# Prendo il tempo di esecuzione per risolvere un'istanza
		START_TIME=$(date +%s%N);
		while ! $EXE $INSTANCE_PATH$1 $j $2 $i > $OURS_PATH$i"/"$FILENAME;
		do
			echo "Test fallito. Riprovo.";

			# Resetto l'istante iniziale
			START_TIME=$(date +%s%N);
		done
		END_TIME=$(date +%s%N);
		TEMPO=$((($END_TIME-$START_TIME)/1000000000));

		# Creo il grafico associato alla soluzione
		echo "Creo i grafici del corrente problema.";
		matlab -nodisplay -nosplash -r "cd $GRAPH_PATH; plot_graph('${1:0:(-4)}.$j.$2.$i.morz'); exit" > /dev/null;

		# Confronto le soluzioni con quelle di benchmark
		echo -e "Confronto la soluzione con il benchmark.\r\n";
		# Apro il file con le nostre soluzioni
		OUR=(`sed 's///g' $OURS_PATH$i"/"$FILENAME | sed -n 's/Total Profit: \([0-9][0-9]*$\)/\1/p'`);
		# Apro il file con le soluzioni di benchmark
		SOL=(`sed 's///g' $SOLUTION_PATH$i"/"$FILENAME | sed -n 's/Total Profit: \([0-9][0-9]*$\)/\1/p'`);
		DIFF=`echo \($SOL-$OUR\)*100/$SOL | bc`;
		echo "$1 $j veicoli $i:	$OUR =>	$DIFF%	($SOL) in $TEMPO s" >> $SOLUTION_FILE$i;

	done;
done;

