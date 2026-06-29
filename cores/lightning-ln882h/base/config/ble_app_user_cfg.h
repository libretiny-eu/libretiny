#pragma once

// Default BLE application user configuration.
// All settings here are defaults; ln_ble_app_default_cfg.h guards every value
// with #if !defined, so applications that need different settings can prepend
// their own include path and this file will be shadowed by their version.

// Observer / Central role - no advertising, scanning only.
#define BLE_DEFAULT_ROLE	BLE_ROLE_CENTRAL
#define BLE_CONFIG_AUTO_ADV (0)

// Override the SDK default which uses a GNU compound expression ({...}) that
// breaks when expanded inside a brace-initialiser: ln_bd_addr_t addr = {MACRO}.
// Use a plain brace-list instead - the NIC bytes are a placeholder; applications
// that need a real MAC must override this file via a higher-priority include path.
#define BLE_DEFAULT_PUBLIC_ADDR {0x00, 0x50, 0xC2, 0x00, 0x00, 0x00}
