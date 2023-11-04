#include "video/VGA_text.h"
#include "device/ps2.h"
#include "device/keyboard.h"
#include "pit/pit.h"
#include "container/ring_buffer.h"

#include "stdlib.h"
#include "string.h"


#include <stddef.h>

#define HUD_SIZE 2
#define FLOOR_SIZE 3

#define PX 10
#define PIPES 5
#define PIPESPACING 20
#define PIPEGAP 10
#define PIPEWIDTH 4

VGA_Char buffer[VGA_SIZE];

int prng = 0;

int getRNG(void) {
    return prng++;
}

typedef enum {
    START = 0,
    PLAYING = 1,
    DEAD = 2,

} State;

State gamestate = START;

int playerY = 0;
int yVel = 0;
int score = 0;

int poffset = 0;
int pipes[PIPES];

void drawBackground(void) {
    VGA_Char top = getVGAchar(' ', white, black);
    VGA_Char bottom = getVGAchar('#', light_green, green);
    VGA_Char back = getVGAchar(' ', light_gray, light_blue);

    for (size_t i = 0; i < VGA_WIDTH * HUD_SIZE; ++i) {
        buffer[i] = top;
    }

    size_t floorStart = (VGA_HEIGHT - FLOOR_SIZE) * VGA_WIDTH;
    for (size_t i = VGA_WIDTH * HUD_SIZE; i < floorStart; ++i) {
        buffer[i] = back;
    }

    for (size_t i = floorStart; i < VGA_SIZE; ++i) {
        buffer[i] = bottom;
    }
}

void drawPlayer(void) {

    size_t pos = VGA_WIDTH * playerY + PX;

    buffer[pos] = getVGAchar(' ', black, yellow);
    buffer[pos + 1] = getVGAchar(' ', black, yellow);

    char buf[6] = "";
    itoa_s(score, buf, sizeof(buf));

    size_t sz = strnlen_s(buf, sizeof(buf));
    pos = (HUD_SIZE - 1) * VGA_WIDTH + VGA_WIDTH / 2 - sz / 2;

    for (size_t i = 0; i < sz; ++i) {
        buffer[pos + i] = getVGAchar(buf[i], white, black);
    }
}

void drawPipes(void) {

    VGA_Char pipeBody = getVGAchar(' ', light_green, light_green);
    //VGA_Char pipeEdge = getVGAchar('|', green, light_green);
    //VGA_Char pipeCapL = getVGAchar('[', green, light_green);
    //VGA_Char pipeCapR = getVGAchar(']', green, light_green);
    //VGA_Char pipeTop = getVGAchar('=', green, light_green);

    for (int i = 0; i < PIPES; ++i) {
        int x = i * PIPESPACING + poffset;

        if (x <= -PIPEWIDTH || x >= VGA_WIDTH) {
            continue;
        }

        int topY = pipes[i] - PIPEGAP / 2;
        int bottomY = pipes[i] + PIPEGAP / 2;

        
        for (int ty = HUD_SIZE; ty <= topY; ++ty) {
            int start = ty * VGA_WIDTH + x;

            for (int tx = 0; tx < PIPEWIDTH; ++tx) {
                if (tx + x < 0) {
                    continue;
                }
                else if (tx + x >= VGA_WIDTH) {
                    break;
                }

                buffer[start + tx] = pipeBody;
            }
        }


        for (int ty = bottomY; ty < VGA_HEIGHT - FLOOR_SIZE; ++ty) {
            int start = ty * VGA_WIDTH + x;

            for (int tx = 0; tx < PIPEWIDTH; ++tx) {
                if (tx + x < 0) {
                    continue;
                }
                else if (tx + x >= VGA_WIDTH) {
                    break;
                }

                buffer[start + tx] = pipeBody;
            }
        }


    }
}

void draw(void) {
    drawBackground();

    drawPipes();

    drawPlayer();

    // double buffering B)
    VGA_Char* mem = VGA_MEMORY;
    for (size_t i = 0; i < VGA_SIZE; ++i) {
        mem[i] = buffer[i];
    }
}

int genPipe(void) {
    int val = (((prng * 37) * (prng + 1)) % (VGA_HEIGHT - PIPEGAP - FLOOR_SIZE - HUD_SIZE - 1)) + PIPEGAP - 1;
    ++prng;
    return val;
}

void start(void) {
    playerY = 5;
    yVel = 0;
    score = 0;

    poffset = VGA_WIDTH / 2 + VGA_WIDTH / 4;

    for (size_t i = 0; i < PIPES; ++i) {
        pipes[i] = genPipe();
    }

    gamestate = PLAYING;
}

void tick(void) {
    bool jumped = false;
    struct PS2Buf_t current = popDev1();
    while (current.type != PS2_NONE_EVENT) {
        ++prng;
        if (current.type == PS2_KEY_EVENT && !jumped) {
            ++prng;
            if (current.keyEvent.code == Key_space) {
                jumped = true;
                yVel = -2;
            }
        }

        current = popDev1();
    }

    playerY += yVel;
    yVel += 1;

    if (yVel > 1) {
        yVel = 1;
    }

    if (playerY < HUD_SIZE) {
        yVel = 1;
        playerY = HUD_SIZE;
    }
    else if (playerY >= VGA_HEIGHT - FLOOR_SIZE) {
        yVel = 0;
        gamestate = 1;
        return;
    }

    --poffset;
    if (poffset <= -PIPEWIDTH) {
        poffset += PIPESPACING;

        for (size_t i = 0; i < PIPES - 1; ++i) {
            pipes[i] = pipes[i + 1];
        }

        pipes[PIPES - 1] = genPipe();
    }

    if (poffset + PIPEWIDTH == PX - 1) {
        ++score;
    }

    if (PX + 1 >= poffset && PX <= poffset + PIPEWIDTH) {
        if (playerY <= pipes[0] - PIPEGAP / 2 || playerY >= pipes[0] + PIPEGAP / 2) {
            gamestate = DEAD;
            return;
        }
    }


}

// this may say "test_main", but it's just main
void test_main() {

    start();

    size_t frame = 0;

    while (true) {
        ++frame;

        prng += 1;
        
        if (frame % 10 == 0) {
            tick();
        }

        draw();

        while (gamestate == DEAD) {

        }

        sleep(1);
    }

}