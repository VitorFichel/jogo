#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#include "audio.h"
#include "camera.h"  
#include "inimigo.h"
#include "gamestate.h"
#include "maze.h" // Adicionado para acessar o mazeIsWall()
#include <GL/glut.h>
#include <cmath>
#include <cstdlib>
#include <cstdio>

ma_engine engine;
ma_sound ambientSound;
ma_sound monsterSound;
ma_sound breathSound; 
ma_sound heartSound;  
ma_sound jumpscareSound;

static bool isInitialized = false;

void audioInit() {
    if (ma_engine_init(NULL, &engine) != MA_SUCCESS) return;

    ma_sound_init_from_file(&engine, "ambiente.mp3", MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_ASYNC, NULL, NULL, &ambientSound);
    ma_sound_init_from_file(&engine, "monstro.mp3", MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_ASYNC, NULL, NULL, &monsterSound);
    ma_sound_init_from_file(&engine, "respiracao.mp3", MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_ASYNC, NULL, NULL, &breathSound);
    ma_sound_init_from_file(&engine, "batimentos.mp3", MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_ASYNC, NULL, NULL, &heartSound);
    ma_sound_init_from_file(&engine, "jumpscare.mp3", MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_ASYNC, NULL, NULL, &jumpscareSound);

    ma_sound_set_looping(&ambientSound, MA_TRUE);
    ma_sound_set_volume(&ambientSound, 0.3f); 
    ma_sound_start(&ambientSound);

    ma_sound_set_looping(&breathSound, MA_FALSE);
    ma_sound_set_looping(&heartSound, MA_FALSE);
    ma_sound_set_volume(&heartSound, 1.4f); 

    // O som do monstro não precisa mais fazer loop, será disparado como um evento
    ma_sound_set_looping(&monsterSound, MA_FALSE);

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
        ma_sound_stop(&monsterSound);
        return; 
    } else {
        if (!ma_sound_is_playing(&ambientSound)) ma_sound_start(&ambientSound);
    }

    int now = glutGet(GLUT_ELAPSED_TIME);

    // ---- NOVA LÓGICA: SINAL SONORO DE LINHA DE VISÃO ----
    float dx = ex - px;
    float dz = ez - pz;
    float dist = sqrt(dx * dx + dz * dz);
    float dangerRadius = 40.0f; // Distância máxima para conseguir enxergá-lo no breu

    static bool wasMonsterVisible = false;
    static int lastSpottedTime = 0;
    bool isVisible = false;

    if (dist < dangerRadius) {
        // Normaliza o vetor para saber a direção real do monstro
        float dirX = dx / dist;
        float dirZ = dz / dist;

        // Calcula o vetor para onde a câmera (player) está olhando
        float camX = cos(yaw);
        float camZ = sin(yaw);

        // Produto Escalar (Dot Product): Verifica se ele está no cone de visão (FOV) frontal
        // 0.75f significa um campo de visão cônico de aproximadamente 80 graus
        if ((dirX * camX + dirZ * camZ) > 0.75f) {
            
            // Raycast (Line of Sight): Testa se tem alguma parede entre você e ele
            bool hitWall = false;
            float step = 1.0f; // Avança a checagem de 1 em 1 unidade
            for (float d = 0; d < dist; d += step) {
                if (mazeIsWall(px + dirX * d, pz + dirZ * d)) {
                    hitWall = true;
                    break;
                }
            }

            // Se está no FOV e não bateu em nenhuma parede, o monstro está visível!
            if (!hitWall) {
                isVisible = true;
            }
        }
    }

    // Toca o som apenas no frame exato em que ele entra na visão (transição false -> true)
    // Cooldown de 5 segundos impede que o som se repita como uma metralhadora se você chacoalhar o mouse
    if (isVisible && !wasMonsterVisible) {
        if (now - lastSpottedTime > 5000) {
            ma_sound_set_volume(&monsterSound, 1.0f); // Toca no volume máximo para assustar
            ma_sound_seek_to_pcm_frame(&monsterSound, 0);
            ma_sound_start(&monsterSound);
            lastSpottedTime = now;
        }
    }
    wasMonsterVisible = isVisible;
    // -----------------------------------------------------

    // Lógica de Exaustão (Stamina)
    static bool wasExhausted = false;

    if (isExhausted) {
        if (!wasExhausted) {
            ma_sound_seek_to_pcm_frame(&breathSound, 0);
            ma_sound_seek_to_pcm_frame(&heartSound, 0);
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
}

void audioCleanup() {
    if (!isInitialized) return;
    ma_sound_uninit(&ambientSound);
    ma_sound_uninit(&monsterSound);
    ma_sound_uninit(&breathSound);
    ma_sound_uninit(&heartSound);
    ma_sound_uninit(&jumpscareSound);
    ma_engine_uninit(&engine);
}