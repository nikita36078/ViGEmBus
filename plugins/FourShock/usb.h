/*  LilyPad - Pad plugin for PS2 Emulator
 *  Copyright (C) 2002-2014  PCSX2 Dev Team/ChickenLiver
 *
 *  PCSX2 is free software: you can redistribute it and/or modify it under the
 *  terms of the GNU Lesser General Public License as published by the Free
 *  Software Found- ation, either version 3 of the License, or (at your option)
 *  any later version.
 *
 *  PCSX2 is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with PCSX2.  If not, see <http://www.gnu.org/licenses/>.
 */

// Note:  From libusb 0.1.12. We will redistribute it as LGPL3+ to keep a single license
/*
 * Prototypes, structure definitions and macros.
 *
 * Copyright (c) 2000-2003 Johannes Erdfelt <johannes@erdfelt.com>
 *
 * This library is covered by the LGPL2+, read LICENSE for details.
 *
 * This file (and only this file) may alternatively be licensed under the
 * BSD license as well, read LICENSE for details.
 */

#ifndef __USB_H__
#define __USB_H__

/*
 * 'interface' is defined somewhere in the Windows header files. This macro
 * is deleted here to avoid conflicts and compile errors.
 */

#ifdef interface
#undef interface
#endif

/*
 * PATH_MAX from limits.h can't be used on Windows if the dll and
 * import libraries are build/used by different compilers
 */

#define LIBUSB_PATH_MAX 512


/*
 * USB spec information
 *
 * This is all stuff grabbed from various USB specs and is pretty much
 * not subject to change
 */

/*
 * Device and/or Interface Class codes
 */
#define USB_CLASS_PER_INTERFACE 0 /* for DeviceClass */
#define USB_CLASS_AUDIO 1
#define USB_CLASS_COMM 2
#define USB_CLASS_HID 3
#define USB_CLASS_PRINTER 7
#define USB_CLASS_MASS_STORAGE 8
#define USB_CLASS_HUB 9
#define USB_CLASS_DATA 10
#define USB_CLASS_VENDOR_SPEC 0xff

/*
 * Descriptor types
 */
#define USB_DT_DEVICE 0x01
#define USB_DT_CONFIG 0x02
#define USB_DT_STRING 0x03
#define USB_DT_INTERFACE 0x04
#define USB_DT_ENDPOINT 0x05

#define USB_DT_HID 0x21
#define USB_DT_REPORT 0x22
#define USB_DT_PHYSICAL 0x23
#define USB_DT_HUB 0x29

/*
 * Descriptor sizes per descriptor type
 */
#define USB_DT_DEVICE_SIZE 18
#define USB_DT_CONFIG_SIZE 9
#define USB_DT_INTERFACE_SIZE 9
#define USB_DT_ENDPOINT_SIZE 7
#define USB_DT_ENDPOINT_AUDIO_SIZE 9 /* Audio extension */
#define USB_DT_HUB_NONVAR_SIZE 7


/* ensure byte-packed structures */
#include <pshpack1.h>


/* All standard descriptors have these 2 fields in common */
struct usb_descriptor_header
{
    unsigned char bLength;
    unsigned char bDescriptorType;
};

/* String descriptor */
struct usb_string_descriptor
{
    unsigned char bLength;
    unsigned char bDescriptorType;
    unsigned short wData[1];
};

/* HID descriptor */
struct usb_hid_descriptor
{
    unsigned char bLength;
    unsigned char bDescriptorType;
    unsigned short bcdHID;
    unsigned char bCountryCode;
    unsigned char bNumDescriptors;
};

/* Endpoint descriptor */
#define USB_MAXENDPOINTS 32
struct usb_endpoint_descriptor
{
    unsigned char bLength;
    unsigned char bDescriptorType;
    unsigned char bEndpointAddress;
    unsigned char bmAttributes;
    unsigned short wMaxPacketSize;
    unsigned char bInterval;
    unsigned char bRefresh;
    unsigned char bSynchAddress;

    unsigned char *extra; /* Extra descriptors */
    int extralen;
};

#define USB_ENDPOINT_ADDRESS_MASK 0x0f /* in bEndpointAddress */
#define USB_ENDPOINT_DIR_MASK 0x80

#define USB_ENDPOINT_TYPE_MASK 0x03 /* in bmAttributes */
#define USB_ENDPOINT_TYPE_CONTROL 0
#define USB_ENDPOINT_TYPE_ISOCHRONOUS 1
#define USB_ENDPOINT_TYPE_BULK 2
#define USB_ENDPOINT_TYPE_INTERRUPT 3

