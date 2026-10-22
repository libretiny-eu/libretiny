/* Copyright (c) Kuba Szczodrzyński 2023-03-14. */

#include_next "hal_crypto.h"

#pragma once

// mbedTLS (ssl_tls.c) uses S1 and S2 as variable name
// rtl8710b_pinmux.h defines them as integer values
#if defined(CONFIG_SSL_RSA) && defined(RTL_HW_CRYPTO) && defined(S1)
#undef S1
#undef S2
#endif
