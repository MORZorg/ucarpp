% Riccardo Orizio etc..
% 1 Giugno 2013
% Stampa grafici per miglior comprensione andamento funzione obiettivo

clc;
clear all;
close all;

% Apro il file
nomefile = 'val1A.4.sbra';
file_id = fopen( nomefile, 'r' );
% Leggo la prima riga: tipo risolutore e numero di veicoli nella soluzione
tipo = fscanf( file_id, '%s', 1 );
veicoli = fscanf( file_id, '%d', 1 );
% A seconda del numero di veicoli leggerò più o meno dati
read_format = strcat( '%d ( ', repmat( '%d ', 1, veicoli ), ' )' );
data = fscanf( file_id, read_format );
% Chiudo il file
fclose( file_id );

% Ricavo le informazioni che mi interessano
step = 1 + veicoli;
selector =  1 : 3 * step : length( data );
% Suddivido i dati in base a profitto, costo e domanda
profit = data( selector );
cost = data( selector + step );
demand = data( selector + 2 * step );
% Ricavo le informazioni per ogni veicolo
vehicleSelector = 1 + ones( veicoli, 1 ) * selector + ( 0 : veicoli - 1 )' * ones( 1, length( selector ) );
vehicleProfit = data( vehicleSelector ).';
vehicleCost = data( vehicleSelector + step ).';
vehicleDemand = data( vehicleSelector + 2 * step ).';

% Grafici
% Creo la legenda
legenda = char( 'Totale' );
for i = 1 : veicoli
    legenda = char( legenda, sprintf( 'Veicolo %d', i ) );
end

%f = figure( 'visible', 'off' );
f = figure( 1 );
set( f, 'name', sprintf( '%s %d', tipo, veicoli ) );

% Eseguo un ciclo per poter stampare sia la soluzione ottima, sia la
% corrente.
x_axe = 1 : length( profit ) / 2;
titles = char( char( 'Shaked' ), char( 'Optimal' ) );

for i = 1 : 2
    % Stampo i profitti
    subplot( 3, 2, 1 + ( i - 1 ) );
    hold all;
    plot( x_axe, profit( i : 2 : end ) );
    plot( x_axe, vehicleProfit( i : 2 : end, : ) );
    title( sprintf( 'Profitto %s', titles( i, : ) ) );

    % Stampo i costi
    subplot( 3, 2, 3 + ( i - 1 ) );
    hold all;
    plot( x_axe, cost( i : 2 : end ) );
    plot( x_axe, vehicleCost( i : 2 : end, : ) );
    title( sprintf( 'Costo %s', titles( i, : ) ) );

    % Stampo le domande
    subplot( 3, 2, 5 + ( i - 1 ) );
    hold all;
    plot( x_axe, demand( i : 2 : end ) );
    plot( x_axe, vehicleDemand( i : 2 : end, : ) );
    title( sprintf( 'Domanda %s', titles( i, : ) ) );
end

% Inserisco la legenda nel grafico
legend( legenda, 'Location', [ 0 0.002 0.999 0.05 ] , 'Orientation', 'horizontal' );

% Esporto il grafico in un file jpg
saveas( f, strcat( nomefile, '.jpg' ), 'jpg' );

% Informo l'utente di aver finito con la generazione del grafico
fprintf( 'Fatto grafico di %s\n', nomefile );
