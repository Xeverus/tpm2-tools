//**********************************************************************;
// Copyright (c) 2019, Sebastien LE STUM
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution.
//
// 3. Neither the name of Intel Corporation nor the names of its contributors
// may be used to endorse or promote products derived from this software without
// specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
// THE POSSIBILITY OF SUCH DAMAGE.
//**********************************************************************;

#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <tss2/tss2_esys.h>

#include "log.h"
#include "tpm2_options.h"
#include "tpm2_tool.h"
#include "tpm2_alg_util.h"

typedef struct tpm_gettestresult_ctx tpm_gettestresult_ctx;

struct tpm_gettestresult_ctx {
    TPM2B_MAX_BUFFER*    output;
    TPM2_RC              status;
};

static tpm_gettestresult_ctx ctx;

static int tpm_gettestresult(ESYS_CONTEXT *ectx) {
    TSS2_RC rval = Esys_GetTestResult(ectx, ESYS_TR_NONE, ESYS_TR_NONE, ESYS_TR_NONE, &(ctx.output), &(ctx.status));
    if (rval != TSS2_RC_SUCCESS) {
        LOG_PERR(Esys_SelfTest, rval);
        return 3;
    }
    int retcode;

    tpm2_tool_output("status: ");
    print_yaml_indent(1);
    if(ctx.status){
        if((ctx.status & TPM2_RC_TESTING) == TPM2_RC_TESTING) {
            tpm2_tool_output("testing");
            retcode = 2;
        } else {
            tpm2_tool_output("failed");
            retcode = 1;
        }
    } else {
        tpm2_tool_output("success");
        retcode = 0;
    }

    if(ctx.output->size > 0){
        tpm2_tool_output("\ndata: ");
        print_yaml_indent(1);
        tpm2_util_hexdump(ctx.output->buffer, ctx.output->size);
    }
    tpm2_tool_output("\n");

    free(ctx.output);

    return retcode;
}

static bool on_arg(int argc, char **argv){
    UNUSED(argv);
    if (argc > 0) {
        LOG_ERR("No argument expected, got: %d", argc);
        return false;
    }

    return true;
}

bool tpm2_tool_onstart(tpm2_options **opts) {
    *opts = tpm2_options_new(NULL, 0, NULL, NULL, on_arg, 0);

    return *opts != NULL;
}

int tpm2_tool_onrun(ESYS_CONTEXT *ectx, tpm2_option_flags flags) {
    UNUSED(flags);
    return tpm_gettestresult(ectx);
}

