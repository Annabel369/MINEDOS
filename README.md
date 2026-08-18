<div align="center">

# 🎮 128GN CEAMARE & MINEDOS 3D (MS-DOS)

<img width="1339" height="800" alt="Gemini_Generated_Image_gra3a1gra3a1gra3" src="https://github.com/user-attachments/assets/ac889642-3b46-4fbf-bf15-4733e73163d0" />


[![DOS](https://img.shields.io/badge/OS-MS--DOS%20%2F%20FreeDOS-orange.svg)](https://www.freedos.org/)
[![Compiler](https://img.shields.io/badge/Compiler-Borland%20C%2B%2B%203.1-blue.svg)](https://en.wikipedia.org/wiki/Borland_C%2B%2B)
[![Graphics](https://img.shields.io/badge/Graphics-VGA%20Mode%2013h%20(320x200)-brightgreen.svg)](https://en.wikipedia.org/wiki/Mode_13h)
[![Status](https://img.shields.io/badge/MINEDOS-v4.0%20Release-red.svg)]()

*Um ambiente bootável retrô híbrido para desenvolvimento clássico em **C++**, **Pascal**, multimídia DOS e o projeto **MINEDOS** (Minecraft 3D para MS-DOS).*

---

</div>

## ⛏️ Projeto MINEDOS v4.0 (Minecraft 3D Retro)

O **MINEDOS** é uma recriação nostálgica em **C++** (compilada com **Borland C++ 3.1**) do universo Minecraft feita para rodar em modo real de 16-bits no **MS-DOS**, **FreeDOS** ou via **DOSBox** com **100% de ciclos de CPU**!

<div align="center">
  <img src="FRAMES/PIC7.BMP" width="600" alt="MINEDOS MS-DOS Banner" />
</div>

### 🌟 Principais Recursos do MINEDOS v4.0:

* 🎮 **Motor 3D Voxel Raycaster (Modo 13h)**: Renderização $320 \times 200$ em 256 cores com *Double Buffering* de 64KB em memória convencional (zero oscilação de tela).
* 🖱️ **Driver de Mouse MS-DOS (`INT 33h`)**:
  * Mira 360° fluída movendo o mouse.
  * **Clique Esquerdo**: Quebrar / Minerar bloco na mira.
  * **Clique Direito**: Colocar / Construir bloco selecionado.
* 🦾 **Braço & Mão Animados em 1ª Pessoa**:
  * Exibe o braço do jogador segurando um mini-bloco 3D do item ativo.
  * Animação dinâmica de golpe / soco ao minerar e construir.
* 🌅 **Ciclo de Tempo Dinâmico (Dia, Tarde & Noite)**:
  * **Dia (`D`)**: Céu Azul Claro com iluminação total.
  * **Tarde (`T`)**: Pôr do Sol Dourado/Laranja.
  * **Noite (`N`)**: Céu Noturno Estrelado com Lua e sombreamento noturno.
  * Alternância automática ou manual (Tecla **`T`**).
* 🖼️ **Estrutura de Subpastas de Quadros (`FRAMES/`)**:
  * Carregador de imagens `.BMP` baseado na lógica do `SHOWPIC3.C`.
  * Cutscene de abertura sequencial passando pelos quadros `FRAMES/PIC7.BMP`, `FRAMES/DIA/`, `FRAMES/TARDE/` e `FRAMES/NOITE/`.
* 👾 **Mobs 3D no Mundo**:
  * **Creeper (Verde)**: Mob voxel com rosto clássico gerado nas florestas.
  * **Ovelha Rosa (Pink Sheep)**: Mob voxel de lã rosa gerado nos campos.
* 🧱 **9 Tipos de Blocos Na Hotbar (`1` a `9`)**:
  `Grama`, `Terra`, `Pedra`, `Madeira`, `Folhas`, `Tijolo`, `Chip de Silício (Tech Blue)`, `Ouro Pirata` e `Laço Rosa Minnie`.

---

## ⚡ Como Rodar o Projeto (Quick Start)

### No Linux (Debian / Ubuntu):

Basta executar o script de inicialização que configura o DOSBox em 100% da CPU e abre o menu:

```bash
./Ligar.sh
```

No menu principal que surgir, pressione **`C`** para iniciar o **MINEDOS**!

### Compilação Manual no MS-DOS / Borland C++ 3.1:

```bat
CD \GAME\MINEDOS
COMP_MC.BAT
MINEDOS.EXE
```

---

## 📁 Estrutura do Repositório

```text
/128GN CEAMARE/
├── MINEDOS.CPP                 # Código-fonte principal C++ (v4.0)
├── MINEDOS.EXE                 # Executável compilado (16-bit DOS)
├── COMP_MC.BAT                 # Script de compilação Borland C++ 3.1
├── LEAME.TXT                   # Manual completo do jogo em Português
├── MENU.BAT                    # Menu principal do sistema DOS
├── Ligar.sh                    # Script de lançamento rápido para Linux
├── BCPP31/                     # Compilador Borland C++ 3.1
├── TP7/                        # Turbo Pascal 7
└── game/
    └── MINEDOS/                # Pasta dedicada do projeto MINEDOS
        ├── MINEDOS.CPP
        ├── MINEDOS.EXE
        ├── LEAME.TXT
        └── FRAMES/             # Quadros e Texturas BMP
             ├── PIC7.BMP
             ├── GRASS.BMP, DIRT.BMP, STONE.BMP, etc.
             ├── DIA/           # Cenários de Dia
             ├── TARDE/         # Cenários de Pôr do Sol
             └── NOITE/         # Cenários Noturnos
```

---

## 🛠 Arquitetura do MS-DOS & HIMEM.SYS

Este repositório também inclui documentação técnica e utilitários históricos sobre a gestão de memória x86 no MS-DOS.

### O Contexto Histórico
Nos processadores Intel 8086/80286, o MS-DOS possuía o limite de **1 MB de memória RAM**, onde a **Memória Convencional** (primeiros 640 KB) era o espaço máximo para rodar programas.

| Tipo de Memória | Endereço | Sem HIMEM.SYS | Com HIMEM.SYS |
| :--- | :--- | :--- | :--- |
| **Estendida (XMS)** | Acima de 1 MB | Inacessível para a maioria dos programas. | Acessível para armazenar dados de programas compatíveis. |
| **High Memory Area (HMA)** | Logo acima de 1 MB | Não gerenciada. | Gerenciada. Pode conter o núcleo do DOS (`DOS=HIGH`). |
| **Barreira de 1 MB** | **1 MB** | **Fim da linha.** | **Ponte criada.** |
| **Convencional** | 0 a 640 KB | Cheia (DOS + drivers + programa). | Mais vazia (DOS movido para HMA). |

---

## 📜 Licença & Contribuição

Este projeto é desenvolvido para fins educacionais, retrô e de preservação histórica. Sinta-se à vontade para abrir Pull Requests,Issues ou sugerir novos blocos e recursos para o **MINEDOS**!
