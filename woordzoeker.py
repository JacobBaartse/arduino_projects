from fpdf import FPDF

import random

# Instellingen

grid_size = 11

solution_phrase = "NOSTALGIE MET EEN OPSTEKER".replace(" ", "").upper()

words = [
    "PIN", "SPELD", "STEEK", "SCHIL", "STOK", "WANDEL", "HOED", "PROMOTIE",
    "DORP", "MERK", "PELGRIM", "STAF", "ALP", "BEWAREN", "PRIK", "VERZAMEL"
]

# 1. Leeg rooster maken

grid = [["" for _ in range(grid_size)] for _ in range(grid_size)]

# 2. Willekeurige posities vullen met oplossingletters (de overgebleven letters)

empty_positions = [(r, c) for r in range(grid_size) for c in range(grid_size)]
random.shuffle(empty_positions)
solution_positions = empty_positions[:len(solution_phrase)]
solution_positions.sort()  # zodat ze in leesvolgorde staan (rij voor rij)

for letter, (r, c) in zip(solution_phrase, solution_positions):
    grid[r][c] = letter

# 3. Woorden toevoegen
directions = [(0, 1), (1, 0), (1, 1), (-1, 1), (0, -1), (-1, 0), (-1, -1), (1, -1)]

def can_place(word, row, col, dr, dc):
    for il in range(len(word)):

        r, c = row + dr * il, col + dc * il

        if not (0 <= r < grid_size and 0 <= c < grid_size):
            return False

        if grid[r][c] not in ("", word[il]):
            return False

    return True


def place_word(word):
    attempts = 100
    while attempts > 0:
        dr, dc = random.choice(directions)
        row = random.randint(0, grid_size - 1)
        col = random.randint(0, grid_size - 1)
        if can_place(word, row, col, dr, dc):
            for im in range(len(word)):
                r, c = row + dr * im, col + dc * im
                grid[r][c] = word[im]
            return True
        attempts -= 1
    return False

# Woorden plaatsen

for word in words:
    place_word(word.upper())

# Rest opvullen met random letters

for r in range(grid_size):
    for c in range(grid_size):
        if grid[r][c] == "":
            grid[r][c] = random.choice("ABCDEFGHIJKLMNOPQRSTUVWXYZ")

# 4. PDF genereren

pdf = FPDF()
pdf.add_page()
pdf.set_font("Arial", size=12)

line = "Woordzoeker – Thema: Souvenirs"
pdf.cell(0, 10, txt=line.encode('utf-8').decode('latin-1'), ln=True)

line = "Zoek de woorden en ontdek de zin in de overgebleven letters!"
pdf.cell(0, 10, txt=line.encode('utf-8').decode('latin-1'), ln=True)
pdf.ln(5)

# Woordenlijst

pdf.set_font("Arial", "B", 11)

pdf.cell(0, 10, "Woorden:", ln=True)
pdf.set_font("Arial", size=11)

for i in range(0, len(words), 4):
    line = "   ".join(words[i:i + 4])
    pdf.cell(0, 8, txt=line.encode('utf-8').decode('latin-1'), ln=True)

pdf.ln(5)
pdf.set_font("Courier", size=14)

for row in grid:
    line = " ".join(row)
    pdf.cell(0, 8, txt=line.encode('utf-8').decode('latin-1'), ln=True)

pdf.output("woordzoeker_concept.pdf")
