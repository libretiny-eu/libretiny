#include "Arduino.h"
#include <sockets.h> 
#include <lwip/netif.h>
#include <mbedtls/ssl.h>
#include <mbedtls/net_sockets.h>
#include <mbedtls/error.h>
#include <mbedtls/debug.h>
#include "ard_ssl.h"

#define ARDUINO_MBEDTLS_DEBUG_LEVEL     0   // Set to 0 to disable debug messsages, 5 to enable all debug messages

static unsigned int ard_ssl_arc4random(void)
{
    unsigned int res = xTaskGetTickCount();
    static unsigned int seed = 0xDEADB00B;

    seed = ((seed & 0x007F00FF) << 7) ^ 
        ((seed & 0x0F80FF00) >> 8) ^ // be sure to stir those low bits
        (res << 13) ^ (res >> 9);    // using the clock too!

    return seed;
}

static void get_random_bytes(void *buf, size_t len)
{
    unsigned int ranbuf;
    unsigned int *lp;
    int i, count;
    count = len / sizeof(unsigned int);
    lp = (unsigned int *) buf;

    for (i = 0; i < count; i ++) {
        lp[i] = ard_ssl_arc4random();
        len -= sizeof(unsigned int);
    }

    if (len > 0) {
        ranbuf = ard_ssl_arc4random();
        memcpy(&lp[i], &ranbuf, len);
    }
}

static int my_random(void *p_rng, unsigned char *output, size_t output_len)
{
    p_rng = p_rng;
    get_random_bytes(output, output_len);
    return 0;
}

static int my_verify(void *data, mbedtls_x509_crt *crt, int depth, uint32_t *flags) 
{
    char buf[1024];
    ((void)data);

    mbedtls_x509_crt_info(buf, (sizeof(buf) - 1), "", crt);

    if(ARDUINO_MBEDTLS_DEBUG_LEVEL < 3)
        return(0);

    printf( "\nVerify requested for (Depth %d):\n", depth );
    printf( "%s", buf );

    if ((*flags) == 0)
        printf(" This certificate has no flags\n");
    else
    {
        mbedtls_x509_crt_verify_info(buf, sizeof( buf ), " ! ", *flags);
        printf("%s\n", buf);
    }

    return(0);
}

static void* my_calloc(size_t nelements, size_t elementSize)
{
	size_t size;
	void *ptr = NULL;

	size = nelements * elementSize;
	ptr = pvPortMalloc(size);

	if(ptr)
		memset(ptr, 0, size);

	return ptr;
}

static void my_debug(void *ctx, int level, const char *file, int line, const char *str )
{
    const char *p, *basename;

    ctx = ctx;     // Remove unused parameter warning
    // Extract basename from file
    for( p = basename = file; *p != '\0'; p++ )
        if( *p == '/' || *p == '\\' )
            basename = p + 1;

    printf("%s:%04d: |%d| %s", basename, line, level, str );
}

