#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#include "audio.h"
#include "camera.h"  
#include "inimigo.h"
#include <GL/glut.h>
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include "gamestate.h"

ma_engine engine;
ma_sound ambientSound;
ma_sound monsterSound;
ma_sound breathSound; 
ma_sound heartSound;  
ma_sound jumpscareSound;

static int lastMonsterSoundTime = 0;
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

    // ---- ALTERADO: Desativa o looping para tocar apenas uma vez ----
    ma_sound_set_looping(&breathSound, MA_FALSE);
    ma_sound_set_looping(&heartSound, MA_FALSE);
    ma_sound_set_volume(&heartSound, 1.4f); 

    isInitialized = true;
    lastMonsterSoundTime = glutGet(GLUT_ELAPSED_TIME);
}

void audioUpdate() {
    if (!isInitialized) return;

    // ---- NOVA LÓGICA DE JUMPSCARE NO ÁUDIO ----
    if (state == JUMPSCARE) {
        if (!ma_sound_is_playing(&jumpscareSound)) {
            ma_sound_seek_to_pcm_frame(&jumpscareSound, 0);
            ma_sound_start(&jumpscareSound);
        }
        // Silencia todos os outros sons bruscamente para focar no grito
        ma_sound_stop(&ambientSound);
        ma_sound_stop(&breathSound);
        ma_sound_stop(&heartSound);
        ma_sound_stop(&monsterSound);
        return; // Sai da função para não rodar mais nada
    } else {
        if (!ma_sound_is_playing(&ambientSound)) ma_sound_start(&ambientSound);
    }
    // -------------------------------------------

    // Lógica de distância do Monstro (mantida igual)
    float dist = sqrt(pow(px - ex, 2) + pow(pz - ez, 2));
    float dangerRadius = 35.0f; 
    float volume = (dist < dangerRadius) ? 1.0f - (dist / dangerRadius) : 0.0f;

    int now = glutGet(GLUT_ELAPSED_TIME);
    int randomPause = 3000 + (rand() % 4000); 
    
    if (now - lastMonsterSoundTime > randomPause) {
        if (volume > 0.05f) {
            ma_sound_set_volume(&monsterSound, volume);
            ma_sound_seek_to_pcm_frame(&monsterSound, 0); 
            ma_sound_start(&monsterSound);
        }
        lastMonsterSoundTime = now;
    }

    // ---- NOVA LÓGICA DE DISPARO ÚNICO (ONE-SHOT) ----
    static bool wasExhausted = false;

    if (isExhausted) {
        if (!wasExhausted) {
            // No frame exato em que a stamina zera, reseta e toca ambos os sons uma vez
            ma_sound_seek_to_pcm_frame(&breathSound, 0);
            ma_sound_seek_to_pcm_frame(&heartSound, 0);
            ma_sound_start(&breathSound);
            ma_sound_start(&heartSound);
            wasExhausted = true; // Trava o gatilho para não entrar em loop de frames
        }
    } else {
        if (wasExhausted) {
            // Se o fôlego se recuperar antes do áudio terminar por completo, corta o som
            ma_sound_stop(&breathSound);
            ma_sound_stop(&heartSound);
            wasExhausted = false; // Destrava para a próxima exaustão
        }
    }
}

void audioCleanup() {
    if (!isInitialized) return;
    ma_sound_uninit(&ambientSound);
    ma_sound_uninit(&monsterSound);
    ma_sound_uninit(&breathSound);
    ma_sound_uninit(&heartSound);
    ma_engine_uninit(&engine);
}