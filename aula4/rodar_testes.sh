#!/bin/bash

# Compila o programa primeiro
gcc -O2 -Wall -pthread -o exercicio exercicio.c -lm

# Valores fixos do enunciado
valores_N="2 8 32"
combinacoes="1,1 1,2 1,4 1,8 1,16 2,1 4,1 8,1 16,1"
repeticoes=10

# Arquivo de saída
echo "N,Np,Nc,tempo_medio" > resultados.csv

for N in $valores_N; do
    for combo in $combinacoes; do
        Np=$(echo $combo | cut -d',' -f1)
        Nc=$(echo $combo | cut -d',' -f2)

        soma=0
        for rep in $(seq 1 $repeticoes); do
            tempo=$(./exercicio $N $Np $Nc | grep "Tempo:" | awk '{print $2}')
            soma=$(echo "$soma + $tempo" | bc)
            echo "  N=$N Np=$Np Nc=$Nc rep=$rep -> $tempo s"
        done

        media=$(echo "scale=6; $soma / $repeticoes" | bc)
        media=$(printf "%.6f" "$media")
        echo "$N,$Np,$Nc,$media" >> resultados.csv
        echo ">>> Média N=$N Np=$Np Nc=$Nc: $media s"
    done
done

echo "Concluído! Resultados em resultados.csv"