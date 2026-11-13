/* AMB1 (RTL8711AM / Ameba-1) family lifecycle hook for the common UF2 OTA path.
 *
 * This binding turns a UF2 upload into an image2 staging transaction:
 *   begin()        validates the total UF2 size and the inactive-slot capacity;
 *   write_block()  stages one 256-byte main-flash UF2 data block;
 *   end()          verifies the staged readback and activates the candidate;
 *   abort()        abandons staging without ever publishing a signature.
 *
 * end() publishes the target signature and demotes the running signature, which
 * is what the reviewed AMB1 normal selector uses to choose the next image. It
 * does NOT reboot and does NOT call lt_ota_switch(); power-cycle/boot handoff is
 * a separate, explicit operation.
 *
 * The crypto provider is integrity-only (see image2_integrity.h): readback is
 * compared against the bytes received over the wire. It is NOT authentication.
 */
#ifndef AMB1_LT_OTA_FAMILY_AMB1_H
#define AMB1_LT_OTA_FAMILY_AMB1_H

#include <lt_ota.h>

#include "image2_flash_backend.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Weak, fail-closed flash seam. The default definition in
 * lt_ota_family_amb1.c returns NULL, so every upload is refused until a reviewed
 * integration supplies a strong definition returning a live sdkops object.
 * begin() copies the object; the caller must keep it valid for that call. */
const image2_flash_sdkops *lt_ota_family_amb1_sdkops(void);

/* Strong overrides of the weak common hooks. The authoritative lifecycle
 * contract is documented in <lt_ota.h>; these declarations keep the family
 * binding explicit for integrators and tests. */
lt_ota_family_result_t lt_ota_family_begin(lt_ota_ctx_t *ctx, size_t size);
lt_ota_family_result_t lt_ota_family_write_block(lt_ota_ctx_t *ctx, const uf2_block_t *block);
lt_ota_family_result_t lt_ota_family_end(lt_ota_ctx_t *ctx);
void lt_ota_family_abort(lt_ota_ctx_t *ctx);

#ifdef __cplusplus
}
#endif
#endif
