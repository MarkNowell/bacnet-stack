/**************************************************************************
 *
 * Copyright (C) 2008 Steve Karg <skarg@users.sourceforge.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *********************************************************************/
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "bacnet/config.h"
#include "bacnet/bacdef.h"
#include "bacnet/bacdcode.h"
#include "bacnet/apdu.h"
#include "bacnet/npdu.h"
#include "bacnet/abort.h"
#include "bacnet/cov.h"
#include "bacnet/bactext.h"
#include "bacnet/basic/services.h"
#include "bacnet/basic/tsm/tsm.h"

/** @file h_ucov.c  Handles Unconfirmed COV Notifications. */
#if PRINT_ENABLED
#include <stdio.h>
#define PRINTF(...) fprintf(stderr, __VA_ARGS__)
#else
#define PRINTF(...)
#endif

#ifndef MAX_COV_PROPERTIES
#define MAX_COV_PROPERTIES 2
#endif

/* COV notification callbacks list */
static BACNET_COV_NOTIFICATION Unconfirmed_COV_Notification_Head;

/**
 * @brief call the COV notification callbacks
 * @param cov_data - data decoded from the COV notification
 */
static void handler_ucov_notification_callback(BACNET_COV_DATA *cov_data)
{
    BACNET_COV_NOTIFICATION *head;

    head = &Unconfirmed_COV_Notification_Head;
    do {
        if (head->callback) {
            head->callback(cov_data);
        }
        head = head->next;
    } while (head);
}

/**
 * @brief Add a Confirmed COV notification callback
 * @param cb - COV notification callback to be added
 */
void handler_ucov_notification_add(BACNET_COV_NOTIFICATION *cb)
{
    BACNET_COV_NOTIFICATION *head;

    head = &Unconfirmed_COV_Notification_Head;
    do {
        if (head->next == cb) {
            /* already here! */
            break;
        } else if (!head->next) {
            /* first available free node */
            head->next = cb;
            break;
        }
        head = head->next;
    } while (head);
}

/*  */
/** Handler for an Unconfirmed COV Notification.
 * @ingroup DSCOV
 * Decodes the received list of Properties to update,
 * and print them out with the subscription information.
 * @note Nothing is specified in BACnet about what to do with the
 *       information received from Unconfirmed COV Notifications.
 *
 * @param service_request [in] The contents of the service request.
 * @param service_len [in] The length of the service_request.
 * @param src [in] BACNET_ADDRESS of the source of the message (unused)
 */
void handler_ucov_notification(
    uint8_t *service_request, uint16_t service_len, BACNET_ADDRESS *src)
{
    BACNET_COV_DATA cov_data;
    BACNET_PROPERTY_VALUE property_value[MAX_COV_PROPERTIES];
    BACNET_PROPERTY_VALUE *pProperty_value = NULL;
    int len = 0;

    /* src not needed for this application */
    (void)src;
    /* create linked list to store data if more
       than one property value is expected */
    bacapp_property_value_list_init(&property_value[0], MAX_COV_PROPERTIES);
    cov_data.listOfValues = &property_value[0];
    PRINTF("UCOV: Received Notification!\n");
    /* decode the service request only */
    len = cov_notify_decode_service_request(
        service_request, service_len, &cov_data);
    if (len > 0) {
        handler_ucov_notification_callback(&cov_data);
        PRINTF("UCOV: PID=%u ", cov_data.subscriberProcessIdentifier);
        PRINTF("instance=%u ", cov_data.initiatingDeviceIdentifier);
        PRINTF("%s %u ",
            bactext_object_type_name(cov_data.monitoredObjectIdentifier.type),
            cov_data.monitoredObjectIdentifier.instance);
        PRINTF("time remaining=%u seconds ", cov_data.timeRemaining);
        PRINTF("\n");
        pProperty_value = &property_value[0];
        while (pProperty_value) {
            PRINTF("UCOV: ");
            if (pProperty_value->propertyIdentifier < 512) {
                PRINTF("%s ",
                    bactext_property_name(pProperty_value->propertyIdentifier));
            } else {
                PRINTF("proprietary %u ", pProperty_value->propertyIdentifier);
            }
            if (pProperty_value->propertyArrayIndex != BACNET_ARRAY_ALL) {
                PRINTF("%u ", pProperty_value->propertyArrayIndex);
            }
            PRINTF("\n");
            pProperty_value = pProperty_value->next;
        }
    } else {
        PRINTF("UCOV: Unable to decode service request!\n");
    }
}
