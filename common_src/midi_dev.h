/*
 * mt32-pi-control
 *
 * An mt32-pi control program for DOS PCs
 * and Amiga computers
 *
 * Copyright (C) 2021 Andreas Zdziarstek
 */

#ifndef __MIDI_DEV_H__
#define __MIDI_DEV_H__

int mididev_init(void);
int mididev_deinit(void);

int mididev_send_bytes(const unsigned char *buf, int len);

void mididev_print_usage(void);

void mididev_add_optstr(char *optstr);
int mididev_parse_arg(int c, const char *optarg);

#endif /* __MIDI_DEV_H__ */
