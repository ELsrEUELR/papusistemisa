#!/bin/bash
set -e

EISVOGEL_TEMPLATE="Eisvogel-3.2.0/eisvogel.latex"
BACKGROUND_PAGE="Eisvogel-3.2.0/examples/title-page-background/backgrounds/background5.pdf"
BACKGROUND_TITLE="Eisvogel-3.2.0/examples/title-page-custom/background.pdf"
LOGO="../FIE-Logo-Oro.png"

TITLE_COLOR="FFFFFF"
RULE_COLOR="B8860B"
MAIN_FILE="monitor_de_procesos_estilo_top.md"
INPUT_FILE="$MAIN_FILE"
OUTPUT_FILE="${INPUT_FILE%.*}.pdf"

echo "Generando PDF con Eisvogel..."
echo "Archivo de entrada: $INPUT_FILE"
echo "Archivo de salida: $OUTPUT_FILE"

if [ ! -f "$INPUT_FILE" ]; then
    echo "Error: No se encuentra el archivo $INPUT_FILE"
    exit 1
fi

pandoc "$INPUT_FILE" \
    --from markdown+hard_line_breaks \
    --to pdf \
    --template="$EISVOGEL_TEMPLATE" \
    --pdf-engine=xelatex \
    --metadata lang=es \
    --metadata papersize=letter \
    --metadata geometry="top=2cm, bottom=2.5cm, left=2cm, right=2cm" \
    --metadata fontsize=11pt \
    --metadata colorlinks=true \
    --metadata linkcolor=NavyBlue \
    --metadata urlcolor=NavyBlue \
    --metadata citecolor=NavyBlue \
    --metadata titlepage=true \
    --metadata titlepage-text-color="$TITLE_COLOR" \
    --metadata titlepage-rule-color="$RULE_COLOR" \
    --metadata titlepage-background="$BACKGROUND_TITLE" \
    --metadata titlepage-logo="$LOGO" \
    --metadata logo-width="70mm" \
    --metadata page-background="$BACKGROUND_PAGE" \
    --metadata table-use-row-colors=true \
    --metadata tables=true \
    --highlight-style=tango \
    --preserve-tabs \
    --filter "$(pwd)/../venv/bin/pandoc-latex-environment" \
    --output "$OUTPUT_FILE"

if [ $? -eq 0 ]; then
    echo "PDF generado exitosamente: $OUTPUT_FILE"
    echo "Puedes abrirlo con: xdg-open $OUTPUT_FILE"
else
    echo "Error al generar el PDF"
    exit 1
fi