int start_ssl_client(sslclient_context *ssl_client, uint32_t ipAddress, uint32_t port, unsigned char* rootCABuff, unsigned char* cli_cert, unsigned char* cli_key, unsigned char* pskIdent, unsigned char* psKey, char* SNI_hostname)
{
    int ret = 0;
    //int timeout;
    int enable = 1;
    int keep_idle = 30;
    mbedtls_x509_crt* cacert = NULL;
    mbedtls_x509_crt* _cli_crt = NULL;
    mbedtls_pk_context* _clikey_rsa = NULL;

    do {
        ssl_client->socket = -1;
        ssl_client->socket = lwip_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (ssl_client->socket < 0) {
            printf("ERROR: opening socket failed! \r\n");
            ret = -1;
            break;
        }

        struct sockaddr_in serv_addr;
        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = ipAddress;
        serv_addr.sin_port = htons(port);

        lwip_setsockopt(ssl_client->socket, SOL_SOCKET, SO_KEEPALIVE, &enable, sizeof(enable));
        lwip_setsockopt(ssl_client->socket, IPPROTO_TCP, TCP_KEEPIDLE, &keep_idle, sizeof(keep_idle));
        if (lwip_connect(ssl_client->socket, ((struct sockaddr *)&serv_addr), sizeof(serv_addr)) < 0) {
            lwip_close(ssl_client->socket);
            printf("ERROR: Connect to Server failed! \r\n");
            ret = -1;
            break;
        } else {
        /*/
        if (lwip_connect(ssl_client->socket, ((struct sockaddr *)&serv_addr), sizeof(serv_addr)) == 0) {
            timeout = ssl_client->recvTimeout;
            lwip_setsockopt(ssl_client->socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
            timeout = 30000;
            lwip_setsockopt(ssl_client->socket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
            lwip_setsockopt(ssl_client->socket, IPPROTO_TCP, TCP_NODELAY, &enable, sizeof(enable));
            lwip_setsockopt(ssl_client->socket, SOL_SOCKET, SO_KEEPALIVE, &enable, sizeof(enable));
        } else {
            printf("ERROR: Connect to Server failed!\r\n");
            ret = -1;
            break;
        }//*/

            mbedtls_platform_set_calloc_free(my_calloc,vPortFree);

            ssl_client->ssl = (mbedtls_ssl_context *)malloc(sizeof(mbedtls_ssl_context));
            ssl_client->conf = (mbedtls_ssl_config *)malloc(sizeof(mbedtls_ssl_config));
            if (( ssl_client->ssl == NULL )||( ssl_client->conf == NULL )) {
                printf("ERROR: malloc ssl failed! \r\n");
                ret = -1;
                break;
            }

            mbedtls_ssl_init(ssl_client->ssl);
            mbedtls_ssl_config_init(ssl_client->conf);

            if (ARDUINO_MBEDTLS_DEBUG_LEVEL > 0) {
                mbedtls_ssl_conf_verify(ssl_client->conf, my_verify, NULL);
                mbedtls_ssl_conf_dbg(ssl_client->conf, my_debug, NULL);
                mbedtls_debug_set_threshold(ARDUINO_MBEDTLS_DEBUG_LEVEL);
            }

            if((mbedtls_ssl_config_defaults(ssl_client->conf, MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT)) != 0) {
                printf("ERROR: mbedtls ssl config defaults failed! \r\n");
                ret = -1;
                break;
            }
            
            mbedtls_ssl_conf_rng(ssl_client->conf, my_random, NULL);

            if (rootCABuff != NULL) {
                // Configure mbedTLS to use certificate authentication method
                cacert = (mbedtls_x509_crt *) mbedtls_calloc( sizeof(mbedtls_x509_crt), 1);
                mbedtls_x509_crt_init(cacert);
                if (mbedtls_x509_crt_parse(cacert, rootCABuff, (strlen((char*)rootCABuff)) + 1) != 0) {
                    printf("ERROR: mbedtls x509 crt parse failed! \r\n");
                    ret = -1;
                    break;
                }
                mbedtls_ssl_conf_ca_chain(ssl_client->conf, cacert, NULL);
                mbedtls_ssl_conf_authmode(ssl_client->conf, MBEDTLS_SSL_VERIFY_REQUIRED);
            } else if (pskIdent != NULL && psKey != NULL) {
                // Configure mbedTLS to use PSK authentication method
                // Check for max length and even number of chars
                uint16_t pskey_char_len = strlen((char*)psKey);
                if ( ((pskey_char_len % 2) != 0) || (pskey_char_len > 2*MBEDTLS_PSK_MAX_LEN) ) {
                    printf("ERROR: TLS PSK not in valid hex format or too long \n");
                    return -1;
                }
                uint16_t psk_len = pskey_char_len/2;
                unsigned char psk[MBEDTLS_PSK_MAX_LEN];
                // Convert PSK from hexadecimal chars to binary
                int i;
                for (i = 0; i < pskey_char_len; i = i + 2) {
                    char c = psKey[i];
                    // Convert first 4 bits
                    if (c >= '0' && c <= '9') {
                        c = c - '0';
                    } else if (c >= 'A' && c <= 'F') {
                        c = c - 'A' + 10;
                    } else if (c >= 'a' && c <= 'f') {
                        c = c - 'a' + 10;
                    } else {
                        printf("ERROR: TLS PSK not in valid hex format \n");
                        return -1;
                    }
                    psk[i/2] = c << 4;
                    c = psKey[i+1];
                    // Convert next 4 bits
                    if (c >= '0' && c <= '9') {
                        c = c - '0';
                    } else if (c >= 'A' && c <= 'F') {
                        c = c - 'A' + 10;
                    } else if (c >= 'a' && c <= 'f') {
                        c = c - 'a' + 10;
                    } else {
                        printf("ERROR: TLS PSK not in valid hex format \r\n");
                        return -1;
                    }
                    psk[i/2] |= c;
                }
                if (mbedtls_ssl_conf_psk(ssl_client->conf, psk, psk_len, pskIdent, strlen((char*)pskIdent)) != 0) {
                    printf("ERROR: mbedtls conf psk failed! \r\n");
                }
            } else {
                mbedtls_ssl_conf_authmode(ssl_client->conf, MBEDTLS_SSL_VERIFY_NONE);
            }

            if ((cli_cert != NULL) && (cli_key != NULL)) {
                _cli_crt = (mbedtls_x509_crt *) mbedtls_calloc( sizeof(mbedtls_x509_crt), 1);
                if (_cli_crt == NULL) {
                    printf("ERROR: malloc client_crt failed! \r\n");
                    ret = -1;
                    break;
                }
                mbedtls_x509_crt_init(_cli_crt);

                _clikey_rsa = (mbedtls_pk_context *) mbedtls_calloc( sizeof(mbedtls_pk_context), 1);
                if (_clikey_rsa == NULL) {
                    printf("ERROR: malloc client_rsa failed! \r\n");
                    ret = -1;
                    break;
                }
                mbedtls_pk_init(_clikey_rsa);

                if (mbedtls_x509_crt_parse(_cli_crt, cli_cert, strlen((char*)cli_cert)+1) != 0) {
                    printf("ERROR: mbedtls x509 parse client_crt failed! \r\n");
                    ret = -1;
                    break;
                }

                if (mbedtls_pk_parse_key(_clikey_rsa, cli_key, strlen((char*)cli_key)+1, NULL, 0) != 0) {
                    printf("ERROR: mbedtls x509 parse client_rsa failed! \r\n");
                    ret = -1;
                    break;
                }
                mbedtls_ssl_conf_own_cert(ssl_client->conf, _cli_crt, _clikey_rsa);
            }

            if((mbedtls_ssl_setup(ssl_client->ssl, ssl_client->conf)) != 0) {
                printf("ERROR: mbedtls ssl setup failed!\r\n");
                ret = -1;
                break;
            }
            mbedtls_ssl_set_bio(ssl_client->ssl, &ssl_client->socket, mbedtls_net_send, mbedtls_net_recv, NULL);

            mbedtls_ssl_set_hostname(ssl_client->ssl, SNI_hostname);

            ret = mbedtls_ssl_handshake(ssl_client->ssl);
            if (ret < 0) {
                printf("ERROR: mbedtls ssl handshake failed: -0x%04X \r\n", -ret);
                ret = -1;
            } else { 
                if (ARDUINO_MBEDTLS_DEBUG_LEVEL > 0) {
                    printf("mbedTLS SSL handshake success \r\n");
                }
            }
            //mbedtls_debug_set_threshold(ARDUINO_MBEDTLS_DEBUG_LEVEL);
        }
    } while (0);

    if (_clikey_rsa) {
        mbedtls_pk_free(_clikey_rsa);
        mbedtls_free(_clikey_rsa);
        _clikey_rsa = NULL;
    }

    if (_cli_crt) {
        mbedtls_x509_crt_free(_cli_crt);
        mbedtls_free(_cli_crt);
        _cli_crt = NULL;
    }

    if (cacert) {
        mbedtls_x509_crt_free(cacert);
        mbedtls_free(cacert);
        cacert = NULL;
    }

    if (ret < 0) {
        if (ssl_client->socket >= 0) {
            mbedtls_net_free((mbedtls_net_context *)&ssl_client->socket);
            ssl_client->socket = -1;
        }

        if (ssl_client->ssl != NULL) {
            mbedtls_ssl_free(ssl_client->ssl);
            free(ssl_client->ssl);
            ssl_client->ssl = NULL;
        }
        if (ssl_client->conf != NULL) {
            mbedtls_ssl_config_free(ssl_client->conf);
            free(ssl_client->conf);
            ssl_client->conf = NULL;
        }
    }

    return ssl_client->socket;
}

