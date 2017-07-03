/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus-rsp-hle - zsort_wdc.c                                     *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2015 Bobby Smiles                                       *
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

#include <stdbool.h>

#include "hle_external.h"
#include "hle_internal.h"
#include "memory.h"
#include "hle_internal.h"

// FIXME: use hle->zsort_wdc struct instead of global variables
enum zsort_task_state
{
    INIT,
    MAIN_DL,
    SUB_DL,
    RDP_DL,
    WAIT_YIELD_SIGNAL,
    TASK_DONE
};

uint32_t main_dl_ptr;
uint32_t sub_dl_ptr;
uint32_t rdp_dl_ptr;
uint32_t task_flags;
enum zsort_task_state task_state = INIT;
enum zsort_task_state task_state_at_rdp_dl_begin = MAIN_DL;
int yield_state = 0;
int b_941 = 0;
uint16_t u16_f00;
int s5;

static void flush_rdp_commands_buffer(struct hle_t* hle);

typedef void (*gcmd_callback)(struct hle_t*, uint32_t w1, uint32_t w2);

static void gcmd_0000(struct hle_t* hle, uint32_t w1, uint32_t w2);
static void gcmd_1188(struct hle_t* hle, uint32_t w1, uint32_t w2);
static void gcmd_11ac(struct hle_t* hle, uint32_t w1, uint32_t w2);
static void gcmd_spnoop(struct hle_t* hle, uint32_t w1, uint32_t w2);
static void gcmd_dma(struct hle_t* hle, uint32_t w1, uint32_t w2);
static void gcmd_setword(struct hle_t* hle, uint32_t w1, uint32_t w2);
static void gcmd_transpose3x3(struct hle_t* hle, uint32_t w1, uint32_t w2);
static void gcmd_1258(struct hle_t* hle, uint32_t w1, uint32_t w2);
static void gcmd_12fc(struct hle_t* hle, uint32_t w1, uint32_t w2);
static void gcmd_1444(struct hle_t* hle, uint32_t w1, uint32_t w2);
static void gcmd_1570(struct hle_t* hle, uint32_t w1, uint32_t w2);
static void gcmd_1610(struct hle_t* hle, uint32_t w1, uint32_t w2);
static void gcmd_1664(struct hle_t* hle, uint32_t w1, uint32_t w2);
static void gcmd_16f8(struct hle_t* hle, uint32_t w1, uint32_t w2);
static void gcmd_1704(struct hle_t* hle, uint32_t w1, uint32_t w2);
static void gcmd_1720(struct hle_t* hle, uint32_t w1, uint32_t w2);
static void gcmd_1758(struct hle_t* hle, uint32_t w1, uint32_t w2);
static void gcmd_1790(struct hle_t* hle, uint32_t w1, uint32_t w2);
static void gcmd_17a4(struct hle_t* hle, uint32_t w1, uint32_t w2);
static void gcmd_rdpcmd(struct hle_t* hle, uint32_t w1, uint32_t w2);
static void gcmd_17ec(struct hle_t* hle, uint32_t w1, uint32_t w2);
static void gcmd_17f4(struct hle_t* hle, uint32_t w1, uint32_t w2);
static void gcmd_1bcc(struct hle_t* hle, uint32_t w1, uint32_t w2);
static void gcmd_1bf0(struct hle_t* hle, uint32_t w1, uint32_t w2);
static void gcmd_1cd4(struct hle_t* hle, uint32_t w1, uint32_t w2);
static void gcmd_1cf0(struct hle_t* hle, uint32_t w1, uint32_t w2);
static void gcmd_1d20(struct hle_t* hle, uint32_t w1, uint32_t w2);
static void gcmd_1fbc(struct hle_t* hle, uint32_t w1, uint32_t w2);

