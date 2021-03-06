/*
 * Copyright (c) 2017 - 2018, Intel Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <inttypes.h>
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>

#include <tss2/tss2_sys.h>

#include "common.h"
#include "tpm2-struct-init.h"
#include "test-options.h"
#include "context-util.h"

/*
 * This test exercises the session tracking logic, specifically the ability
 * / feature that allows a session created and saved over one connection to
 * the tabrmd to be loaded from another connection.
 * This test creates an auth session using "connection0", saves it and then
 * loads it from "connection1" *before* "connection0" is closed.
 */
int
main ()
{
    TSS2_RC rc;
    TSS2_SYS_CONTEXT *sapi_context0, *sapi_context1;
    TPMI_SH_AUTH_SESSION  session_handle = 0, session_handle_load = 0;
    TPMS_CONTEXT          context = TPMS_CONTEXT_ZERO_INIT;
    test_opts_t opts = TEST_OPTS_DEFAULT_INIT;

    get_test_opts_from_env (&opts);
    if (sanity_check_test_opts (&opts) != 0)
        exit (1);

    sapi_context0 = sapi_init_from_opts (&opts);
    if (sapi_context0 == NULL) {
        g_error ("Failed to create SAPI context.");
    }
    /* create an auth session */
    rc = start_auth_session (sapi_context0, &session_handle);
    if (rc != TSS2_RC_SUCCESS) {
        g_error ("Tss2_Sys_StartAuthSession failed: 0x%" PRIxHANDLE, rc);
    }
    /* save context */
    rc = Tss2_Sys_ContextSave (sapi_context0, session_handle, &context);
    if (rc != TSS2_RC_SUCCESS) {
        g_error ("Tss2_Sys_ContextSave failed: 0x%" PRIxHANDLE, rc);
    }

    sapi_context1 = sapi_init_from_opts (&opts);
    if (sapi_context1 == NULL) {
        g_error ("Failed to create SAPI context.");
    }
    /* reload the session through new connection */
    rc = Tss2_Sys_ContextLoad (sapi_context1, &context, &session_handle_load);
    if (rc != TSS2_RC_SUCCESS) {
        g_error ("Tss2_Sys_ContextLoad failed: 0x%" PRIxHANDLE, rc);
    }
    if (session_handle_load == session_handle) {
        g_info ("session_handle == session_handle_load");
    } else {
        g_error ("session_handle != session_handle_load");
    }

    /* teardown sapi connection */
    sapi_teardown_full (sapi_context0);
    sapi_teardown_full (sapi_context1);

    return 0;
}
