#ifndef __OS__GUI__FONT_H
#define __OS__GUI__FONT_H

#include <common/types.h>


namespace os {
	
	namespace gui {
		
		//Font is 8x5 but 9x6 is easier to read.
		constexpr common::uint8_t font_width = 6;
		constexpr common::uint8_t font_height = 9;


		//font_system
		static common::uint8_t font_full[5] {
			0xff, 0xff, 0xff, 0xff, 0xff
		};




		//font_special *******************************************************
		static common::uint8_t font_space[5] {
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000
		};
		
		static common::uint8_t font_exclamation[5] {
			0b00000000,
			0b00000000,
			0b01011111,
			0b00000000,
			0b00000000
		};
		
		static common::uint8_t font_quote[5] {
			0b00000000,
			0b00000111,
			0b00000000,
			0b00000111,
			0b00000000
		};
		
		static common::uint8_t font_pound[5] {
			0b00100010,
			0b01111111,
			0b00100010,
			0b01111111,
			0b00100010
		};
		
		static common::uint8_t font_dollar[5] {
			0b00000000,
			0b01001000,
			0b11110100,
			0b01011110,
			0b00100100
		};
		
		static common::uint8_t font_percent[5] {
			0b00000000,
			0b01100010,
			0b00011000,
			0b00001100,
			0b00100011
		};
		
		
		static common::uint8_t font_ampersand[5] {
			0b00000000,
			0b00110100,
			0b01001010,
			0b01010100,
			0b00100000
		};
		
		
		static common::uint8_t font_apos[5] {
			0b00000000,
			0b00000000,
			0b00000111,
			0b00000000,
			0b00000000
		};
		
		
		static common::uint8_t font_rightpar[5] {
			0b00000000,
			0b00000000,
			0b00011100,
			0b00100010,
			0b01000001
		};
		
		static common::uint8_t font_leftpar[5] {
			0b00000000,
			0b01000001,
			0b00100010,
			0b00011100,
			0b00000000
		};
		
		static common::uint8_t font_asterisk[5] {
			0b00000000,
			0b00001010,
			0b00000111,
			0b00001010,
			0b00000000
		};
		
		static common::uint8_t font_plus[5] {
			0b00000000,
			0b00010000,
			0b00111000,
			0b00010000,
			0b00000000
		};
		
		static common::uint8_t font_comma[5] {
			0b00000000,
			0b10000000,
			0b01100000,
			0b00000000,
			0b00000000
		};
		
		static common::uint8_t font_dash[5] {
			0b00000000,
			0b00010000,
			0b00010000,
			0b00010000,
			0b00000000
		};
		
		static common::uint8_t font_period[5] {
			0b00000000,
			0b01000000,
			0b00000000,
			0b00000000,
			0b00000000
		};
		
		static common::uint8_t font_slash[5] {
			0b00000000,
			0b01100000,
			0b00011000,
			0b00001100,
			0b00000011
		};
		
		
		static common::uint8_t font_colon[5] {
			0b00000000,
			0b00000000,
			0b01100110,
			0b00000000,
			0b00000000
		};
		
		static common::uint8_t font_semicolon[5] {
			0b00000000,
			0b10000000,
			0b01100110,
			0b00000000,
			0b00000000
		};
		
		static common::uint8_t font_less[5] {
			0b00000000,
			0b00001000,
			0b00010100,
			0b00100010,
			0b01000001
		};
		
		static common::uint8_t font_equal[5] {
			0b00000000,
			0b00101000,
			0b00101000,
			0b00101000,
			0b00000000
		};
		
		static common::uint8_t font_more[5] {
			0b00000000,
			0b01000001,
			0b00100010,
			0b00010100,
			0b00001000
		};
		
		static common::uint8_t font_question[5] {
			0b00000000,
			0b00000010,
			0b01010001,
			0b00001001,
			0b00000110
		};
		
		static common::uint8_t font_at[5] {
			0b00000000,
			0b00111100,
			0b01000010,
			0b01011010,
			0b00100010
		};
		
