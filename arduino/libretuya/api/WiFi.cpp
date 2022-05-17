/* Copyright (c) Kuba Szczodrzyński 2022-05-17. */

#include "WiFi.h"

std::vector<EventHandler> IWiFiGenericClass::handlers;

uint16_t IWiFiGenericClass::onEvent(EventCb callback, EventId eventId) {
	if (!callback)
		return 0;
	EventHandler handler;
	handler.cb		= callback;
	handler.eventId = eventId;
	handlers.push_back(handler);
	return handler.id;
}

uint16_t IWiFiGenericClass::onEvent(EventFuncCb callback, EventId eventId) {
	if (!callback)
		return 0;
	EventHandler handler;
	handler.fcb		= callback;
	handler.eventId = eventId;
	handlers.push_back(handler);
	return handler.id;
}

uint16_t IWiFiGenericClass::onEvent(EventSysCb callback, EventId eventId) {
	if (!callback)
		return 0;
	EventHandler handler;
	handler.scb		= callback;
	handler.eventId = eventId;
	handlers.push_back(handler);
	return handler.id;
}

void IWiFiGenericClass::removeEvent(EventCb callback, EventId eventId) {
	if (!callback)
		return;
	for (uint16_t i = 0; i < handlers.size(); i++) {
		EventHandler handler = handlers[i];
		if (handler.cb == callback && handler.eventId == eventId) {
			handlers.erase(handlers.begin() + i);
		}
	}
}

void IWiFiGenericClass::removeEvent(EventSysCb callback, EventId eventId) {
	if (!callback)
		return;
	for (uint16_t i = 0; i < handlers.size(); i++) {
		EventHandler handler = handlers[i];
		if (handler.scb == callback && handler.eventId == eventId) {
			handlers.erase(handlers.begin() + i);
		}
	}
}

void IWiFiGenericClass::removeEvent(uint16_t id) {
	for (uint16_t i = 0; i < handlers.size(); i++) {
		EventHandler handler = handlers[i];
		if (handler.id == id) {
			handlers.erase(handlers.begin() + i);
		}
	}
}

void IWiFiGenericClass::postEvent(EventId eventId, EventInfo eventInfo) {
	for (auto handler : handlers) {
		if (handler.eventId != ARDUINO_EVENT_MAX && handler.eventId != eventId)
			continue;
		if (handler.cb) {
			handler.cb(eventId);
		} else if (handler.fcb) {
			handler.fcb(eventId, eventInfo);
		} else if (handler.scb) {
			Event_t event = {
				.event_id	= eventId,
				.event_info = eventInfo,
			};
			handler.scb(&event);
		}
	}
}