void stop_ssl_socket(sslclient_context *ssl_client)
{
    lwip_shutdown(ssl_client->socket, SHUT_RDWR);
    lwip_close(ssl_client->socket);
    //mbedtls_net_free((mbedtls_net_context *)&ssl_client->socket);
    ssl_client->socket = -1;

    if (ssl_client->ssl != NULL) {
        mbedtls_ssl_free(ssl_client->ssl);
        free(ssl_client->ssl);
        ssl_client->ssl = NULL;
    }
    if (ssl_client->conf != NULL) {
        mbedtls_ssl_config_free(ssl_client->conf);
        free(ssl_client->conf);
        ssl_client->conf = NULL;
    }
}

int send_ssl_data(sslclient_context *ssl_client, const uint8_t *data, uint16_t len)
{
    int ret = -1;

    if (ssl_client->ssl != NULL) {
        ret = mbedtls_ssl_write(ssl_client->ssl, data, len);
    }

    return ret;
}

int get_ssl_receive(sslclient_context *ssl_client, uint8_t* data, int length, int flag)
{
    int ret = 0;
    uint8_t has_backup_recvtimeout = 0;
    int backup_recv_timeout, recv_timeout;
    socklen_t len;

    if (ssl_client->ssl == NULL) {
        return 0;
    }

    if (flag & 0x01) {
        // peek for 10ms
        ret = lwip_getsockopt(ssl_client->socket, SOL_SOCKET, SO_RCVTIMEO, &backup_recv_timeout, &len);
        if (ret >= 0) {
            recv_timeout = 100;
            ret = lwip_setsockopt(ssl_client->socket, SOL_SOCKET, SO_RCVTIMEO, &recv_timeout, sizeof(recv_timeout));
            if (ret >= 0) {
                has_backup_recvtimeout = 1;
            }
        }
    }

    memset(data, 0, length);
    ret = mbedtls_ssl_read(ssl_client->ssl, data, length);

    if ((flag & 0x01) && (has_backup_recvtimeout == 1)) {
        // restore receiving timeout
        lwip_setsockopt(ssl_client->socket, SOL_SOCKET, SO_RCVTIMEO, &backup_recv_timeout, sizeof(recv_timeout));
    }

    return ret;
}

int get_ssl_sock_errno(sslclient_context *ssl_client) {
    int so_error;
    socklen_t len = sizeof(so_error);
    lwip_getsockopt(ssl_client->socket, SOL_SOCKET, SO_ERROR, &so_error, &len);
    return so_error;
}

int get_ssl_bytes_avail(sslclient_context *ssl_client) {
    if (ssl_client->ssl != NULL) {
        return mbedtls_ssl_get_bytes_avail(ssl_client->ssl);
    } else {
        return 0;
    }
}
