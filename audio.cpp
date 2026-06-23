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
ma_sound monsterSound;
ma_sound breathSound; 
ma_sound heartSound;  
ma_sound jumpscareSound;
ma_sound monsterPatrolSound; // Novo som para a patrulha contínua

static bool isInitialized = false;

void audioInit() {
    if (ma_engine_init(NULL, &engine) != MA_SUCCESS) return;

    ma_sound_init_from_file(&engine, "assets/audio/ambiente.mp3", MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_ASYNC, NULL, NULL, &ambientSound);
    ma_sound_init_from_file(&engine, "assets/audio/monstro.mp3", MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_ASYNC, NULL, NULL, &monsterSound);
    ma_sound_init_from_file(&engine, "assets/audio/respiracao.mp3", MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_ASYNC, NULL, NULL, &breathSound);
    ma_sound_init_from_file(&engine, "assets/audio/batimentos.mp3", MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_ASYNC, NULL, NULL, &heartSound);
    ma_sound_init_from_file(&engine, "assets/audio/jumpscare.mp3", MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_ASYNC, NULL, NULL, &jumpscareSound);
    
    // Inicia o som de passos/patrulha. Adicione um arquivo "passos.mp3" ou mude o nome aqui.
    ma_sound_init_from_file(&engine, "assets/audio/passos.mp3", MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_ASYNC, NULL, NULL, &monsterPatrolSound);

    ma_sound_set_looping(&ambientSound, MA_TRUE);
    ma_sound_set_volume(&ambientSound, 0.3f); 
    ma_sound_start(&ambientSound);

    ma_sound_set_looping(&breathSound, MA_FALSE);
    ma_sound_set_looping(&heartSound, MA_FALSE);
    ma_sound_set_volume(&heartSound, 1.4f); 

    // O som do monstro visualizado não faz loop
    ma_sound_set_looping(&monsterSound, MA_FALSE);

    // O som de patrulha faz loop o tempo todo, mas começa mutado
    ma_sound_set_looping(&monsterPatrolSound, MA_TRUE);
    ma_sound_set_volume(&monsterPatrolSound, 0.0f);
    ma_sound_start(&monsterPatrolSound);

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
        ma_sound_stop(&monsterPatrolSound); // Para a patrulha durante o susto
        return; 
    } else {
        if (!ma_sound_is_playing(&ambientSound)) ma_sound_start(&ambientSound);
        if (!ma_sound_is_playing(&monsterPatrolSound)) ma_sound_start(&monsterPatrolSound);
    }

    int now = glutGet(GLUT_ELAPSED_TIME);

    // ---- LÓGICA: SINAL SONORO DE LINHA DE VISÃO ----
    float dx = ex - px;
    float dz = ez - pz;
    float dist = sqrt(dx * dx + dz * dz);
    float dangerRadius = 40.0f; 

    static bool wasMonsterVisible = false;
    static int lastSpottedTime = 0;
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
            if (!hitWall) {
                isVisible = true;
            }
        }
    }

    if (isVisible && !wasMonsterVisible) {
        if (now - lastSpottedTime > 5000) {
            ma_sound_set_volume(&monsterSound, 1.0f); 
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

// ---- ATUALIZA O VOLUME BASEADO NA DISTÂNCIA ----
void audioUpdateMonsterVolume(float px, float pz, float ex, float ez) {
    if (!isInitialized || state == JUMPSCARE) return;

    float dx = px - ex;
    float dz = pz - ez;
    float distance = sqrt(dx * dx + dz * dz);

    float maxHearingDistance = 15.0f; // Distância máxima para ouvir o monstro

    float volume = 1.0f - (distance / maxHearingDistance);

    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;

    ma_sound_set_volume(&monsterPatrolSound, volume);
}

void audioCleanup() {
    if (!isInitialized) return;
    ma_sound_uninit(&ambientSound);
    ma_sound_uninit(&monsterSound);
    ma_sound_uninit(&breathSound);
    ma_sound_uninit(&heartSound);
    ma_sound_uninit(&jumpscareSound);
    ma_sound_uninit(&monsterPatrolSound);
    ma_engine_uninit(&engine);
}