# uf2ota library

uf2ota library allows to write a LibreTiny UF2 file to the flash, while parsing all the necessary tags. It manages the target partitions, compatibility checks, and works on top of the FAL provided by FlashDB.

## Usage example

```c
uint8_t target	   = 1;			 // target OTA scheme - 1 or 2
uint32_t family	   = F_RTL8710B; // chip's UF2 family ID
uf2_ota_t *ctx	   = uf2_ctx_init(target, family);
uf2_info_t *info   = uf2_info_init(); // optional, for getting firmware info
uf2_block_t *block = (uf2_block_t *)malloc(UF2_BLOCK_SIZE);
uf2_err_t err;

// ... // read the first header block (512 bytes) into *block

// check the block for validity
err = uf2_check_block(ctx, block);
if (err > UF2_ERR_IGNORE)
	// handle the error
	return;

// parse the header block
// note: if you don't need info, you can skip this step and call uf2_write() directly
err = uf2_parse_header(ctx, block, info);
if (err)
	// handle the error
	return;

while (/* have input data */) {

	// ... // read the next block into *block

	// check the block for validity
	err = uf2_check_block(ctx, block);
	if (err == UF2_ERR_IGNORE)
		// skip this block
		continue;
	if (err)
		// handle the error
		return;

	// write the block to flash
	err = uf2_write(ctx, block);
	if (err > UF2_ERR_IGNORE)
		// handle the error
		return;
}

// finish the update process

// ... // activate your new OTA partition

// cleanup
free(ctx);
free(block);
uf2_info_free(info);
```
