/**
 * Morse KOB System/Server structures, enums, typedefs, facilities.
 *
 * Facilities used by the MKOB subsystems and structures, definitions, etc.
 * to comply with the Morse KOB Server and other standard clients.
 *
 * Copyright 2023 AESilky
 * SPDX-License-Identifier: MIT License
 *
*/
#include "mks.h"

#include <assert.h>
#include <stdbool.h>
#include <string.h>

#include "pico/mutex.h"

code_element_t mcode_long_break = (-32767);

static bool _initialized = false;
auto_init_mutex(mcs_mutex);

typedef struct _MCS_POOL_ENTRY_ {
    int index;
    bool free;
    mcode_seq_t mcode_seq;
} _mcs_pool_entry_t;

// Build out an MCode Sequence instance pool.
#define _MCODE_SEQ_POOL_SIZE 16
static code_element_t _codeseq_pool[_MCODE_SEQ_POOL_SIZE][MKS_CODESEQ_MAX_LEN + 1]; // Used in the mcode_seq
static _mcs_pool_entry_t _mcode_seq_pool[_MCODE_SEQ_POOL_SIZE];

mcode_seq_t* mcode_seq_alloc(mcode_source_t source, code_element_t* code_seq, int len) {
    mcode_seq_t* mcode_seq = NULL;
    // Find an empty one
    for (int j = 0; j < 2; j++) {
        for (int i = 0; i < _MCODE_SEQ_POOL_SIZE; i++) {
            uint32_t flags = save_and_disable_interrupts();
            mutex_enter_blocking(&mcs_mutex);
            register bool was_free = _mcode_seq_pool[i].free;
            _mcode_seq_pool[i].free = false;
            mutex_exit(&mcs_mutex);
            restore_interrupts(flags);
            if (was_free) {
                mcode_seq = &(_mcode_seq_pool[i].mcode_seq);
                break;
            }
        }
        if (mcode_seq) {
            break;
        }
    }
    if (!mcode_seq) {
        panic("MKS - No mcode_seq available for use.");
    }
    else {
        mcode_seq->len = (len <= MKS_CODESEQ_MAX_LEN ? len : MKS_CODESEQ_MAX_LEN);
        mcode_seq->source = source;
        memcpy(mcode_seq->code_seq, code_seq, mcode_seq->len * sizeof(code_element_t));
    }

    return (mcode_seq);
}

int mcode_seq_append(mcode_seq_t* mcode_seq, code_element_t* code_seq, int len) {
    int l = (mcode_seq->len + len <= MKS_CODESEQ_MAX_LEN ? len : MKS_CODESEQ_MAX_LEN - mcode_seq->len);
    memcpy(mcode_seq->code_seq + mcode_seq->len, code_seq, l * sizeof(code_element_t));
    mcode_seq->len += l;

    return (l);
}

mcode_seq_t* mcode_seq_copy(const mcode_seq_t* mcode_seq_src) {
    mcode_seq_t* mcode_seq = mcode_seq_alloc(mcode_seq_src->source, mcode_seq_src->code_seq, mcode_seq_src->len);

    return (mcode_seq);
}

void mcode_seq_free(mcode_seq_t* mcode_seq) {
    if (mcode_seq) {
        // Find the entry
        for (int i = 0; i < _MCODE_SEQ_POOL_SIZE; i++) {
            if (mcode_seq == &(_mcode_seq_pool[i].mcode_seq)) {
                mcode_seq->source = MCODE_SRC_UNKNOWN;
                mcode_seq->len = 0;
                _mcode_seq_pool[i].free = true;
                break;
            }
        }
    }
}

extern void mks_module_init() {
    assert(!_initialized);
    _initialized = true;

    // Initialize the mcode_seq pool
    for (int i = 0; i < _MCODE_SEQ_POOL_SIZE; i++) {
        _mcode_seq_pool[i].index = i;
        _mcode_seq_pool[i].mcode_seq.code_seq = _codeseq_pool[i];
        _mcode_seq_pool[i].mcode_seq.source = MCODE_SRC_UNKNOWN;
        _mcode_seq_pool[i].free = true;
    }
}

