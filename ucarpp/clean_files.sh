#/bin/bash

# Percorsi utili
INSTANCE_PATH="../instances&Results/instances/";
GRAPH_PATH="../progressive_output/";
RESULTS_PATH="../instances&Results/";
OURS_PATH="../instances&Results/ours_";
SOLUTION_FILES=$RESULTS_PATH"results.MDF "$RESULTS_PATH"results.ORG "$RESULTS_PATH"results.TOT";
EXE="./full_test.sh";
INSTANCES=(`ls $INSTANCE_PATH`);

# Ripulisco la cartella dei grafici
rm $GRAPH_PATH

# Cancello i confronti con i benchmark
rm $SOLUTION_FILES

# Cancello i risultati
TYPE[0]="ORG";
TYPE[1]="MDF";

for i in ${TYPE[@]}
do
	rm $OURS_PATH$i
done;
