======================================================================
                  MINEDOS v4.0 - MINECRAFT 3D FOR MS-DOS
======================================================================
Desenvolvido em C++ para Borland C++ 3.1 (BCPP31)
Modo de Vídeo: VGA Modo 13h (320x200 pixels - 256 Cores)
Tecnologia: Rendering 3D Voxel / Raycaster + Double Buffering 64KB
Suporte a Mouse MS-DOS (INT 33h) & Ciclo Dia / Tarde / Noite

----------------------------------------------------------------------
1. SOBRE O JOGO
----------------------------------------------------------------------
O MINEDOS v4.0 e uma recriacao retro e nostalgica do universo Minecraft
feita especialmente para rodar em computadores MS-DOS, FreeDOS ou via
DOSBox com 100% da CPU!

O jogo conta com:
  - Renderizacao 3D de blocos com sombreamento direcional.
  - Suporte total a Mouse MS-DOS (Mira 360° e cliques de minerar/construir).
  - Braco em 1ª Pessoa com animacao de golpe e segurando mini-bloco 3D.
  - Ciclo de Tempo Dinamico: DIA, TARDE (Por do Sol) e NOITE.
  - Subpastas de Quadros de Imagem: FRAMES\DIA\, FRAMES\TARDE\, FRAMES\NOITE\.
  - Cutscene de Abertura com quadros sequenciais (PIC7.BMP).
  - Mobs 3D no Mundo: Creeper (Verde) e Ovelha Rosa (Pink Sheep).
  - 9 Tipos de Blocos: Grama, Terra, Pedra, Madeira, Folhas, Tijolo,
    Chip de Silicio (Tech Blue), Ouro Pirata e Laco Rosa Minnie.

----------------------------------------------------------------------
2. CONTROLES DO JOGO
----------------------------------------------------------------------
  [ CONTROLES DE MOUSE (INT 33h) ]
  - Mover o Mouse        : Girar Câmera em 360° (Mira Livre)
  - Botão Esquerdo (LMB) : Quebrar / Minerar Bloco na Mira
  - Botão Direito (RMB)  : Colocar / Construir Bloco Selecionado

  [ CONTROLES DE TECLADO ]
  - [ W ] / [ S ]        : Mover para Frente / Trás
  - [ A ] / [ D ]        : Girar Câmera para Esquerda / Direita
  - [ R ] / [ F ]        : Mover Câmera para Cima / Baixo (Voo)
  - [ Q ] / [ E ]        : Olhar para Cima / Baixo (Inclinação)
  - Setas do Teclado     : Movimentação / Olhar alternativo

  [ SELECAO DE BLCOS DA HOTBAR ]
  - [ 1 ] = Grama (Grass)           [ 6 ] = Tijolo (Brick)
  - [ 2 ] = Terra (Dirt)            [ 7 ] = Chip de Silício (Silicon)
  - [ 3 ] = Pedra (Stone)           [ 8 ] = Ouro Pirata (Gold)
  - [ 4 ] = Madeira (Wood)          [ 9 ] = Laço Rosa Minnie (Pink Bow)
  - [ 5 ] = Folhas (Leaves)

  [ TECLAS DE ATALHO ESPECIAS ]
  - [ T ]                : Alternar Ciclo de Tempo (DIA -> TARDE -> NOITE)
  - [ ESPACO ] / [ B ]   : Destruir Bloco (com animação de soco no braço)
  - [ ENTER ]  / [ C ]   : Colocar Bloco (com animação de soco no braço)
  - [ TAB ]              : Regenerar Terreno e Mobs no Mapa
  - [ ESC ]              : Sair do Jogo e Retornar ao Prompt do MS-DOS

----------------------------------------------------------------------
3. ESTRUTURA DAS PASTAS E IMAGENS
----------------------------------------------------------------------
  /game/MINEDOS/
    ├── MINEDOS.CPP      -> Código-fonte C++ v4.0 (Borland C++ 3.1)
    ├── MINEDOS.EXE      -> Executável final para MS-DOS
    ├── COMP_MC.BAT      -> Script de compilação automatizada
    ├── LEAME.TXT        -> Este manual de instruções
    ├── setup.txt        -> Lançador automático do menu DOS
    └── FRAMES/          -> Subpasta de quadros e texturas BMP
         ├── PIC7.BMP    -> Frame de abertura principal
         ├── GRASS.BMP   -> Textura 16x16 de Grama
         ├── DIRT.BMP    -> Textura 16x16 de Terra
         ├── STONE.BMP   -> Textura 16x16 de Pedra
         ├── WOOD.BMP    -> Textura 16x16 de Madeira
         ├── LEAVES.BMP  -> Textura 16x16 de Folhas
         ├── BRICK.BMP   -> Textura 16x16 de Tijolo
         ├── SILICON.BMP -> Textura 16x16 de Chip de Silício
         ├── GOLD.BMP    -> Textura 16x16 de Ouro Pirata
         ├── PINK.BMP    -> Textura 16x16 de Laço Rosa
         ├── DIA/        -> Subpasta do cenário de Dia (PIC7.BMP)
         ├── TARDE/      -> Subpasta do cenário de Pôr do Sol (PIC7.BMP)
         └── NOITE/      -> Subpasta do cenário Noturno (PIC7.BMP)

----------------------------------------------------------------------
4. COMO COMPILAR E JOGAR
----------------------------------------------------------------------
No seu terminal Linux Debian:
  1. Execute: ./Ligar.sh
  2. No Menu Principal do DOS, digite a tecla [ C ] para iniciar o jogo!

Caso queira recompilar manualmente no MS-DOS:
  CD \GAME\MINEDOS
  COMP_MC.BAT

======================================================================
            Divirta-se criando no MINEDOS para MS-DOS!
======================================================================
