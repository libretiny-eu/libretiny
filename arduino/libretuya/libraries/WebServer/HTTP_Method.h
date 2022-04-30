#pragma once

/* Request Methods */
#define HTTP_METHOD_MAP(XX)                                                                                            \
	XX(0, DELETE, DELETE)                                                                                              \
	XX(1, GET, GET)                                                                                                    \
	XX(2, HEAD, HEAD)                                                                                                  \
	XX(3, POST, POST)                                                                                                  \
	XX(4, PUT, PUT)                                                                                                    \
	/* pathological */                                                                                                 \
	XX(5, CONNECT, CONNECT)                                                                                            \
	XX(6, OPTIONS, OPTIONS)                                                                                            \
	XX(7, TRACE, TRACE)                                                                                                \
	/* WebDAV */                                                                                                       \
	XX(8, COPY, COPY)                                                                                                  \
	XX(9, LOCK, LOCK)                                                                                                  \
	XX(10, MKCOL, MKCOL)                                                                                               \
	XX(11, MOVE, MOVE)                                                                                                 \
	XX(12, PROPFIND, PROPFIND)                                                                                         \
	XX(13, PROPPATCH, PROPPATCH)                                                                                       \
	XX(14, SEARCH, SEARCH)                                                                                             \
	XX(15, UNLOCK, UNLOCK)                                                                                             \
	XX(16, BIND, BIND)                                                                                                 \
	XX(17, REBIND, REBIND)                                                                                             \
	XX(18, UNBIND, UNBIND)                                                                                             \
	XX(19, ACL, ACL)                                                                                                   \
	/* subversion */                                                                                                   \
	XX(20, REPORT, REPORT)                                                                                             \
	XX(21, MKACTIVITY, MKACTIVITY)                                                                                     \
	XX(22, CHECKOUT, CHECKOUT)                                                                                         \
	XX(23, MERGE, MERGE)                                                                                               \
	/* upnp */                                                                                                         \
	XX(24, MSEARCH, M - SEARCH)                                                                                        \
	XX(25, NOTIFY, NOTIFY)                                                                                             \
	XX(26, SUBSCRIBE, SUBSCRIBE)                                                                                       \
	XX(27, UNSUBSCRIBE, UNSUBSCRIBE)                                                                                   \
	/* RFC-5789 */                                                                                                     \
	XX(28, PATCH, PATCH)                                                                                               \
	XX(29, PURGE, PURGE)                                                                                               \
	/* CalDAV */                                                                                                       \
	XX(30, MKCALENDAR, MKCALENDAR)                                                                                     \
	/* RFC-2068, section 19.6.1.2 */                                                                                   \
	XX(31, LINK, LINK)                                                                                                 \
	XX(32, UNLINK, UNLINK)                                                                                             \
	/* icecast */                                                                                                      \
	XX(33, SOURCE, SOURCE)

enum http_method {

#define XX(num, name, string) HTTP_##name = num,
	HTTP_METHOD_MAP(XX)
#undef XX
};

typedef enum http_method HTTPMethod;
#define HTTP_ANY (HTTPMethod)(255)
