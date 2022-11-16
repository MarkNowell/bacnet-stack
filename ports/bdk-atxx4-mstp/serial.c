/**************************************************************************
*
* Copyright (C) 2009 Steve Karg <skarg@users.sourceforge.net>
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
*********************************************************************/
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "hardware.h"
#include "fifo.h"
#include "serial.h"

/* baud rate */
static uint32_t Baud_Rate = BAUD_SERIAL_DEFAULT ;

/* utility serial buffer */
char serBuf[MAX_SERBUF] ;
uint16_t serLen ;

/* buffer for storing received bytes - size must be power of two */
static uint8_t Receive_Buffer_Data[128];
static FIFO_BUFFER Receive_Buffer;

static void serial_receiver_enable(
    void)
{
    UCSR0B = _BV(TXEN0) | _BV(RXEN0) | _BV(RXCIE0);
}

ISR(USART0_RX_vect)
{
    uint8_t data_byte;

    if (BIT_CHECK(UCSR0A, RXC0)) {
        /* data is available */
        data_byte = UDR0;
        (void) FIFO_Put(&Receive_Buffer, data_byte);
    }
}

bool serial_byte_get(
    uint8_t * data_register)
{
    bool data_available = false;        /* return value */

    if (!FIFO_Empty(&Receive_Buffer)) {
        *data_register = FIFO_Get(&Receive_Buffer);
        data_available = true;
    }

    return data_available;
}

bool serial_byte_peek(
    uint8_t * data_register)
{
    bool data_available = false;        /* return value */

    if (!FIFO_Empty(&Receive_Buffer)) {
        *data_register = FIFO_Peek(&Receive_Buffer);
        data_available = true;
    }

    return data_available;
}

void serial_bytes_send(
    uint8_t * buffer,   /* data to send */
    uint16_t nbytes)
{       /* number of bytes of data */
    while (!BIT_CHECK(UCSR0A, UDRE0)) {
        /* do nothing - wait until Tx buffer is empty */
    }
    while (nbytes) {
        /* Send the data byte */
        UDR0 = *buffer;
        while (!BIT_CHECK(UCSR0A, UDRE0)) {
            /* do nothing - wait until Tx buffer is empty */
        }
        buffer++;
        nbytes--;
    }
    /* was the frame sent? */
    while (!BIT_CHECK(UCSR0A, TXC0)) {
        /* do nothing - wait until the entire frame in the
           Transmit Shift Register has been shifted out */
    }
    /* Clear the Transmit Complete flag by writing a one to it. */
    BIT_SET(UCSR0A, TXC0);

    return;
}

void serial_byte_send(
    uint8_t ch)
{
    while (!BIT_CHECK(UCSR0A, UDRE0)) {
        /* do nothing - wait until Tx buffer is empty */
    }
    /* Send the data byte */
    UDR0 = ch;

    return;
}

void serial_byte_transmit_complete(
    void)
{
    /* was the frame sent? */
    while (!BIT_CHECK(UCSR0A, TXC0)) {
        /* do nothing - wait until the entire frame in the
           Transmit Shift Register has been shifted out */
    }
    /* Clear the Transmit Complete flag by writing a one to it. */
    BIT_SET(UCSR0A, TXC0);
}

uint32_t serial_baud_rate(
    void)
{
    return Baud_Rate;
}

bool serial_baud_rate_set(
    uint32_t baud)
{
    bool valid = true;

    switch (baud) {
        case 9600:
        case 19200:
        case 38400:
        case 57600:
        case 76800:
        case 115200:
            Baud_Rate = baud;
            /* 2x speed mode */
            BIT_SET(UCSR0A, U2X0);
            /* configure baud rate */
            UBRR0 = (F_CPU / (8UL * Baud_Rate)) - 1;
            /* FIXME: store the baud rate */
            break;
        default:
            valid = false;
            break;
    }

    return valid;
}

static void serial_usart_init(
    void)
{
    /* enable the internal pullup on RXD0 -- all I/O setup in main.c
    BIT_CLEAR(DDRD, DDD2);
    BIT_SET(PORTD, PD2); */
    /* enable Transmit and Receive */
    UCSR0B = _BV(TXEN0) | _BV(RXEN0);
    /* Set USART Control and Status Register n C */
    /* Asynchronous USART 8-bit data, No parity, 1 stop */
    /* Set USART Mode Select: UMSELn1 UMSELn0 = 00 for Asynchronous USART */
    /* Set Parity Mode:  UPMn1 UPMn0 = 00 for Parity Disabled */
    /* Set Stop Bit Select: USBSn = 0 for 1 stop bit */
    /* Set Character Size: UCSZn2 UCSZn1 UCSZn0 = 011 for 8-bit */
    /* Clock Polarity: UCPOLn = 0 when asynchronous mode is used. */
    UCSR0C = _BV(UCSZ01) | _BV(UCSZ00);
    power_usart0_enable();
}

void serial_init(
    void)
{
    FIFO_Init(&Receive_Buffer, &Receive_Buffer_Data[0],
        (unsigned) sizeof(Receive_Buffer_Data));
    serial_usart_init();
    serial_baud_rate_set(Baud_Rate);
    serial_receiver_enable();
}