/* Interface descriptor */
#define USB_MAXINTERFACES 32
struct usb_interface_descriptor
{
    unsigned char bLength;
    unsigned char bDescriptorType;
    unsigned char bInterfaceNumber;
    unsigned char bAlternateSetting;
    unsigned char bNumEndpoints;
    unsigned char bInterfaceClass;
    unsigned char bInterfaceSubClass;
    unsigned char bInterfaceProtocol;
    unsigned char iInterface;

    struct usb_endpoint_descriptor *endpoint;

    unsigned char *extra; /* Extra descriptors */
    int extralen;
};

#define USB_MAXALTSETTING 128 /* Hard limit */

struct usb_interface
{
    struct usb_interface_descriptor *altsetting;

    int num_altsetting;
};

/* Configuration descriptor information.. */
#define USB_MAXCONFIG 8
struct usb_config_descriptor
{
    unsigned char bLength;
    unsigned char bDescriptorType;
    unsigned short wTotalLength;
    unsigned char bNumInterfaces;
    unsigned char bConfigurationValue;
    unsigned char iConfiguration;
    unsigned char bmAttributes;
    unsigned char MaxPower;

    struct usb_interface *interface;

    unsigned char *extra; /* Extra descriptors */
    int extralen;
};

/* Device descriptor */
struct usb_device_descriptor
{
    unsigned char bLength;
    unsigned char bDescriptorType;
    unsigned short bcdUSB;
    unsigned char bDeviceClass;
    unsigned char bDeviceSubClass;
    unsigned char bDeviceProtocol;
    unsigned char bMaxPacketSize0;
    unsigned short idVendor;
    unsigned short idProduct;
    unsigned short bcdDevice;
    unsigned char iManufacturer;
    unsigned char iProduct;
    unsigned char iSerialNumber;
    unsigned char bNumConfigurations;
};

struct usb_ctrl_setup
{
    unsigned char bRequestType;
    unsigned char bRequest;
    unsigned short wValue;
    unsigned short wIndex;
    unsigned short wLength;
};

/*
 * Standard requests
 */
#define USB_REQ_GET_STATUS 0x00
#define USB_REQ_CLEAR_FEATURE 0x01
/* 0x02 is reserved */
#define USB_REQ_SET_FEATURE 0x03
/* 0x04 is reserved */
#define USB_REQ_SET_ADDRESS 0x05
#define USB_REQ_GET_DESCRIPTOR 0x06
#define USB_REQ_SET_DESCRIPTOR 0x07
#define USB_REQ_GET_CONFIGURATION 0x08
#define USB_REQ_SET_CONFIGURATION 0x09
#define USB_REQ_GET_INTERFACE 0x0A
#define USB_REQ_SET_INTERFACE 0x0B
#define USB_REQ_SYNCH_FRAME 0x0C

#define USB_TYPE_STANDARD (0x00 << 5)
#define USB_TYPE_CLASS (0x01 << 5)
#define USB_TYPE_VENDOR (0x02 << 5)
#define USB_TYPE_RESERVED (0x03 << 5)

#define USB_RECIP_DEVICE 0x00
#define USB_RECIP_INTERFACE 0x01
#define USB_RECIP_ENDPOINT 0x02
#define USB_RECIP_OTHER 0x03

/*
 * Various libusb API related stuff
 */

#define USB_ENDPOINT_IN 0x80
#define USB_ENDPOINT_OUT 0x00

/* Error codes */
#define USB_ERROR_BEGIN 500000

/*
 * This is supposed to look weird. This file is generated from autoconf
 * and I didn't want to make this too complicated.
 */
#define USB_LE16_TO_CPU(x)

/* Data types */
/* struct usb_device; */
/* struct usb_bus; */

struct usb_device
{
    struct usb_device *next, *prev;

    char filename[LIBUSB_PATH_MAX];

    struct usb_bus *bus;

    struct usb_device_descriptor descriptor;
    struct usb_config_descriptor *config;

    void *dev; /* Darwin support */

    unsigned char devnum;

    unsigned char num_children;
    struct usb_device **children;
};

struct usb_bus
{
    struct usb_bus *next, *prev;

    char dirname[LIBUSB_PATH_MAX];

    struct usb_device *devices;
    unsigned long location;

    struct usb_device *root_dev;
};

/* Version information, Windows specific */
struct usb_version
{
    struct
    {
        int major;
        int minor;
        int micro;
        int nano;
    } dll;
    struct
    {
        int major;
        int minor;
        int micro;
        int nano;
    } driver;
};


struct usb_dev_handle;
typedef struct usb_dev_handle usb_dev_handle;

/* Variables */
#ifndef __USB_C__
#define usb_busses usb_get_busses()
#endif



#include <poppack.h>

#endif /* __USB_H__ */
