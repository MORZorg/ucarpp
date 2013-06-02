% Riccardo Orizio etc..
% 1 Giugno 2013
% Stampa grafici per miglior comprensione andamento funzione obiettivo

clc;
clear all;
close all;

% Apro il file
nomefile = 'val1A.3.sbra';
file_id = fopen( nomefile, 'r' );
tipo = fscanf( file_id, '%s', 1 );
veicoli = fscanf( file_id, '%d', 1 );
read_format = strcat( '%d ( ', repmat( '%d ', 1, veicoli ), ' )' );
data = fscanf( file_id, read_format );
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
% Creo la legenda
legenda = char( 'Totale' );
for i = 1 : veicoli
    legenda = char( legenda, sprintf( 'Veicolo %d', i ) );
end

%f = figure( 'visible', 'off' );
f = figure( 1 );
set( f, 'name', sprintf( '%s %d', tipo, veicoli ) );

subplot( 3, 1, 1 );
hold all;
plot( 1 : length( profit ), profit );
plot( 1 : length( profit ), vehicleProfit );
title( 'Profitto' );

subplot( 3, 1, 2 );
hold all;
plot( 1 : length( cost ), cost );
plot( 1 : length( profit ), vehicleCost );
title( 'Costo' );

legend( legenda, 'Location', [ 0 0.002 0.999 0.05 ] , 'Orientation', 'horizontal' );

subplot( 3, 1, 3 );
hold all;
plot( 1 : length( demand ), demand );
plot( 1 : length( profit ), vehicleDemand );
title( 'Domanda' );

%legend( legenda, 'Location', 'SouthOutside', 'Orientation', 'horizontal' );

saveas( f, strcat( nomefile, '.jpg' ), 'jpg' );

fprintf( 'Fatto grafico di %s\n', nomefile );

