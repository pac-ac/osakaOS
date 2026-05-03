#ifndef __OS__COMMON__TYPES_H
#define __OS__COMMON__TYPES_H



//init by removing most significant 2 bits
//from each byte in web color value

#define W_EMPTY 0x00
//#define W000000 0x00
#define W0000AA 0x01
#define W00AA00 0x02
#define W00AAAA 0x03
#define WAA0000 0x04
#define WAA00AA 0x05
#define WAA5500 0x06
#define WAAAAAA 0x07
#define W555555 0x08
#define W5555FF 0x09
#define W55FF55 0x0a
#define W55FFFF 0x0b
#define WFF5555 0x0c
#define WFF55FF 0x0d
#define WFFFF55 0x0e
#define WFFFFFF 0x0f

#define W000000 0x10
#define W101010 0x11
#define W202020 0x12
#define W353535 0x13
#define W454545 0x14
//#define W555555 0x15
#define W656565 0x16
#define W757575 0x17
#define W8A8A8A 0x18
#define W9A9A9A 0x19
//#define WAAAAAA 0x1a
#define WBABABA 0x1b
#define WCACACA 0x1c
#define WDFDFDF 0x1d
#define WEFEFEF 0x1e
//#define WFFFFFF 0x0f


#define W0000FF 0x20
#define W4100FF 0x21
#define W8200FF 0x22
#define WBE00FF 0x23
#define WFF00FF 0x24
#define WFF00BE 0x25
#define WFF0082 0x26
#define WFF0041 0x27
#define WFF0000 0x28
#define WFF4100 0x29
#define WFF8200 0x2a
#define WFFBE00 0x2b
#define WFFFF00 0x2c
#define WBEFF00 0x2d
#define W82FF00 0x2e
#define W41FF00 0x2f
#define W00FF00 0x30
#define W00FF41 0x31
#define W00FF82 0x32
#define W00FFBE 0x33
#define W00FFFF 0x34
#define W00BEFF 0x35
#define W0082FF 0x36
#define W0041FF 0x37
#define W8282FF 0x38
#define W9E82FF 0x39
#define WBE82FF 0x3a
#define WDF82FF 0x3b
#define WFF82FF 0x3c
#define WFF82DF 0x3d
#define WFF82BE 0x3e
#define WFF829E 0x3f
#define WFF8282 0x40
#define WFF9E82 0x41
#define WFFBE82 0x42
#define WFFDF82 0x43
#define WFFFF82 0x44
#define WDFFF82 0x45
#define WBEFF82 0x46
#define W9EFF82 0x47
#define W82FF82 0x48
#define W82FF9E 0x49
#define W82FFBE 0x4a
#define W82FFDF 0x4b
#define W82FFFF 0x4c
#define W82DFFF 0x4d
#define W82BEFF 0x4e
#define W829EFF 0x4f
#define WBABAFF 0x50
#define WCABAFF 0x51
#define WDFBAFF 0x52
#define WEFBAFF 0x53
#define WFFBAFF 0x54
#define WFFBAEF 0x55
#define WFFBADF 0x56
#define WFFBACA 0x57
#define WFFBABA 0x58
#define WFFCABA 0x59
#define WFFDFBA 0x5a
#define WFFEFBA 0x5b
#define WFFFFBA 0x5c
#define WEFFFBA 0x5d
#define WDFFFBA 0x5e
#define WCAFFBA 0x5f
#define WBAFFBA 0x60
#define WBAFFCA 0x61
#define WBAFFDF 0x62
#define WBAFFEF 0x63
#define WBAFFFF 0x64
#define WBAEFFF 0x65
#define WBADFFF 0x66
#define WBACAFF 0x67