static const gcmd_callback GBI[] =
{
    gcmd_spnoop,            // 00
    gcmd_11ac,              // 01
    gcmd_dma,               // 02
    gcmd_setword,           // 03
    gcmd_transpose3x3,      // 04
    gcmd_1258,              // 05
    gcmd_12fc,              // 06
    gcmd_1610,              // 07
    gcmd_17f4,              // 08
    gcmd_1fbc,              // 09
    gcmd_1444,              // 0a
    gcmd_1444,              // 0b
    gcmd_1570,              // 0c
    gcmd_1188,              // 0d
    gcmd_1bf0,              // 0e
    gcmd_1cd4,              // 0f
    gcmd_0000,              // 10
    gcmd_1cf0,              // 11
    gcmd_1d20,              // 12
    gcmd_1bcc,              // 13

    gcmd_1790,              // dd
    gcmd_17ec,              // de
    gcmd_1664,              // df
    gcmd_rdpcmd,            // e0
    gcmd_16f8,              // e1
    gcmd_1758,              // e2
    gcmd_1720,              // e3
    gcmd_rdpcmd,            // e4
    gcmd_rdpcmd,            // e5
    gcmd_rdpcmd,            // e6
    gcmd_rdpcmd,            // e7
    gcmd_rdpcmd,            // e8
    gcmd_rdpcmd,            // e9
    gcmd_rdpcmd,            // ea
    gcmd_rdpcmd,            // eb
    gcmd_rdpcmd,            // ec
    gcmd_rdpcmd,            // ed
    gcmd_rdpcmd,            // ee
    gcmd_17a4,              // ef
    gcmd_rdpcmd,            // f0
    gcmd_1704,              // f1
    gcmd_rdpcmd,            // f2
    gcmd_rdpcmd,            // f3
    gcmd_rdpcmd,            // f4
    gcmd_rdpcmd,            // f5
    gcmd_rdpcmd,            // f6
    gcmd_rdpcmd,            // f7
    gcmd_rdpcmd,            // f8
    gcmd_rdpcmd,            // f9
    gcmd_rdpcmd,            // fa
    gcmd_rdpcmd,            // fb
    gcmd_rdpcmd,            // fc
    gcmd_rdpcmd,            // fd
    gcmd_rdpcmd,            // fe
    gcmd_rdpcmd             // ff
};


/**************************************************************************
 * Z-Sort based World Driver Championship / Stunt Racer ucode
 **************************************************************************/
void zsort_wdc_task(struct hle_t* hle)
{
    bool run_loop = true;
    const uint32_t* w;
    unsigned int idx;

    while(run_loop)
    {
        switch(task_state)
        {
            case INIT:
                main_dl_ptr = *dmem_u32(hle, TASK_DATA_PTR);
                sub_dl_ptr = *dmem_u32(hle, TASK_YIELD_DATA_PTR);
                task_flags = *dmem_u32(hle, TASK_FLAGS);

                HleWarnMessage(hle, "zsort_wdc: exec task main_dl=%08x sub_dl=%08x flags=%08x",
                        main_dl_ptr, sub_dl_ptr, task_flags);

                /* setup ucode variables */
                yield_state = 0;
                b_941 = 0;

                /* start execute ucode */
                *hle->sp_status &= ~0x300;  // clear sig1 | sig2
                *hle->sp_status |= 0x800;   // set sig4

                *hle->mi_intr |= MI_INTR_SP;
                HleCheckInterrupts(hle->user_defined);

                if (task_flags & 0x8)
                {
                    HleWarnMessage(hle, "task reset flag set !");
                }

                task_state = MAIN_DL;
                break;

            case MAIN_DL:
                HleWarnMessage(hle, "zsort_wdc: main_dl=%08x", main_dl_ptr);
                w = dram_u32(hle, main_dl_ptr);
                idx = w[0] >> 25;

                *hle->mi_intr |= MI_INTR_SP;
                HleCheckInterrupts(hle->user_defined);

                if (*hle->sp_status & 0x80)
                {
                    *hle->sp_status &= ~0x80;
                    yield_state = 1;
                }

                main_dl_ptr += 0x08;
                GBI[idx](hle, w[0], w[1]);
                break;

            case SUB_DL:
                HleWarnMessage(hle, "zsort_wdc: sub_dl=%08x", sub_dl_ptr);
                w = dram_u32(hle, sub_dl_ptr);
                idx = w[0] >> 25;
                sub_dl_ptr += 0x08;
                GBI[idx](hle, w[0], w[1]);
                break;

            case RDP_DL:
                HleWarnMessage(hle, "zsort_wdc: rdp_dl=%08x", rdp_dl_ptr);
                w = dram_u32(hle, rdp_dl_ptr);
                idx = (w[0] >> 24) - 0xc9;

//                if (s7 >= 0x158)
//                    flush_rdp_commands_buffer(hle);

                rdp_dl_ptr += 0x08;
                GBI[idx](hle, w[0], w[1]);
                break;

            case WAIT_YIELD_SIGNAL:
                HleWarnMessage(hle, "zsort_wdc: WAIT_YIELD_SIGNAL");

                *hle->mi_intr |= MI_INTR_SP;
                HleCheckInterrupts(hle->user_defined);

                if (*hle->sp_status & 0x80)
                {
                    *hle->sp_status &= ~0x80;
                    yield_state = 1;
                    task_state = SUB_DL;
                }
                else
                {
                    run_loop = false;
                }
                break;

            case TASK_DONE:
                HleWarnMessage(hle, "zsort_wdc: task done !");
                run_loop = false;
                rsp_break(hle, SP_STATUS_TASKDONE);
                /* prepare next task invocation */
                task_state = INIT;
                break;


            default:
                HleWarnMessage(hle, "zsort_wdc: unknown state !");
                run_loop = false;
                break;
        }
    }

    HleWarnMessage(hle, "zsort_wdc: return");
}