		static common::uint8_t font_rightbracket[5] {
			0b00000000,
			0b01111111,
			0b01000001,
			0b01000001,
			0b00000000
		};
		
		static common::uint8_t font_backslash[5] {
			0b00000000,
			0b00000011,
			0b00001100,
			0b00011000,
			0b01100000
		};
		
		static common::uint8_t font_leftbracket[5] {
			0b00000000,
			0b01000001,
			0b01000001,
			0b01111111,
			0b00000000
		};
		
		static common::uint8_t font_up[5] {
			0b00000000,
			0b00000110,
			0b00000001,
			0b00000110,
			0b00000000
		};
		
		static common::uint8_t font_underscore[5] {
			0b00000000,
			0b10000000,
			0b10000000,
			0b10000000,
			0b10000000
		};
		
		static common::uint8_t font_backtick[5] {
			0b00000000,
			0b00000001,
			0b00000010,
			0b00000010,
			0b00000000
		};
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		




		
		//font_numbers *******************************************************
		static common::uint8_t font_0[5] {
			0b00000000,
			0b00111110,
			0b01001001,
			0b01010001,
			0b00111110
		};
		
		static common::uint8_t font_1[5] {
			0b00000000,
			0b01000010,
			0b01111111,
			0b01000000,
			0b00000000
		};
		
		static common::uint8_t font_2[5] {
			0b00000000,
			0b01100010,
			0b01010001,
			0b01001001,
			0b01000110
		};
		
		static common::uint8_t font_3[5] {
			0b00000000,
			0b00100010,
			0b01001001,
			0b01001001,
			0b00110110
		};
		
		static common::uint8_t font_4[5] {
			0b00000000,
			0b00001111,
			0b00001000,
			0b00001000,
			0b01111111
		};
		
		static common::uint8_t font_5[5] {
			0b00000000,
			0b01000111,
			0b01001001,
			0b01001001,
			0b00110001
		};
		
		static common::uint8_t font_6[5] {
			0b00000000,
			0b00111100,
			0b01001010,
			0b01001001,
			0b00110001
		};
		
		static common::uint8_t font_7[5] {
			0b00000000,
			0b01100001,
			0b00010001,
			0b00001001,
			0b00000111
		};
		
		static common::uint8_t font_8[5] {
			0b00000000,
			0b00110110,
			0b01001001,
			0b01001001,
			0b00110110
		};
		
		static common::uint8_t font_9[5] {
			0b00000000,
			0b00000110,
			0b01001001,
			0b00101001,
			0b00011110
		};
	

		
		
		



		//font_lower *******************************************************
		static common::uint8_t font_a[5] {
			0b00000000,
			0b00100100,
			0b01010100,
			0b01010100,
			0b01111000
		};
		
		static common::uint8_t font_b[5] {
			0b00000000,
			0b01111111,
			0b01001000,
			0b01001000,
			0b00110000
		};
		
		static common::uint8_t font_c[5] {
			0b00000000,
			0b00111000,
			0b01000100,
			0b01000100,
			0b01000100
		};
		
		static common::uint8_t font_d[5] {
			0b00000000,
			0b00110000,
			0b01001000,
			0b01001000,
			0b01111111
		};
		
		static common::uint8_t font_e[5] {
			0b00000000,
			0b00111000,
			0b01010100,
			0b01010100,
			0b00101100
		};
		
		static common::uint8_t font_f[5] {
			0b00000000,
			0b00000100,
			0b01111110,
			0b00000101,
			0b00000001
		};
		
		static common::uint8_t font_g[5] {
			0b00000000,
			0b10011000,
			0b10100100,
			0b10100100,
			0b01111100
		};
		
		static common::uint8_t font_h[5] {
			0b00000000,
			0b01111111,
			0b00000100,
			0b00000100,
			0b01111000
		};
		
		static common::uint8_t font_i[5] {
			0b00000000,
			0b00000000,
			0b00111101,
			0b01000000,
			0b00000000
		};

