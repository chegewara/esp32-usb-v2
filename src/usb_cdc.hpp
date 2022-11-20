#pragma once
#include <memory>
#include <functional>
#include "usb_device.hpp"

// #if CONFIG_TINYUSB
// #if CFG_TUD_CDC

namespace esptinyusb
{

    class USBcdc : public BaseDevice
    {
        typedef void (*read_cb_t)();

    protected:
        uint8_t port = 0;
        static uint8_t _port;

    public:
        using BaseDevice::BaseDevice;
        ~USBcdc();

        virtual bool begin(uint8_t _eps = 1);
        virtual bool end();
        virtual void onData(read_cb_t cb)
        {
            onReadCb = cb;
        }
        virtual void onLineCoding(cdc_line_coding_t const *p_line_coding)
        {
            printf("line coding cb => %d\n", port);
        }
        virtual bool onLineState(bool dtr, bool rts)
        {
            printf("line state cb\n");
            return true;
        }
        virtual void onComplete() {}

        virtual inline uint32_t available() { return tud_cdc_n_available(port); }
        virtual inline bool isConnected() { return tud_cdc_n_connected(port); }

        virtual inline uint32_t read(void *buffer, uint32_t bufsize) // Read received bytes
        {
            return tud_cdc_n_read(port, buffer, bufsize);
        }
        virtual inline char read() // Read received bytes
        {
            char c = '1';
            tud_cdc_n_read(port, &c, 1);
            return c;
        }

        virtual inline uint32_t write(void const *buffer, uint32_t bufsize) // Write bytes to TX FIFO, data may remain in the FIFO for a while
        {
            tud_cdc_n_write(port, buffer, bufsize);
            return tud_cdc_n_write_flush(port);
        }
        virtual inline uint32_t write(char c) // Write bytes to TX FIFO, data may remain in the FIFO for a while
        {
            tud_cdc_n_write(port, &c, 1);
            return tud_cdc_n_write_flush(port);
        }

        virtual inline bool clear() // Clear the transmit FIFO
        {
            return tud_cdc_n_write_clear(port);
        }
        virtual inline bool peek(uint8_t *ui8) // Get a byte from FIFO at the specified position without removing it
        {
            return tud_cdc_n_peek(port, ui8);
        }
        virtual inline void flush() // Clear the received FIFO
        {
            tud_cdc_n_read_flush(port);
        }

        friend void tud_cdc_rx_cb(uint8_t itf);
        virtual inline void onData()
        {
            if (onReadCb)
                onReadCb();
        }

    protected:
        read_cb_t onReadCb = nullptr;
        // std::function<void(void)>{
        // for (size_t i = 0; i <= available(); i++)
        // {
        //     read();
        // } };
    };

}

// #endif // CFG_TUD_CDC
// #endif // CONFIG_TINYUSB


// // Get current line state. Bit 0:  DTR (Data Terminal Ready), Bit 1: RTS (Request to Send)
// uint8_t tud_cdc_n_get_line_state(uint8_t itf);

// // Get current line encoding: bit rate, stop bits parity etc ..
// void tud_cdc_n_get_line_coding(uint8_t itf, cdc_line_coding_t *coding);

// // Set special character that will trigger tud_cdc_rx_wanted_cb() callback on receiving
// void tud_cdc_n_set_wanted_char(uint8_t itf, char wanted);

// // Force sending data if possible, return number of forced bytes
// uint32_t tud_cdc_n_write_flush(uint8_t itf);

// // Return the number of bytes (characters) available for writing to TX FIFO buffer in a single n_write operation.
// uint32_t tud_cdc_n_write_available(uint8_t itf);
