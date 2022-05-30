/* Copyright (c) Kuba Szczodrzyński 2022-05-29. */

#include "uf2priv.h"

uf2_err_t uf2_binpatch(uint8_t *data, const uint8_t *binpatch, uint8_t binpatch_len) {
	const uint8_t *binpatch_end = binpatch + binpatch_len;
	// +2 to make sure opcode and length is present
	while ((binpatch + 2) < binpatch_end) {
		uf2_opcode_t opcode = binpatch[0];
		uint8_t len			= binpatch[1];
		switch (opcode) {
			case UF2_OPC_DIFF32:
				uf2_binpatch_diff32(data, binpatch + 1);
				break;
		}
		// advance by opcode + length + data
		binpatch += len + 2;
	}
	return UF2_ERR_OK;
}

void uf2_binpatch_diff32(uint8_t *data, const uint8_t *patch) {
	uint8_t num_offs = patch[0] - 4;			   // read offset count
	uint32_t diff	 = *((uint32_t *)(patch + 1)); // read diff value
	patch += 5;									   // skip num_offs and diff value
	for (uint8_t i = 0; i < num_offs; i++) {
		// patch the data
		uint8_t offs	= patch[i];
		uint32_t *value = (uint32_t *)(data + offs);
		*(value) += diff;
	}
}
