/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus-rsp-hle - regtest.c                                       *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2014 Bobby Smiles                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "common.h"

#include "hle.h"
#include "hle_internal.h"
#include "hle_external.h"

/* n64 state (at least what the rsp can see) */
struct n64state_t
{
    unsigned char dram[0x800000];
    unsigned char spmem[0x2000];
    uint32_t mi_intr_reg;
    uint32_t sp_regs[9];
    uint32_t dpc_regs[8];
};

/* function prototypes */
static void hle_bind_n64state(struct hle_t* hle, struct n64state_t* n64state);
static int hle_check_equal(const struct hle_t* expected, const struct hle_t* actual);

/*  */
int main(int UNUSED(argc), char** UNUSED(argv))
{
    clock_t t1, t2;
    struct hle_t hle, expected_hle;
    struct n64state_t* n64 = NULL;
    struct n64state_t* expected_n64 = NULL;
    int result = 0;

    /* FIXME: parse command line instead of hardcoding */
    const char* input_file = "alist_audio_200.bin";
    const char* output_file = "output_hle.bin";
    const char* expected_file = "expected_hle.bin";
    int exec_count = 1;
    int i;

    /* alloc n64state structures */
    n64 = malloc(sizeof(*n64));
    expected_n64 = malloc(sizeof(*expected_n64));

    memset(n64, 0, sizeof(*n64));
    memset(expected_n64, 0, sizeof(*expected_n64));

    if (n64 == NULL || expected_n64 == NULL) {
        printf("Error: couldn't allocate memory for n64 state!\n");
        result = 1;
        goto cleanup;
    }

    /* connect hle to n64 */
    hle_bind_n64state(&hle, n64);
    hle_bind_n64state(&expected_hle, expected_n64);

    /* load state files */
    if (hle_load(&hle, input_file)) {
        printf("Error: can't load state file %s\n", input_file);
        result = 1;
        goto cleanup;
    }

    if (hle_load(&expected_hle, expected_file)) {
        printf("Error: can't load state file %s\n", expected_file);
        result = 1;
        goto cleanup;
    }

    printf("starting execution\n");

    /* execution loop */
    t1 = clock();
    for(i = 0; i < exec_count; ++i)
        hle_execute(&hle);
    t2 = clock();

    /* compare states */
    result = hle_check_equal(&expected_hle, &hle);

    if (result != 0) {
        printf("%d comparison(s) failed !\n"
               "You can inspect resulting hle state in file %s\n",
               result, output_file);

        if (hle_save(&hle, output_file))
            printf("Error: couldn't save state file %s\n", output_file);
    }
    else {
        printf("Good job ! No regression !\n");
    }

    /* print execution times statistics */
    if (exec_count != 0) {
        float total_time = (float)(t2 - t1)/CLOCKS_PER_SEC * 1000.0;
        float avg_time = total_time / exec_count;
        printf("Total time=%.3fms Average time=%.3fms\n", total_time, avg_time);
    }

    /* cleanup allocated memory */
cleanup:
    free(n64); n64 = NULL;
    free(expected_n64); expected_n64 = NULL;

    /* return # of differences (0 == SUCESS) */
    return result;
}

static void hle_bind_n64state(struct hle_t* hle, struct n64state_t* n64state)
{
    hle_init(hle,
             &n64state->dram[0],
             &n64state->spmem[0],
             &n64state->spmem[0x1000],
             &n64state->mi_intr_reg,
             &n64state->sp_regs[0],
             &n64state->sp_regs[1],
             &n64state->sp_regs[2],
             &n64state->sp_regs[3],
             &n64state->sp_regs[4],
             &n64state->sp_regs[5],
             &n64state->sp_regs[6],
             &n64state->sp_regs[7],
             &n64state->sp_regs[8],
             &n64state->dpc_regs[0],
             &n64state->dpc_regs[1],
             &n64state->dpc_regs[2],
             &n64state->dpc_regs[3],
             &n64state->dpc_regs[4],
             &n64state->dpc_regs[5],
             &n64state->dpc_regs[6],
             &n64state->dpc_regs[7],
             NULL);
}

