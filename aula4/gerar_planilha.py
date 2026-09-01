#!/usr/bin/env python3
"""
gerar_planilha.py

Le o arquivo resultados.csv (gerado pelo rodar_testes.sh) e monta uma
planilha .xlsx com:
  - aba "Dados": tabela N, Np, Nc, tempo_medio
  - aba "Tabela Pivot": tempo medio organizado por combinacao (Np,Nc) x N
  - grafico de linhas: tempo medio x combinacao de threads, uma curva por N

Uso:
    python3 gerar_planilha.py [caminho_para_resultados.csv] [saida.xlsx]

Se nao passar argumentos, usa "resultados.csv" como entrada e
"resultados.xlsx" como saida, na pasta atual.

Requisitos: pip install openpyxl --break-system-packages
"""

import csv
import sys
from openpyxl import Workbook
from openpyxl.chart import LineChart, Reference
from openpyxl.styles import Font, Alignment, PatternFill
from openpyxl.utils import get_column_letter


# ordem das combinacoes (Np, Nc) exatamente como no enunciado
COMBINACOES = [(1, 1), (1, 2), (1, 4), (1, 8), (1, 16),
               (2, 1), (4, 1), (8, 1), (16, 1)]

VALORES_N = [2, 8, 32]


def ler_csv(caminho):
    linhas = []
    with open(caminho, newline="") as f:
        leitor = csv.DictReader(f)
        for row in leitor:
            linhas.append({
                "N": int(row["N"]),
                "Np": int(row["Np"]),
                "Nc": int(row["Nc"]),
                "tempo_medio": float(row["tempo_medio"]),
            })
    return linhas


def montar_planilha(linhas, caminho_saida):
    wb = Workbook()

    # ---------- aba 1: dados brutos ----------
    ws_dados = wb.active
    ws_dados.title = "Dados"

    cabecalho = ["N", "Np", "Nc", "tempo_medio (s)"]
    ws_dados.append(cabecalho)
    for col in range(1, 5):
        cel = ws_dados.cell(row=1, column=col)
        cel.font = Font(bold=True, color="FFFFFF")
        cel.fill = PatternFill("solid", fgColor="4472C4")
        cel.alignment = Alignment(horizontal="center")

    for linha in linhas:
        ws_dados.append([linha["N"], linha["Np"], linha["Nc"], linha["tempo_medio"]])

    for col, largura in zip("ABCD", [8, 8, 8, 16]):
        ws_dados.column_dimensions[col].width = largura

    # ---------- aba 2: tabela pivot (Np,Nc) x N, pronta para grafico ----------
    ws_piv = wb.create_sheet("Tabela para Grafico")

    ws_piv.cell(row=1, column=1, value="Combinacao (Np,Nc)").font = Font(bold=True)
    for j, N in enumerate(VALORES_N, start=2):
        c = ws_piv.cell(row=1, column=j, value=f"N={N}")
        c.font = Font(bold=True, color="FFFFFF")
        c.fill = PatternFill("solid", fgColor="4472C4")
        c.alignment = Alignment(horizontal="center")

    ws_piv.cell(row=1, column=1).fill = PatternFill("solid", fgColor="4472C4")
    ws_piv.cell(row=1, column=1).font = Font(bold=True, color="FFFFFF")

    for i, (np_, nc_) in enumerate(COMBINACOES, start=2):
        ws_piv.cell(row=i, column=1, value=f"({np_},{nc_})")
        for j, N in enumerate(VALORES_N, start=2):
            valor = next(
                (l["tempo_medio"] for l in linhas
                 if l["N"] == N and l["Np"] == np_ and l["Nc"] == nc_),
                None,
            )
            ws_piv.cell(row=i, column=j, value=valor)

    ws_piv.column_dimensions["A"].width = 20
    for col in "BCD":
        ws_piv.column_dimensions[col].width = 12

    # ---------- grafico de linhas ----------
    chart = LineChart()
    chart.title = "Tempo medio de execucao vs combinacao de threads (Np, Nc)"
    chart.x_axis.title = "Combinacao (Np, Nc)"
    chart.y_axis.title = "Tempo medio (s)"
    chart.style = 10
    chart.width = 24
    chart.height = 12

    n_linhas = len(COMBINACOES)
    dados = Reference(ws_piv, min_col=2, max_col=1 + len(VALORES_N),
                       min_row=1, max_row=1 + n_linhas)
    categorias = Reference(ws_piv, min_col=1, min_row=2, max_row=1 + n_linhas)

    chart.add_data(dados, titles_from_data=True)
    chart.set_categories(categorias)

    for serie in chart.series:
        serie.marker.symbol = "circle"
        serie.smooth = False

    ws_piv.add_chart(chart, f"{get_column_letter(2 + len(VALORES_N) + 1)}2")

    # ---------- aba 3: melhor combinacao e observacoes ----------
    ws_info = wb.create_sheet("Resumo")
    melhor = min(linhas, key=lambda l: l["tempo_medio"])

    ws_info["A1"] = "Resumo do experimento"
    ws_info["A1"].font = Font(bold=True, size=14)

    info = [
        ("Numero total de execucoes (combinacoes x repeticoes)", len(linhas) and 27 * 10),
        ("Numero de combinacoes distintas (N,Np,Nc)", len(linhas)),
        ("", ""),
        ("Melhor combinacao (menor tempo medio)", f"N={melhor['N']}, Np={melhor['Np']}, Nc={melhor['Nc']}"),
        ("Tempo medio da melhor combinacao (s)", melhor["tempo_medio"]),
    ]
    for i, (label, valor) in enumerate(info, start=3):
        ws_info.cell(row=i, column=1, value=label).font = Font(bold=True)
        ws_info.cell(row=i, column=2, value=valor)

    ws_info.column_dimensions["A"].width = 45
    ws_info.column_dimensions["B"].width = 20

    wb.save(caminho_saida)
    print(f"Planilha salva em: {caminho_saida}")
    print(f"Melhor combinacao: N={melhor['N']}, Np={melhor['Np']}, Nc={melhor['Nc']} "
          f"-> {melhor['tempo_medio']:.6f} s")


def main():
    entrada = sys.argv[1] if len(sys.argv) > 1 else "resultados.csv"
    saida = sys.argv[2] if len(sys.argv) > 2 else "resultados.xlsx"

    try:
        linhas = ler_csv(entrada)
    except FileNotFoundError:
        print(f"Erro: arquivo '{entrada}' nao encontrado.")
        sys.exit(1)

    if not linhas:
        print("Erro: CSV vazio ou sem dados validos.")
        sys.exit(1)

    montar_planilha(linhas, saida)


if __name__ == "__main__":
    main()