/*
 * mt32-pi-control
 *
 * An mt32-pi control program for DOS PCs
 * and Amiga computers
 *
 * Copyright (C) 2021 Andreas Zdziarstek
 */

#ifndef MIDI_DEV_H
#define MIDI_DEV_H

int mididev_init(void);
int mididev_deinit(void);

int mididev_send_bytes(const unsigned char *buf, int len);

void mididev_print_usage(void);

void mididev_add_optstr(char *optstr);
int mididev_parse_arg(int c, const char *optarg);

#endif /* MIDI_DEV_H */
