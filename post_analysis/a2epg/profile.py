import csv
import os
import re
from collections.abc import Sequence
from operator import attrgetter


class EPGSegment:
    def __init__(self, kind, start, end):
        self.waveform = str(kind).lower()
        self.start = start
        self.end = end

    def __repr__(self):
        return 'EPGSegment({}, {}, {})'.format(
            self.waveform, self.start, self.end)


class EPGProfile(Sequence):
    def __init__(self, name, segments):
        self.name = name
        self.basename = re.split(r'-(auto|manual)\d*$', self.name)[0]
        self.source = re.search(r'-(auto|manual)\d*$', self.name).group(1)
        self._segments = tuple(segments)
        self.waveforms = frozenset(map(attrgetter('waveform'), self._segments))

    def __getitem__(self, item):
        return self._segments[item]

    def __len__(self):
        return len(self._segments)

    def __repr__(self):
        return 'EPGProfile({}, {})'.format(repr(self.name), len(self))

    @property
    def time_span(self):
        return self[-1].end

    def count(self, val=None):
        if val is None:
            return len(self._segments)
        return len([seg for seg in self._segments if seg.waveform == val])

    @staticmethod
    def from_file(pathname):
        with open(pathname) as csvfile:
            tokens = csvfile.readline().split(',')
            fieldnames = tuple(map(lambda tok: tok.strip().lower(), tokens))
            reader = csv.DictReader(csvfile, fieldnames=fieldnames)
            segments = []
            for row in reader:
                start, end = [float(row[i]) for i in ('start', 'end')]
                segments.append(EPGSegment(row['type'], start, end))
        basename = os.path.splitext(os.path.basename(pathname))[0]
        return EPGProfile(basename, segments)
