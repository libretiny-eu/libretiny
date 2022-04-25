#pragma once

#include <Arduino.h>
#include <api/WiFi.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <FreeRTOS.h>
#include <semphr.h>
#include <wifi_structures.h>

#ifdef __cplusplus
} // extern "C"
#endif

class WiFiClass : public IWiFiClass,
				  public IWiFiGenericClass,
				  public IWiFiSTAClass,
				  public IWiFiScanClass,
				  public IWiFiAPClass {
  private:
	static WiFiAuthMode securityTypeToAuthMode(uint8_t type);

	bool _initialized;
	bool _sleep;

	bool _scanning = false;
	SemaphoreHandle_t _scanSem;
	uint8_t _netCount	   = 0;
	char **_netSsid		   = NULL;
	WiFiAuthMode *_netEncr = NULL;
	int32_t *_netRssi	   = NULL;
	rtw_mac_t *_netBssid   = NULL;
	int32_t *_netChannel   = NULL;

	static rtw_result_t scanHandler(rtw_scan_handler_result_t *result);

  public:
	// IWiFiClass
	WiFiClass();
	~WiFiClass();
	void printDiag(Print &dest);

  public:
	// IWiFiGenericClass
	int32_t channel(void);

	bool mode(WiFiMode mode) override;
	WiFiMode getMode();
	WiFiStatus status();

	bool enableSTA(bool enable);
	bool enableAP(bool enable);

	bool setSleep(bool enable);
	bool getSleep();

	bool setTxPower(int power);
	int getTxPower();

	IPAddress hostByName(const char *hostname);

	static IPAddress calculateNetworkID(IPAddress ip, IPAddress subnet);
	static IPAddress calculateBroadcast(IPAddress ip, IPAddress subnet);
	static uint8_t calculateSubnetCIDR(IPAddress subnetMask);
	static String macToString(uint8_t *mac);

  public:
	// IWiFiSTAClass
	WiFiStatus begin(
		const char *ssid,
		const char *passphrase = NULL,
		int32_t channel		   = 0,
		const uint8_t *bssid   = NULL,
		bool connect		   = true
	) override;
	WiFiStatus
	begin(char *ssid, char *passphrase = NULL, int32_t channel = 0, const uint8_t *bssid = NULL, bool connect = true);

	bool config(
		IPAddress localIP,
		IPAddress gateway,
		IPAddress subnet,
		IPAddress dns1 = (uint32_t)0x00000000,
		IPAddress dns2 = (uint32_t)0x00000000
	);

	inline bool reconnect() {
		return reconnect(NULL);
	}

	bool reconnect(const uint8_t *bssid = NULL);
	bool disconnect(bool wifiOff = false);

	bool isConnected() override;

	bool setAutoReconnect(bool autoReconnect);
	bool getAutoReconnect();

	WiFiStatus waitForConnectResult(unsigned long timeout);

	IPAddress localIP();
	uint8_t *macAddress(uint8_t *mac);
	String macAddress();
	IPAddress subnetMask();
	IPAddress gatewayIP();
	IPAddress dnsIP(uint8_t dns_no);
	IPAddress broadcastIP();
	IPAddress networkID();
	uint8_t subnetCIDR();
	bool enableIpV6();
	IPv6Address localIPv6();
	const char *getHostname();
	bool setHostname(const char *hostname);
	bool setMacAddress(const uint8_t *mac);

	const String SSID();
	const String psk();
	uint8_t *BSSID();
	String BSSIDstr();
	int8_t RSSI();
	WiFiAuthMode getEncryption();

  public:
	// IWiFiScanClass
	int16_t scanNetworks(
		bool async				 = false,
		bool showHidden			 = false,
		bool passive			 = false,
		uint32_t maxMsPerChannel = 300,
		uint8_t channel			 = 0
	);
	bool getNetworkInfo(
		uint8_t networkItem,
		String &ssid,
		WiFiAuthMode &encryptionType,
		int32_t &RSSI,
		uint8_t *&BSSID,
		int32_t &channel
	);

	int16_t scanComplete();
	void scanDelete();

	String SSID(uint8_t networkItem);
	WiFiAuthMode encryptionType(uint8_t networkItem);
	int32_t RSSI(uint8_t networkItem);
	uint8_t *BSSID(uint8_t networkItem);
	String BSSIDstr(uint8_t networkItem);
	int32_t channel(uint8_t networkItem);

  public:
	// IWiFiAPClass
	bool softAP(
		const char *ssid, const char *passphrase = NULL, int channel = 1, bool ssidHidden = false, int maxClients = 4
	);
	bool softAPConfig(IPAddress localIP, IPAddress gateway, IPAddress subnet);
	bool softAPdisconnect(bool wifiOff = false);

	uint8_t softAPgetStationNum();

	IPAddress softAPIP();
	IPAddress softAPBroadcastIP();
	IPAddress softAPNetworkID();
	uint8_t softAPSubnetCIDR();
	bool softAPenableIpV6();
	IPv6Address softAPIPv6();
	const char *softAPgetHostname();
	bool softAPsetHostname(const char *hostname);
	uint8_t *softAPmacAddress(uint8_t *mac);
	String softAPmacAddress(void);
	const String softAPSSID(void);
};

extern WiFiClass WiFi;
