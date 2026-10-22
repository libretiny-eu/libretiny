
#include <vector>

#include "RBCXNvs.h"

namespace rb {

Nvs::Nvs(const char* name, const char* partiton)
    : m_dirty(false) {
    ESP_ERROR_CHECK(initFlash());
    ESP_ERROR_CHECK(
        nvs_open_from_partition(partiton, name, NVS_READWRITE, &m_handle));
}

Nvs::~Nvs() {
    if (m_dirty)
        commit();
    nvs_close(m_handle);
}

esp_err_t Nvs::initFlash() {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES
        || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    return err;
}

bool Nvs::existsInt(const char* key) {
    int value;
    return nvs_get_i32(m_handle, key, &value) == ESP_OK;
}

int Nvs::getInt(const char* key) {
    int value;
    ESP_ERROR_CHECK(nvs_get_i32(m_handle, key, &value));
    return value;
}

void Nvs::writeInt(const char* key, int value) {
    ESP_ERROR_CHECK(nvs_set_i32(m_handle, key, value));
    m_dirty = true;
}

bool Nvs::existsString(const char* key) {
    size_t len;
    return nvs_get_str(m_handle, key, NULL, &len) == ESP_OK;
}

std::string Nvs::getString(const char* key) {
    size_t len;
    ESP_ERROR_CHECK(nvs_get_str(m_handle, key, NULL, &len));

    std::vector<char> res(len, ' ');
    ESP_ERROR_CHECK(nvs_get_str(m_handle, key, res.data(), &len));
    return std::string(res.data());
}

void Nvs::writeString(const char* key, const std::string& value) {
    ESP_ERROR_CHECK(nvs_set_str(m_handle, key, value.c_str()));
    m_dirty = true;
}

void Nvs::commit() {
    nvs_commit(m_handle);
    m_dirty = false;
}

} // namespace rb