static void flush_rdp_commands_buffer(struct hle_t* hle)
{
    HleWarnMessage(hle, "zsort_wdc: Flush RDP Commands buffer !");
    HleProcessRdpList(hle);
}

static void gcmd_0000(struct hle_t* hle, uint32_t w1, uint32_t w2)
{
    HleWarnMessage(hle, "GCMD_0000: %08x %08x", w1, w2);
}

static void gcmd_1188(struct hle_t* hle, uint32_t w1, uint32_t w2)
{
    HleWarnMessage(hle, "GCMD_1188: %08x %08x", w1, w2);

    if (b_941 != 0)
    {
        task_state = TASK_DONE;
        return;
    }

    yield_state = -1;
    // FIXME: task_state can be either MAIN_DL or DRAW_ZOBJECT
    task_state = MAIN_DL;
}

static void gcmd_11ac(struct hle_t* hle, uint32_t w1, uint32_t w2)
{
    HleWarnMessage(hle, "GCMD_11AC: %08x %08x", w1, w2);

    if (yield_state < 0)
    {
        task_state = TASK_DONE;
        return;
    }

    b_941 = -1;

    task_state = (yield_state > 0)
        ? SUB_DL
        : WAIT_YIELD_SIGNAL;
}

static void gcmd_spnoop(struct hle_t* hle, uint32_t w1, uint32_t w2)
{
    HleWarnMessage(hle, "GCMD_SPNOOP: %08x %08x", w1, w2);
}

static void gcmd_dma(struct hle_t* hle, uint32_t w1, uint32_t w2)
{
    HleWarnMessage(hle, "GCMD_DMA: %08x %08x", w1, w2);
}

static void gcmd_setword(struct hle_t* hle, uint32_t w1, uint32_t w2)
{
    HleWarnMessage(hle, "GCMD_SETWORD: %08x %08x", w1, w2);
}

static void gcmd_transpose3x3(struct hle_t* hle, uint32_t w1, uint32_t w2)
{
    HleWarnMessage(hle, "GCMD_TRANSPOSE_3X3: %08x %08x", w1, w2);
}

static void gcmd_1258(struct hle_t* hle, uint32_t w1, uint32_t w2)
{
    HleWarnMessage(hle, "GCMD_1258: %08x %08x", w1, w2);
}

static void gcmd_12fc(struct hle_t* hle, uint32_t w1, uint32_t w2)
{
    HleWarnMessage(hle, "GCMD_12FC: %08x %08x", w1, w2);
}

