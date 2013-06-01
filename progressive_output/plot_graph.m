% Riccardo Orizio etc..
% 1 Giugno 2013
% Stampa grafici per miglior comprensione andamento funzione obiettivo

clc;
clear all;
close all;

% Apro il file
file_id = fopen( 'val1A.dat.sbra', 'r' );
tipo = fscanf( file_id, '%s', 1 );
data = fscanf( file_id, '%d' );
fclose( file_id );

% Ricavo le informazioni che mi interessano
profit = data( 1 : 3 : end );
cost = data( 2 : 3 : end );
demand = data( 3 : 3 : end );

% Grafici
figure( 1 );
subplot( 3, 1, 1 );
plot( [ 1 : length( profit ) ], profit );
title( 'Profitto' );

subplot( 3, 1, 2 );
plot( [ 1 : length( cost ) ], cost );
title( 'Costo' );

subplot( 3, 1, 3 );
plot( [ 1 : length( demand ) ], demand );
title( 'Domanda' );