		static common::uint8_t font_j[5] {
			0b00000000,
			0b01000000,
			0b01000000,
			0b00111101,
			0b00000000
		};

		static common::uint8_t font_k[5] {
			0b00000000,
			0b01111111,
			0b00010000,
			0b00101000,
			0b01000100
		};

		static common::uint8_t font_l[5] {
			0b00000000,
			0b00000000,
			0b00111111,
			0b01000000,
			0b00000000
		};
	
		static common::uint8_t font_m[5] {
			0b01111100,
			0b00000100,
			0b01111100,
			0b00000100,
			0b01111000
		};

		static common::uint8_t font_n[5] {
			0b00000000,
			0b01111100,
			0b00000100,
			0b00000100,
			0b01111000
		};

		static common::uint8_t font_o[5] {
			0b00000000,
			0b00111000,
			0b01000100,
			0b01000100,
			0b00111000
		};
	
		static common::uint8_t font_p[5] {
			0b00000000,
			0b11111100,
			0b00100100,
			0b00100100,
			0b00111100
		};

		static common::uint8_t font_q[5] {
			0b00000000,
			0b00111100,
			0b00100100,
			0b00100100,
			0b11111100
		};

		static common::uint8_t font_r[5] {
			0b00000000,
			0b01111100,
			0b00001000,
			0b00001000,
			0b00011000
		};

		static common::uint8_t font_s[5] {
			0b00000000,
			0b01001000,
			0b01010100,
			0b01010100,
			0b00100100
		};

		static common::uint8_t font_t[5] {
			0b00000000,
			0b00000100,
			0b00111111,
			0b01000100,
			0b00000000
		};

		static common::uint8_t font_u[5] {
			0b00000000,
			0b00111100,
			0b01000000,
			0b01000000,
			0b01111100
		};

		static common::uint8_t font_v[5] {
			0b00000000,
			0b00111100,
			0b01000000,
			0b00111100,
			0b00000000
		};

		static common::uint8_t font_w[5] {
			0b00111100,
			0b01000000,
			0b00111000,
			0b01000000,
			0b00111100
		};

		static common::uint8_t font_x[5] {
			0b00000000,
			0b01101100,
			0b00010000,
			0b01101100,
			0b00000000
		};

		static common::uint8_t font_y[5] {
			0b00000000,
			0b00011100,
			0b10100000,
			0b10100000,
			0b11111100
		};

		static common::uint8_t font_z[5] {
			0b00000000,
			0b01100100,
			0b01010100,
			0b01010100,
			0b01001100
		};
		

		//font_upper *******************************************************
		static common::uint8_t font_A[5] {
			0b00000000,
			0b01111110,
			0b00001001,
			0b00001001,
			0b01111110
		};
		
		static common::uint8_t font_B[5] {
			0b00000000,
			0b01111111,
			0b01001001,
			0b01001001,
			0b00110110
		};
		
		static common::uint8_t font_C[5] {
			0b00000000,
			0b00111110,
			0b01000001,
			0b01000001,
			0b01000001
		};
		
		static common::uint8_t font_D[5] {
			0b00000000,
			0b01111111,
			0b01000001,
			0b01000001,
			0b00111110
		};
		
		static common::uint8_t font_E[5] {
			0b00000000,
			0b01111111,
			0b01001001,
			0b01001001,
			0b01001001
		};
		
		static common::uint8_t font_F[5] {
			0b00000000,
			0b01111111,
			0b00001001,
			0b00001001,
			0b00000001
		};
		
		static common::uint8_t font_G[5] {
			0b00000000,
			0b00111110,
			0b01000001,
			0b01001001,
			0b01111001
		};
		
		static common::uint8_t font_H[5] {
			0b00000000,
			0b01111111,
			0b00001000,
			0b00001000,
			0b01111111
		};
		
		static common::uint8_t font_I[5] {
			0b00000000,
			0b01000001,
			0b01111111,
			0b01000001,
			0b00000000
		};
		