static void gcmd_1444(struct hle_t* hle, uint32_t w1, uint32_t w2)
{
    HleWarnMessage(hle, "GCMD_1444: %08x %08x", w1, w2);
}

static void gcmd_1570(struct hle_t* hle, uint32_t w1, uint32_t w2)
{
    HleWarnMessage(hle, "GCMD_1570: %08x %08x", w1, w2);
}

static void gcmd_1610(struct hle_t* hle, uint32_t w1, uint32_t w2)
{
    HleWarnMessage(hle, "GCMD_1610: %08x %08x", w1, w2);

    task_state_at_rdp_dl_begin = task_state;
    task_state = RDP_DL;
    rdp_dl_ptr = (w2 & 0xffffff);
    s5 = 1;
}

static void gcmd_1664(struct hle_t* hle, uint32_t w1, uint32_t w2)
{
    HleWarnMessage(hle, "GCMD_1664: %08x %08x", w1, w2);

    if (s5 != 0)
    {
        flush_rdp_commands_buffer(hle);
    }

    task_state = task_state_at_rdp_dl_begin;
}

static void gcmd_16f8(struct hle_t* hle, uint32_t w1, uint32_t w2)
{
    HleWarnMessage(hle, "GCMD_16F8: %08x %08x", w1, w2);
}

static void gcmd_1704(struct hle_t* hle, uint32_t w1, uint32_t w2)
{
    HleWarnMessage(hle, "GCMD_1704: %08x %08x", w1, w2);
}

static void gcmd_1720(struct hle_t* hle, uint32_t w1, uint32_t w2)
{
    HleWarnMessage(hle, "GCMD_1720: %08x %08x", w1, w2);
}

static void gcmd_1758(struct hle_t* hle, uint32_t w1, uint32_t w2)
{
    HleWarnMessage(hle, "GCMD_1758: %08x %08x", w1, w2);
}

static void gcmd_1790(struct hle_t* hle, uint32_t w1, uint32_t w2)
{
    HleWarnMessage(hle, "GCMD_1790: %08x %08x", w1, w2);
}

static void gcmd_17a4(struct hle_t* hle, uint32_t w1, uint32_t w2)
{
    HleWarnMessage(hle, "GCMD_17A4: %08x %08x", w1, w2);
}

static void gcmd_rdpcmd(struct hle_t* hle, uint32_t w1, uint32_t w2)
{
    HleWarnMessage(hle, "GCMD_RDPCMD: %08x %08x", w1, w2);
}

static void gcmd_17ec(struct hle_t* hle, uint32_t w1, uint32_t w2)
{
    HleWarnMessage(hle, "GCMD_17EC: %08x %08x", w1, w2);
}

static void gcmd_17f4(struct hle_t* hle, uint32_t w1, uint32_t w2)
{
    HleWarnMessage(hle, "GCMD_17F4: %08x %08x", w1, w2);
}

static void gcmd_1bcc(struct hle_t* hle, uint32_t w1, uint32_t w2)
{
    HleWarnMessage(hle, "GCMD_1BCC: %08x %08x", w1, w2);
}

static void gcmd_1bf0(struct hle_t* hle, uint32_t w1, uint32_t w2)
{
    HleWarnMessage(hle, "GCMD_1BF0: %08x %08x", w1, w2);
}

static void gcmd_1cd4(struct hle_t* hle, uint32_t w1, uint32_t w2)
{
    HleWarnMessage(hle, "GCMD_1CD4: %08x %08x", w1, w2);
}

static void gcmd_1cf0(struct hle_t* hle, uint32_t w1, uint32_t w2)
{
    HleWarnMessage(hle, "GCMD_1CF0: %08x %08x", w1, w2);
}

static void gcmd_1d20(struct hle_t* hle, uint32_t w1, uint32_t w2)
{
    HleWarnMessage(hle, "GCMD_1D20: %08x %08x", w1, w2);
}

static void gcmd_1fbc(struct hle_t* hle, uint32_t w1, uint32_t w2)
{
    HleWarnMessage(hle, "GCMD_1FBC: %08x %08x", w1, w2);
}


