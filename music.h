#ifndef __MUSIC_H__
#define __MUSIC_H__

#include <math.h>
#include <stdlib.h>

#include "pitches.h"

struct Note {
    unsigned short frequency;
    float duration;
};

struct Song {
    unsigned short tempo; // Notes per minute to play.
    unsigned int size; // The number of notes.
    struct Note* notes; // The notes data.
    unsigned short maxFreq;
    unsigned short minFreq;
};

struct PlayingSong {
    bool playing;
    unsigned int elapsed_ms;
    unsigned int next_note_at;
    size_t current_note;
    struct Song song;
};

struct Note makeNote(unsigned short f, float t) {
    struct Note n;
    n.frequency = f;
    n.duration = t;
    return n;
}

struct Song makeSong(unsigned short tempo, unsigned int size, struct Note* notes) {
    struct Song s;
    s.tempo = tempo;
    s.size = size;
    s.notes = notes;
    s.maxFreq = notes[0].frequency;
    s.minFreq = notes[0].frequency;
    for (unsigned int i = 1; i < size; i++) {
        unsigned short f = notes[i].frequency;
        if (f != NOTE_REST && f > s.maxFreq) s.maxFreq = f;
        if (f != NOTE_REST && f < s.minFreq) s.minFreq = f; 
    }
    return s;
}

void playSong(struct PlayingSong* ps, struct Song s) {
  ps->playing = true;
  ps->elapsed_ms = 0;
  ps->current_note = 0;
  ps->next_note_at = 0;
  ps->song = s;
}

unsigned int getMillisecondsPerNote(struct Song s) {
    return 240000 / s.tempo;
}

unsigned int getNoteDuration(struct Song s, size_t note) {
  return getMillisecondsPerNote(s) / s.notes[note].duration;
}

unsigned int getSongDuration(struct Song s) {
    return getMillisecondsPerNote(s) * s.size;
}

float getCurrentNoteProgress(struct PlayingSong ps) {
  unsigned int timeLeft = ps.next_note_at - ps.elapsed_ms;
  return getNoteDuration(ps.song, ps.current_note) - timeLeft;
}

float getRelativeNoteIntensity(struct Note n, struct Song s) {
    float min2 = sqrt(s.minFreq);
    float max2 = sqrt(s.maxFreq);
    float f2 = sqrt(n.frequency);

    //float f = ((float) n.frequency - s.minFreq) / (s.maxFreq - s.minFreq);
    float f = (f2 - min2) / (max2 - min2);
    if (f < 0) return 0;
    if (f > 1) return 1;
    return f;
}

// Updates the given playing song, and returns a reference to a note to play, if any. Otherwise, nullptr is returned.
struct Note* updateSong(struct PlayingSong* ps, unsigned int elapsed_ms) {
  if (!ps->playing) return NULL;
  ps->elapsed_ms += elapsed_ms;
  struct Note* note_to_play = NULL;
  if (ps->elapsed_ms > ps->next_note_at) {
    ps->current_note++;
    if (ps->current_note < ps->song.size) {
      note_to_play = &ps->song.notes[ps->current_note];
      ps->next_note_at = ps->elapsed_ms + (getMillisecondsPerNote(ps->song) / note_to_play->duration);
    } else {
      ps->playing = false;
    }
  }
  return note_to_play;
}

float getInterpolatedRelativeIntensity(struct PlayingSong* ps) {
    //return getRelativeNoteIntensity(ps->song.notes[ps->current_note], ps->song);
    // TODO: Figure out cubic bezier interpolation.
    struct Note note_a;
    struct Note note_b;
    float dur_a;
    float dur_b;
    unsigned int elapsed_time; // Elapsed ms between note a and b.
    unsigned int time_until_next_note = ps->next_note_at - ps->elapsed_ms;
    unsigned int current_note_dur = getNoteDuration(ps->song, ps->current_note);
    unsigned int current_note_elapsed_time = (current_note_dur - time_until_next_note);
    if (current_note_elapsed_time < current_note_dur / 2) {
        // We are on the lower-half of the current note.
        if (ps->current_note == 0) {
            note_a = makeNote(ps->song.minFreq, 4);
        } else {
            note_a = ps->song.notes[ps->current_note - 1];
        }
        note_b = ps->song.notes[ps->current_note];
        unsigned int dur_a = getMillisecondsPerNote(ps->song) / note_a.duration;
        elapsed_time = dur_a / 2 + current_note_elapsed_time;
    } else {
        // We are on the upper half of the current note.
        if (ps->current_note == ps->song.size - 1) {
            note_b = makeNote(ps->song.minFreq, 4);
        } else {
            note_b = ps->song.notes[ps->current_note + 1];
        }
        note_a = ps->song.notes[ps->current_note];
        elapsed_time = current_note_elapsed_time - (current_note_dur / 2);
    }
    dur_a = getMillisecondsPerNote(ps->song) / note_a.duration;
    dur_b = getMillisecondsPerNote(ps->song) / note_b.duration;
    float t = ((float) elapsed_time) / (dur_a / 2 + dur_b / 2);
    float intensity_a = getRelativeNoteIntensity(note_a, ps->song);
    float intensity_b = getRelativeNoteIntensity(note_b, ps->song);
    
    // linear
    //return (1 - t) * intensity_a + t * intensity_b;

    // cubic
    float y0 = intensity_a;
    float y1 = y0;
    float y2 = intensity_b;
    float y3 = y2;
    float n = 1 - t;

    return n*n*n*y0 + 3*t*n*n*y1 + 3*t*t*n*y2 + t*t*t*y3;
}

#endif
