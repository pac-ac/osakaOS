#ifndef __OS__HARDWARECOMMUNICATION__PORT_H
#define __OS__HARDWARECOMMUNICATION__PORT_H


#include <common/types.h>


namespace os
{
    namespace hardwarecommunication
    {

        class Port
        {
            protected:
                Port(os::common::uint16_t portnumber);
                ~Port();
                os::common::uint16_t portnumber;
        };

        
        class Port8Bit : public Port
        {
            public:
                Port8Bit(os::common::uint16_t portnumber);
                ~Port8Bit();

                virtual os::common::uint8_t Read();
                virtual void Write(os::common::uint8_t data);

            protected:
                static inline os::common::uint8_t Read8(os::common::uint16_t _port)
                {
                    os::common::uint8_t result;
                    __asm__ volatile("inb %1, %0" : "=a" (result) : "Nd" (_port));
                    return result;
                }

                static inline void Write8(os::common::uint16_t _port, os::common::uint8_t _data)
                {
                    __asm__ volatile("outb %0, %1" : : "a" (_data), "Nd" (_port));
                }
        };



        class Port8BitSlow : public Port8Bit
        {
            public:
                Port8BitSlow(os::common::uint16_t portnumber);
                ~Port8BitSlow();

                virtual void Write(os::common::uint8_t data);
            protected:
                static inline void Write8Slow(os::common::uint16_t _port, os::common::uint8_t _data)
                {
                    __asm__ volatile("outb %0, %1\njmp 1f\n1: jmp 1f\n1:" : : "a" (_data), "Nd" (_port));
                }

        };



        class Port16Bit : public Port
        {
            public:
                Port16Bit(os::common::uint16_t portnumber);
                ~Port16Bit();

                virtual os::common::uint16_t Read();
                virtual void Write(os::common::uint16_t data);

            protected:
                static inline os::common::uint16_t Read16(os::common::uint16_t _port)
                {
                    os::common::uint16_t result;
                    __asm__ volatile("inw %1, %0" : "=a" (result) : "Nd" (_port));
                    return result;
                }

                static inline void Write16(os::common::uint16_t _port, os::common::uint16_t _data)
                {
                    __asm__ volatile("outw %0, %1" : : "a" (_data), "Nd" (_port));
                }
        };



        class Port32Bit : public Port
        {
            public:
                Port32Bit(os::common::uint16_t portnumber);
                ~Port32Bit();

                virtual os::common::uint32_t Read();
                virtual void Write(os::common::uint32_t data);

            protected:
                static inline os::common::uint32_t Read32(os::common::uint16_t _port)
                {
                    os::common::uint32_t result;
                    __asm__ volatile("inl %1, %0" : "=a" (result) : "Nd" (_port));
                    return result;
                }

                static inline void Write32(os::common::uint16_t _port, os::common::uint32_t _data)
                {
                    __asm__ volatile("outl %0, %1" : : "a"(_data), "Nd" (_port));
                }
        };

    }
}


#endif
