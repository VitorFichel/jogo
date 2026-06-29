#define MINIAUDIO_IMPLEMENTATION

#include "miniaudio.h"
#include "audio.h"
#include "camera.h"  
#include "inimigo.h"
#include "gamestate.h"
#include "maze.h" 
#include <GL/glut.h>
#include <cmath>
#include <cstdlib>
#include <cstdio>

ma_engine engine;
ma_sound ambientSound;
ma_sound breathSound; 
ma_sound heartSound;  
ma_sound jumpscareSound;

// Risadas e latin
static ma_sound laughs[4];
static ma_sound latinSound;
static int activeLaugh = -1; // qual risada está tocando (-1 = nenhuma)

static bool isInitialized = false;

// Toca uma risada aleatória, garantindo que não sobreponha outra
static void playRandomLaugh() {
    // Se ainda tem uma risada tocando, não interrompe
    if (activeLaugh >= 0 && ma_sound_is_playing(&laughs[activeLaugh]))
        return;

    activeLaugh = rand() % 4;
    ma_sound_seek_to_pcm_frame(&laughs[activeLaugh], 0);
    ma_sound_start(&laughs[activeLaugh]);
}

// Toca o latin (som raro de perseguição/tensão máxima)
static void playLatin() {
    if (ma_sound_is_playing(&latinSound)) return;
    ma_sound_seek_to_pcm_frame(&latinSound, 0);
    ma_sound_start(&latinSound);
}

void audioInit() {
    if (ma_engine_init(NULL, &engine) != MA_SUCCESS) return;

    ma_sound_init_from_file(&engine, "assets/audio/ambiente.mp3",  MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_ASYNC, NULL, NULL, &ambientSound);
    ma_sound_init_from_file(&engine, "assets/audio/respiracao.mp3",MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_ASYNC, NULL, NULL, &breathSound);
    ma_sound_init_from_file(&engine, "assets/audio/batimentos.mp3",MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_ASYNC, NULL, NULL, &heartSound);
    ma_sound_init_from_file(&engine, "assets/audio/jumpscare.mp3", MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_ASYNC, NULL, NULL, &jumpscareSound);

    ma_sound_init_from_file(&engine, "assets/audio/riso1.mp3", MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_ASYNC, NULL, NULL, &laughs[0]);
    ma_sound_init_from_file(&engine, "assets/audio/riso2.mp3", MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_ASYNC, NULL, NULL, &laughs[1]);
    ma_sound_init_from_file(&engine, "assets/audio/riso3.mp3", MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_ASYNC, NULL, NULL, &laughs[2]);
    ma_sound_init_from_file(&engine, "assets/audio/riso4.mp3", MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_ASYNC, NULL, NULL, &laughs[3]);
    ma_sound_init_from_file(&engine, "assets/audio/latin.mp3",  MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_ASYNC, NULL, NULL, &latinSound);

    // Todos os sons de susto/risada não fazem loop
    for (int i = 0; i < 4; i++)
        ma_sound_set_looping(&laughs[i], MA_FALSE);
    ma_sound_set_looping(&latinSound,    MA_FALSE);
    ma_sound_set_looping(&jumpscareSound,MA_FALSE);
    ma_sound_set_looping(&breathSound,   MA_FALSE);
    ma_sound_set_looping(&heartSound,    MA_FALSE);

    ma_sound_set_volume(&heartSound, 1.4f); 

    ma_sound_set_looping(&ambientSound, MA_TRUE);
    ma_sound_set_volume(&ambientSound, 0.3f); 
    ma_sound_start(&ambientSound);

    isInitialized = true;
}

void audioUpdate() {
    if (!isInitialized) return;

    if (state == JUMPSCARE) {
        if (!ma_sound_is_playing(&jumpscareSound)) {
            ma_sound_seek_to_pcm_frame(&jumpscareSound, 0);
            ma_sound_start(&jumpscareSound);
        }
        ma_sound_stop(&ambientSound);
        ma_sound_stop(&breathSound);
        ma_sound_stop(&heartSound);
        // Para qualquer risada que esteja tocando
        for (int i = 0; i < 4; i++) ma_sound_stop(&laughs[i]);
        ma_sound_stop(&latinSound);
        return; 
    } else {
        if (!ma_sound_is_playing(&ambientSound)) ma_sound_start(&ambientSound);
    }

    int now = glutGet(GLUT_ELAPSED_TIME);

    // ---- LÓGICA: LINHA DE VISÃO → RISADA ----
    float dx = ex - px;
    float dz = ez - pz;
    float dist = sqrt(dx * dx + dz * dz);
    float dangerRadius = 40.0f;

    static bool wasMonsterVisible = false;
    static int  lastSpottedTime   = 0;
    bool isVisible = false;

    if (dist < dangerRadius) {
        float dirX = dx / dist;
        float dirZ = dz / dist;
        float camX = cos(yaw);
        float camZ = sin(yaw);

        if ((dirX * camX + dirZ * camZ) > 0.75f) {
            bool hitWall = false;
            float step = 1.0f;
            for (float d = 0; d < dist; d += step) {
                if (checkCollisionAABB(px + dirX * d, pz + dirZ * d, 0.05f)) {
                    hitWall = true;
                    break;
                }
            }
            if (!hitWall) isVisible = true;
        }
    }

    // Jogador acabou de avistar o monstro → risada
    if (isVisible && !wasMonsterVisible) {
        if (now - lastSpottedTime > 10000) {
            playRandomLaugh();
            lastSpottedTime = now;
        }
    }
    wasMonsterVisible = isVisible;
    // ------------------------------------------

    // ---- RISADA PERIÓDICA ALEATÓRIA (20–45 s) ----
    static int nextRandomLaugh = 0;
    if (nextRandomLaugh == 0)
        nextRandomLaugh = now + 20000 + rand() % 25000; // primeiro gatilho

    if (now > nextRandomLaugh) {
        playRandomLaugh();
        nextRandomLaugh = now + 20000 + rand() % 25000;
    }
    // -----------------------------------------------

    // ---- LATIN: toca quando o monstro fica muito perto (< 5 unidades) ----
    static int lastLatinTime = 0;
    if (dist < 5.0f && now - lastLatinTime > 15000) {
        playLatin();
        lastLatinTime = now;
    }
    // -----------------------------------------------------------------------

    // ---- EXAUSTÃO (Stamina) ----
    static bool wasExhausted = false;
    if (isExhausted) {
        if (!wasExhausted) {
            ma_sound_seek_to_pcm_frame(&breathSound, 0);
            ma_sound_seek_to_pcm_frame(&heartSound,  0);
            ma_sound_start(&breathSound);
            ma_sound_start(&heartSound);
            wasExhausted = true;
        }
    } else {
        if (wasExhausted) {
            ma_sound_stop(&breathSound);
            ma_sound_stop(&heartSound);
            wasExhausted = false;
        }
    }
    // ----------------------------
}

// audioUpdateMonsterVolume não é mais necessária (sem patrolSound),
// mas mantida vazia caso algo ainda chame ela no main
void audioUpdateMonsterVolume(float px, float pz, float ex, float ez) {
    (void)px; (void)pz; (void)ex; (void)ez;
}

void audioCleanup() {
    if (!isInitialized) return;
    ma_sound_uninit(&ambientSound);
    ma_sound_uninit(&breathSound);
    ma_sound_uninit(&heartSound);
    ma_sound_uninit(&jumpscareSound);
    for (int i = 0; i < 4; i++) ma_sound_uninit(&laughs[i]);
    ma_sound_uninit(&latinSound);
    ma_engine_uninit(&engine);
}