		static common::uint8_t font_J[5] {
			0b00000000,
			0b01000001,
			0b01000001,
			0b00111111,
			0b00000001
		};
		
		static common::uint8_t font_K[5] {
			0b00000000,
			0b01111111,
			0b00001000,
			0b00010100,
			0b01100011
		};
		
		static common::uint8_t font_L[5] {
			0b00000000,
			0b01111111,
			0b01000000,
			0b01000000,
			0b01000000
		};
		
		static common::uint8_t font_M[5] {
			0b01111111,
			0b00000010,
			0b01111100,
			0b00000010,
			0b01111111
		};
		
		static common::uint8_t font_N[5] {
			0b00000000,
			0b01111111,
			0b00000010,
			0b00000100,
			0b01111111
		};
		
		static common::uint8_t font_O[5] {
			0b00000000,
			0b00111110,
			0b01000001,
			0b01000001,
			0b00111110
		};
		
		static common::uint8_t font_P[5] {
			0b00000000,
			0b01111111,
			0b00001001,
			0b00001001,
			0b00000110
		};
		
		static common::uint8_t font_Q[5] {
			0b00000000,
			0b00111110,
			0b01000001,
			0b01000001,
			0b10111110
		};
		
		static common::uint8_t font_R[5] {
			0b00000000,
			0b01111111,
			0b00001001,
			0b00001001,
			0b01110110
		};
		
		static common::uint8_t font_S[5] {
			0b00000000,
			0b01000110,
			0b01001001,
			0b01001001,
			0b00110001
		};
		
		static common::uint8_t font_T[5] {
			0b00000001,
			0b00000001,
			0b01111111,
			0b00000001,
			0b00000001
		};
		
		static common::uint8_t font_U[5] {
			0b00000000,
			0b00111111,
			0b01000000,
			0b01000000,
			0b00111111
		};
		
		static common::uint8_t font_V[5] {
			0b00000000,
			0b00111111,
			0b01000000,
			0b00111111,
			0b00000000
		};
		
		static common::uint8_t font_W[5] {
			0b00111111,
			0b01000000,
			0b00111100,
			0b01000000,
			0b00111111
		};
		
		static common::uint8_t font_X[5] {
			0b00000000,
			0b01110111,
			0b00001000,
			0b01110111,
			0b00000000
		};
		
		static common::uint8_t font_Y[5] {
			0b00000000,
			0b00000111,
			0b01111000,
			0b00000111,
			0b00000000
		};
		
		static common::uint8_t font_Z[5] {
			0b00000000,
			0b01100001,
			0b01011001,
			0b01001101,
			0b01000011
		};
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		



		
		//font_array *******************************************************
		static common::uint8_t* charset[256] {
							//special 1
							font_space, font_exclamation, font_quote, font_pound, 
							font_dollar, font_percent, font_ampersand, font_apos,
							font_rightpar, font_leftpar, font_asterisk, font_plus,
							font_comma, font_dash, font_period, font_slash,

							//numbers
							font_0, font_1, font_2, font_3, font_4, font_5, 
							font_6, font_7, font_8, font_9,

							//special 2
							font_colon, font_semicolon, font_less, font_equal,
							font_more, font_question, font_at,

							//uppercase
							font_A, font_B, font_C, font_D, font_E, font_F,
							font_G, font_H, font_I, font_J, font_K, font_L, 
							font_M, font_N, font_O, font_P, font_Q, font_R, 
							font_S, font_T, font_U, font_V, font_W, font_X, 
							font_Y, font_Z,

							//special 3
							font_rightbracket, font_backslash, font_leftbracket,
							font_up, font_underscore, font_backtick,
							
							//lower case
							font_a, font_b, font_c, font_d, font_e, font_f, 
							font_g, font_h, font_i, font_j, font_k, font_l, 
							font_m, font_n, font_o, font_p, font_q, font_r, 
							font_s, font_t, font_u, font_v, font_w, font_x, 
							font_y, font_z

							
		
		};
	
	}
}

#endif