/* FIXME: move it to hle.c ? */
static int hle_check_equal(const struct hle_t* expected, const struct hle_t* actual)
{
    size_t k;
    int result = 0;

    /* FIXME: more detailed comparison report */

    /* n64 memory */
    if (memcmp(expected->dram, actual->dram, 0x800000)) { ++result; }
    if (memcmp(expected->dmem, actual->dmem, 0x1000))    { ++result; }
    if (memcmp(expected->imem, actual->imem, 0x1000))    { ++result; }

    /* n64 registers */
    if (*expected->mi_intr      != *actual->mi_intr)      { ++result; }
    if (*expected->sp_mem_addr  != *actual->sp_mem_addr)  { ++result; }
    if (*expected->sp_dram_addr != *actual->sp_dram_addr) { ++result; }
    if (*expected->sp_rd_length != *actual->sp_rd_length) { ++result; }
    if (*expected->sp_wr_length != *actual->sp_wr_length) { ++result; }
    if (*expected->sp_status    != *actual->sp_status)    { ++result; }
    if (*expected->sp_dma_full  != *actual->sp_dma_full)  { ++result; }
    if (*expected->sp_dma_busy  != *actual->sp_dma_busy)  { ++result; }
    if (*expected->sp_pc        != *actual->sp_pc)        { ++result; }
    if (*expected->sp_semaphore != *actual->sp_semaphore) { ++result; }
    if (*expected->dpc_start    != *actual->dpc_start)    { ++result; }
    if (*expected->dpc_end      != *actual->dpc_end)      { ++result; }
    if (*expected->dpc_current  != *actual->dpc_current)  { ++result; }
    if (*expected->dpc_status   != *actual->dpc_status)   { ++result; }
    if (*expected->dpc_clock    != *actual->dpc_clock)    { ++result; }
    if (*expected->dpc_bufbusy  != *actual->dpc_bufbusy)  { ++result; }
    if (*expected->dpc_pipebusy != *actual->dpc_pipebusy) { ++result; }
    if (*expected->dpc_tmem     != *actual->dpc_tmem)     { ++result; }

    /* user_defined is not taken into account */

    /* alist buffer */
    if (memcmp(expected->alist_buffer, actual->alist_buffer, 0x1000))   { ++result; }

    /* alist_audio state */
    for (k = 0; k < N_SEGMENTS; ++k) {
        if (expected->alist_audio.segments[k] != actual->alist_audio.segments[k]) { ++result; }
    }
    if (expected->alist_audio.in        != actual->alist_audio.in)        { ++result; }
    if (expected->alist_audio.out       != actual->alist_audio.out)       { ++result; }
    if (expected->alist_audio.count     != actual->alist_audio.count)     { ++result; }
    if (expected->alist_audio.dry_right != actual->alist_audio.dry_right) { ++result; }
    if (expected->alist_audio.wet_left  != actual->alist_audio.wet_left)  { ++result; }
    if (expected->alist_audio.wet_right != actual->alist_audio.wet_right) { ++result; }
    if (expected->alist_audio.dry       != actual->alist_audio.dry)       { ++result; }
    if (expected->alist_audio.wet       != actual->alist_audio.wet)       { ++result; }
    for (k = 0; k < 2; ++k) {
        if (expected->alist_audio.vol[k]    != actual->alist_audio.vol[k])    { ++result; }
        if (expected->alist_audio.target[k] != actual->alist_audio.target[k]) { ++result; }
        if (expected->alist_audio.rate[k]   != actual->alist_audio.rate[k])   { ++result; }
    }
    if (expected->alist_audio.loop       != actual->alist_audio.loop)      { ++result; }
    for (k = 0; k < 16*8; ++k) {
        if (expected->alist_audio.table[k]   != actual->alist_audio.table[k])  { ++result; }
    }

    if (expected->alist_naudio.dry != actual->alist_naudio.dry) { ++result; }
    if (expected->alist_naudio.wet != actual->alist_naudio.wet) { ++result; }
    for (k = 0; k < 2; ++k) {
        if (expected->alist_naudio.vol[k]    != actual->alist_naudio.vol[k])    { ++result; }
        if (expected->alist_naudio.target[k] != actual->alist_naudio.target[k]) { ++result; }
        if (expected->alist_naudio.rate[k]   != actual->alist_naudio.rate[k])   { ++result; }
    }
    if (expected->alist_naudio.loop != actual->alist_naudio.loop) { ++result; }
    for (k = 0; k < 16*8; ++k) {
        if (expected->alist_naudio.table[k] != actual->alist_naudio.table[k]) { ++result; }
    }

    /* alist_nead state */
    if (expected->alist_nead.in != actual->alist_nead.in) { ++result; }
    if (expected->alist_nead.out != actual->alist_nead.out) { ++result; }
    if (expected->alist_nead.count != actual->alist_nead.count) { ++result; }
    for (k = 0; k < 3; ++k) {
        if (expected->alist_nead.env_values[k] != actual->alist_nead.env_values[k]) { ++result; }
        if (expected->alist_nead.env_steps[k] != actual->alist_nead.env_steps[k]) { ++result; }
    }
    if (expected->alist_nead.loop != actual->alist_nead.loop) { ++result; }
    for (k = 0; k < 16*8; ++k) {
        if (expected->alist_nead.table[k] != actual->alist_nead.table[k]) { ++result; }
    }
    if (expected->alist_nead.filter_count != actual->alist_nead.filter_count) { ++result; }
    for (k = 0; k < 2; ++k) {
        if (expected->alist_nead.filter_lut_address[k]
        != actual->alist_nead.filter_lut_address[k]) { ++result; }
    }

    /* mp3 ucode state */
    if (memcmp(expected->mp3_buffer, actual->mp3_buffer, 0x1000)) { ++result; }

    return result;
}

/* Global functions needed by HLE core */
void HleVerboseMessage(void* UNUSED(user_defined), const char *message, ...)
{
    printf("HLE: ");

    va_list args;
    va_start(args, message);
    vprintf(message, args);
    va_end(args);

    printf("\n");
}

void HleErrorMessage(void* UNUSED(user_defined), const char *message, ...)
{
    printf("HLE: ");

    va_list args;
    va_start(args, message);
    vprintf(message, args);
    va_end(args);

    printf("\n");
}

void HleWarnMessage(void* UNUSED(user_defined), const char *message, ...)
{
    printf("HLE: ");

    va_list args;
    va_start(args, message);
    vprintf(message, args);
    va_end(args);

    printf("\n");
}

void HleCheckInterrupts(void* UNUSED(user_defined))
{
    /* disabled */
}

void HleProcessDlistList(void* UNUSED(user_defined))
{
    /* disabled */
}

void HleProcessAlistList(void* UNUSED(user_defined))
{
    /* disabled */
}

void HleProcessRdpList(void* UNUSED(user_defined))
{
    /* disabled */
}

void HleShowCFB(void* UNUSED(user_defined))
{
    /* disabled */
}

