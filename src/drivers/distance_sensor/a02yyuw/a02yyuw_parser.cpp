/****************************************************************************
 *
 *   Copyright (c) 2017-2019 PX4 Development Team. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name PX4 nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/**
 * @file modified from tfmini_parser.cpp
 * @author Alex Thatcher <thatcher@electro.aero>
 *
 * Parser for the DFRobot A02YYUW Ultrasonic Sensor
 */

#include "a02yyuw_parser.h"
#include <string.h>
#include <stdlib.h>

// #define A02YYUW_DEBUG

#ifdef A02YYUW_DEBUG
#include <stdio.h>

const char *parser_state[] = {
	"0_UNSYNC",
	"1_SYNC",
	"2_GOT_DIST_H",
	"3_GOT_DIST_L",
	"4_GOT_CHECKSUM"
};
#endif

int a02yyuw_parse(char c, char *parserbuf, unsigned *parserbuf_index, A02YYUW_PARSE_STATE *state, float *dist)
{
	int ret = -1;
	//char *end;

	switch (*state) {
	case A02YYUW_PARSE_STATE::STATE4_GOT_CHECKSUM:
		if (c == 0xFF) {
			*state = A02YYUW_PARSE_STATE::STATE1_SYNC;
			parserbuf[*parserbuf_index] = c;
			(*parserbuf_index)++;

		} else {
			*state = A02YYUW_PARSE_STATE::STATE0_UNSYNC;
		}

		break;

	case A02YYUW_PARSE_STATE::STATE0_UNSYNC:
		if (c == 0xFF) {
			*state = A02YYUW_PARSE_STATE::STATE1_SYNC;
			parserbuf[*parserbuf_index] = c;
			(*parserbuf_index)++;
		}

		break;

	case A02YYUW_PARSE_STATE::STATE1_SYNC:
		*state = A02YYUW_PARSE_STATE::STATE2_GOT_DIST_H;
		parserbuf[*parserbuf_index] = c;
		(*parserbuf_index)++;

		break;

	case A02YYUW_PARSE_STATE::STATE2_GOT_DIST_H:
		*state = A02YYUW_PARSE_STATE::STATE3_GOT_DIST_L;
		parserbuf[*parserbuf_index] = c;
		(*parserbuf_index)++;

		break;

	case A02YYUW_PARSE_STATE::STATE3_GOT_DIST_L:
		// Find the checksum
		unsigned char cksm = 0;

		for (int i = 0; i < 3; i++) {
			cksm += parserbuf[i];
		}

		if (c == cksm) {
			parserbuf[*parserbuf_index] = '\0';
			unsigned int t1 = parserbuf[1];
			unsigned int t2 = parserbuf[2];
			t1 <<= 8;
			t1 += t2;
			*dist = ((float)t1) / 1000;
			*state = A02YYUW_PARSE_STATE::STATE4_GOT_CHECKSUM;
			*parserbuf_index = 0;
			ret = 0;

		} else {
			*state = A02YYUW_PARSE_STATE::STATE0_UNSYNC;
			*parserbuf_index = 0;
		}

		break;
	}

#ifdef A02YYUW_DEBUG
	printf("state: A02YYUW_PARSE_STATE%s\n", parser_state[*state]);
#endif

	return ret;
}
