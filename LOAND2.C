/*
 * LOAND2.C - Loading screen com slideshow das 11 imagens BMP
 * Carrega PIC.BMP, PIC1.BMP ... PIC10.BMP uma de cada vez,
 * exibindo a barra de progresso conforme troca de imagem.
 * Modo VESA 640x480 256 cores (modo 0x101)
 * Compilar com: BCC -ml LOAND2.C
 */

#include <conio.h>
#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ======================= VESA / VGA ======================= */

void set_vesa_mode(int mode) {
    union REGS regs;
    regs.x.ax = 0x4F02;
    regs.x.bx = mode;
    int86(0x10, &regs, &regs);
}

void set_vesa_bank(int bank) {
    union REGS regs;
    regs.x.ax = 0x4F05;
    regs.x.bx = 0;
    regs.x.dx = bank;
    int86(0x10, &regs, &regs);
}

void set_palette(unsigned char index,
                 unsigned char r, unsigned char g, unsigned char b) {
    outp(0x03C8, index);
    outp(0x03C9, r);
    outp(0x03C9, g);
    outp(0x03C9, b);
}

/* ====================== DESENHO ========================== */

void draw_rect(int rx, int ry, int rw, int rh, unsigned char color) {
    int x, y;
    int current_bank = -1;
    long offset;
    int bank;
    unsigned int vga_offset;
    unsigned char far *vga;

    for (y = ry; y < ry + rh; y++) {
        offset = (long)y * 640 + rx;
        for (x = 0; x < rw; x++) {
            bank = (int)(offset >> 16);
            vga_offset = (unsigned int)(offset & 0xFFFF);
            if (bank != current_bank) {
                set_vesa_bank(bank);
                current_bank = bank;
            }
            vga = (unsigned char far *)MK_FP(0xA000, vga_offset);
            *vga = color;
            offset++;
        }
    }
}

/* ==================== BARRA DE PROGRESSO ================= */
/*
 * Barra de 10 blocos verdes.
 * step  = passo atual  (0..total_steps)
 * total = total de passos
 */
void draw_progress_bar(int step, int total) {
    int total_blocks = 10;
    int current_blocks;
    int b;

    /* Limpa fundo da barra */
    draw_rect(100, 400, 440, 30, 253); /* preto */

    current_blocks = (step * total_blocks) / total;
    for (b = 0; b < current_blocks; b++) {
        draw_rect(105 + (b * 43), 405, 38, 20, 255); /* verde */
    }
}

/* ==================== CARREGA BMP ======================== */
/*
 * Le a paleta do BMP e aplica no modo VESA.
 * Depois renderiza os pixels linha a linha (BMP e bottom-up).
 * Retorna 0 em sucesso, -1 em erro.
 */
int load_and_draw_bmp(const char *filename) {
    FILE *fp;
    unsigned char palette[1024];
    unsigned char far *vga = (unsigned char far *)MK_FP(0xA000, 0);
    unsigned char *buffer;
    int i, x, y;
    int w = 640, h = 480;
    long offset;
    int current_bank = -1;
    int new_bank;
    unsigned int vga_offset;

    fp = fopen(filename, "rb");
    if (!fp) {
        return -1;
    }

    /* Paleta fica em offset 54 no BMP de 8bpp */
    fseek(fp, 54, SEEK_SET);
    fread(palette, 1, 1024, fp);

    /* Aplica paleta (BGR -> RGB, escala 8->6 bits) */
    for (i = 0; i < 256; i++) {
        set_palette((unsigned char)i,
                    palette[i * 4 + 2] >> 2,
                    palette[i * 4 + 1] >> 2,
                    palette[i * 4    ] >> 2);
    }

    /* Reserva cores especiais da barra */
    set_palette(253, 0,  0,  0 );   /* preto  */
    set_palette(254, 63, 63, 63);   /* branco */
    set_palette(255, 0,  60, 0 );   /* verde  */

    /* Renderiza pixels */
    buffer = (unsigned char *)malloc(640);
    if (!buffer) {
        fclose(fp);
        return -1;
    }

    current_bank = -1;
    for (y = h - 1; y >= 0; y--) {
        fread(buffer, 1, 640, fp);

        offset    = (long)y * w;
        new_bank  = (int)(offset >> 16);
        vga_offset = (unsigned int)(offset & 0xFFFF);

        if (new_bank != current_bank) {
            set_vesa_bank(new_bank);
            current_bank = new_bank;
        }

        for (x = 0; x < w; x++) {
            vga[vga_offset] = buffer[x];
            vga_offset++;
            if (vga_offset == 0) {          /* overflow -> troca banco */
                current_bank++;
                set_vesa_bank(current_bank);
            }
        }
    }

    free(buffer);
    fclose(fp);
    return 0;
}

/* ========================= MAIN ========================== */

int main() {
    /*
     * Lista de imagens na ordem desejada de exibicao.
     * PIC.BMP  = indice 0 (sem numero)
     * PIC1.BMP = indice 1
     * ...
     * PIC10.BMP = indice 10
     * Total: 11 imagens
     */
    static const char *images[] = {
        "PIC.BMP",
        "PIC1.BMP",
        "PIC2.BMP",
        "PIC3.BMP",
        "PIC4.BMP",
        "PIC5.BMP",
        "PIC6.BMP",
        "PIC7.BMP",
        "PIC8.BMP",
        "PIC9.BMP",
        "PIC10.BMP"
    };
    int num_images = 11;

    /*
     * Duracao em ms que cada imagem fica na tela antes de
     * avancar para a proxima. Ajuste conforme quiser.
     */
    int dwell_ms = 900;   /* ~0.9 s por imagem */
    int step_ms  = 100;   /* granularidade do delay */

    int img;
    int steps_per_image;
    int s;
    int total_done;       /* imagens ja exibidas */
    int total_steps;      /* total de "passos" globais */

    /* Entra em modo VESA 640x480 256 cores */
    set_vesa_mode(0x101);

    steps_per_image = dwell_ms / step_ms;  /* = 9 */
    total_steps     = num_images * steps_per_image;

    for (img = 0; img < num_images; img++) {

        /* Carrega a imagem (tambem atualiza a paleta) */
        if (load_and_draw_bmp(images[img]) != 0) {
            /*
             * Imagem nao encontrada: desenha tela preta e continua.
             * Nao interrompe o slideshow.
             */
            draw_rect(0, 0, 640, 480, 253);
        }

        /* Re-desenha contorno da barra sobre a nova imagem */
        draw_rect(98,  398, 444, 34, 254);  /* borda branca */
        draw_rect(100, 400, 440, 30, 253);  /* fundo preto  */

        total_done = img * steps_per_image;

        /* Anima a barra durante o tempo de exibicao desta imagem */
        for (s = 1; s <= steps_per_image; s++) {
            draw_progress_bar(total_done + s, total_steps);
            delay(step_ms);
        }
    }

    /* Barra completa: todos os 10 blocos acesos */
    draw_progress_bar(total_steps, total_steps);
    delay(400);

    /* Aguarda tecla e volta ao modo texto */
    getch();

    {
        union REGS regs;
        regs.x.ax = 0x0003;
        int86(0x10, &regs, &regs);
    }

    return 0;
}
