% Riccardo Orizio etc..
% 1 Giugno 2013
% Stampa grafici per miglior comprensione andamento funzione obiettivo

clc;
clear all;
close all;

% Apro il file
file_id = fopen( 'val1A.dat.sbra', 'r' );
tipo = fscanf( file_id, '%s', 1 );
veicoli = fscanf( file_id, '%d', 1 );
data = fscanf( file_id, '%d ( %d %d %d )' );
fclose( file_id );

% Ricavo le informazioni che mi interessano
step = 1 + veicoli;
selector =  1 : 3 * step : length( data );
profit = data( selector );
cost = data( selector + step );
demand = data( selector + 2 * step );

vehicleSelector = 1 + ones( veicoli, 1 ) * selector + ( 0 : veicoli - 1 )' * ones( 1, length( selector ) );
vehicleProfit = data( vehicleSelector );
vehicleCost = data( vehicleSelector + step );
vehicleDemand = data( vehicleSelector + 2 * step );

% Grafici
f = figure( 1 );
set( f, 'name', sprintf( '%s %d', tipo, veicoli ) );

subplot( 3, 1, 1 );
hold all;
plot( 1 : length( profit ), vehicleProfit );
plot( 1 : length( profit ), profit );
title( 'Profitto' );

subplot( 3, 1, 2 );
hold all;
plot( 1 : length( profit ), vehicleCost );
plot( 1 : length( cost ), cost );
title( 'Costo' );

subplot( 3, 1, 3 );
hold all;
plot( 1 : length( profit ), vehicleDemand );
plot( 1 : length( demand ), demand );
title( 'Domanda' );

