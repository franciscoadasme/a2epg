from collections import namedtuple
from collections.abc import Iterator, Sequence
from operator import attrgetter, itemgetter
from .stats import ConfusionMatrix, EPGComparisonStats

EPGPair = namedtuple('EPGPair', 'auto manual')


class EPGComparisonError(Exception):
    def __init__(self, group, msg):
        super().__init__(msg)
        self.group = group

    def __str__(self):
        return '{}: {}'.format(self.group[0].basename, super().__str__())


class EPGComparator:
    def __init__(self, group):
        if group[0].time_span != group[1].time_span:
            raise EPGComparisonError(
                group, 'cannot compare profiles with different length')
        auto, manual = sorted(group, key=attrgetter('source'))
        self.group = EPGPair(auto, manual)

    auto = property(lambda self: self.group.auto)
    manual = property(lambda self: self.group.manual)

    def compare(self):
        fingerprints = tuple(_EPGFingerprint(profile) for profile in self.group)
        waveforms = sorted(self.auto.waveforms.union(self.manual.waveforms))
        cfm = self._create_confusion_matrix(fingerprints, waveforms)
        offsets = self._calculate_offsets(fingerprints)
        return EPGComparisonStats(self.group, cfm, offsets)

    @staticmethod
    def _calculate_offsets(fingerprints, fractional=True):
        auto, manual = fingerprints
        offsets = {k: [[], []] for k in set(manual) - {None}}
        manual_iter = iter(manual)
        while not manual_iter.at_end():
            waveform, start, end = manual_iter.advance()
            match = auto.find_closest_match_to(manual, start, end)
            if match is None:
                continue
            elapsed = end - start + 1 if fractional else 1
            offsets[waveform][0].append((match[0] - start) / elapsed)
            offsets[waveform][1].append((match[1] - end) / elapsed)
        return offsets

    @staticmethod
    def _create_confusion_matrix(fingerprints, waveforms):
        cfm = ConfusionMatrix(waveforms)
        for i in range(len(fingerprints[0])):
            waveforms = tuple(map(itemgetter(i), fingerprints))
            if waveforms[0] is not None and waveforms[1] is not None:
                cfm[tuple(waveforms)] += 1
        return cfm


class _EPGFingerprint(Sequence):
    SAMPLING_RATE = 100

    def __init__(self, data):
        if not isinstance(data, str):
            data = _EPGFingerprint.make(data)
        self.data = data

    def __getitem__(self, item):
        return self.data[item]

    def __iter__(self):
        return _EPGFingerprintIterator(self)

    def __len__(self):
        return len(self.data)

    def find_start_of_closest_match_to(self, waveform, start, end):
        if waveform == self[start]:
            while start - 1 >= 0 and self[start - 1] == waveform:
                start -= 1
        else:
            while start < end and self[start] != waveform:
                start += 1
            if start == end:  # no match found
                return None
        return start

    def find_closest_match_to(self, other, start, end):
        waveform = other[start]
        m_start = self.find_start_of_closest_match_to(waveform, start, end)
        if m_start is None:
            return None
        m_end = m_start + 1
        while m_end + 1 < len(self) and self[m_end + 1] == waveform:
            m_end += 1
        if any(self[i] == waveform for i in range(m_end + 1, end + 1)) or \
           any(other[i] == waveform for i in range(m_start, start)) or \
           any(other[i] == waveform for i in range(end + 1, m_end + 1)):
            return None
        return m_start, m_end

    @staticmethod
    def make(profile):
        to_npts = lambda n: round(n * _EPGFingerprint.SAMPLING_RATE)
        fingerprint, prev_start = [], 0
        for seg in profile:
            start, end = to_npts(seg.start), to_npts(seg.end)
            if prev_start < start:  # fill empty spaces with None
                content = [None] * (start - prev_start)
                fingerprint.extend(content)
            fingerprint.extend([seg.waveform] * (end - start + 1))
            prev_start = end + 1
        return fingerprint


class _EPGFingerprintIterator(Iterator):
    def __init__(self, fingerprint):
        self.fingerprint = fingerprint
        self.start, self.end = -1, -1

    def __next__(self):
        self.end = self.start = self.start + 1
        if self.at_end():
            raise StopIteration
        return self.fingerprint[self.start]

    def advance(self):
        self.start = self.end + 1
        size = len(self.fingerprint)
        while self.start < size and self.fingerprint[self.start] is None:
            self.start += 1
            continue
        val, self.end = self.fingerprint[self.start], self.start + 1
        while self.end + 1 < size and self.fingerprint[self.end + 1] == val:
            self.end += 1
        return val, self.start, self.end

    def at_end(self):
        return self.end >= len(self.fingerprint) - 1