#define W000071 0x68
#define W1C0071 0x69
#define W390071 0x6a
#define W550071 0x6b
#define W710071 0x6c
#define W710055 0x6d
#define W710039 0x6e
#define W71001C 0x6f
#define W710000 0x70
#define W711C00 0x71
#define W713900 0x72
#define W715500 0x73
#define W717100 0x74
#define W557100 0x75
#define W397100 0x76
#define W1C7100 0x77
#define W007100 0x78
#define W00711C 0x79
#define W007139 0x7a
#define W007155 0x7b
#define W007171 0x7c
#define W005571 0x7d
#define W003971 0x7e
#define W001C71 0x7f
#define W393971 0x80
#define W453971 0x81
#define W553971 0x82
#define W613971 0x83
#define W713971 0x84
#define W713961 0x85
#define W713955 0x86
#define W713945 0x87
#define W713939 0x88
#define W714539 0x89
#define W715539 0x8a
#define W716139 0x8b
#define W717139 0x8c
#define W617139 0x8d
#define W557139 0x8e
#define W457139 0x8f
#define W397139 0x90
#define W397145 0x91
#define W397155 0x92
#define W397161 0x93
#define W397171 0x94
#define W396171 0x95
#define W395571 0x96
#define W394571 0x97
#define W515171 0x98
#define W595171 0x99
#define W615171 0x9a
#define W695171 0x9b
#define W715171 0x9c
#define W715169 0x9d
#define W715161 0x9e
#define W715159 0x9f
#define W715151 0xa0
#define W715951 0xa1
#define W716151 0xa2
#define W716951 0xa3
#define W717151 0xa4
#define W697151 0xa5
#define W617151 0xa6
#define W597151 0xa7
#define W517151 0xa8
#define W517159 0xa9
#define W517161 0xaa
#define W517169 0xab
#define W517171 0xac
#define W516971 0xad
#define W516171 0xae
#define W515971 0xaf

#define W000041 0xb0
#define W100041 0xb1
#define W200041 0xb2
#define W310041 0xb3
#define W410041 0xb4
#define W410031 0xb5
#define W410020 0xb6
#define W410010 0xb7
#define W410000 0xb8
#define W411000 0xb9
#define W412000 0xba
#define W413100 0xbb
#define W414100 0xbc
#define W314100 0xbd
#define W204100 0xbe
#define W104100 0xbf
#define W004100 0xc0
#define W004110 0xc1
#define W004120 0xc2
#define W004131 0xc3
#define W004141 0xc4
#define W003141 0xc5
#define W002041 0xc6
#define W001041 0xc7
#define W202041 0xc8
#define W282041 0xc9
#define W312041 0xca
#define W392041 0xcb
#define W412041 0xcc
#define W412039 0xcd
#define W412031 0xce
#define W412028 0xcf
#define W412020 0xd0
#define W412820 0xd1
#define W413120 0xd2
#define W413920 0xd3
#define W414120 0xd4
#define W394120 0xd5
#define W314120 0xd6
#define W284120 0xd7
#define W204120 0xd8
#define W204128 0xd9
#define W204131 0xda
#define W204139 0xdb
#define W204141 0xdc
#define W203941 0xdd
#define W203141 0xde
#define W202841 0xdf
#define W2D2D41 0xe0
#define W312D41 0xe1
#define W352D41 0xe2
#define W3D2D41 0xe3
#define W412D41 0xe4
#define W412D3D 0xe5
#define W412D35 0xe6
#define W412D31 0xe7
#define W412D2D 0xe8
#define W41312D 0xe9
#define W41352D 0xea
#define W413D2D 0xeb
#define W41412D 0xec
#define W3D412D 0xed
#define W35412D 0xee
#define W31412D 0xef
#define W2D412D 0xf0
#define W2D4131 0xf1
#define W2D4135 0xf2
#define W2D413D 0xf3
#define W2D4141 0xf4
#define W2D3D41 0xf5
#define W2D3541 0xf6
#define W2D3141 0xf7

/*
#define W000000 0xf8
#define W000000 0xf9
#define W000000 0xfa
#define W000000 0xfb
#define W000000 0xfc
#define W000000 0xfd
#define W000000 0xfe
#define W000000 0xff
*/
#define W0055AA 0xf8
#define WAA0055 0xf9
#define WAA5555 0xfa
#define WAAAA55 0xfb
#define WFF5500 0xfc
#define WFF55AA 0xfd
#define WFFAA00 0xfe
#define WFFAA55 0xff


namespace os {

	namespace common {

		typedef char int8_t;
		typedef unsigned char uint8_t;
	
		typedef short int16_t;
		typedef unsigned short uint16_t;

		typedef int int32_t;
		typedef unsigned int uint32_t;
	
		typedef long long int int64_t;
		typedef unsigned long long int uint64_t;

		typedef const char* string;
		typedef uint32_t size_t;
	}
}

#endif
