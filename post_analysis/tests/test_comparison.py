import os
import unittest
from statistics import mean
from autoepg import EPGComparator, EPGProfile
from autoepg.comparison import _EPGFingerprint

WORKDIR = os.path.dirname(os.path.realpath(__file__))


def example_file(fname):
    return os.path.join(WORKDIR, 'examples', fname)


class EPGFingerprintTests(unittest.TestCase):
    def test_making_fingerprint(self):
        profile = EPGProfile.from_file(example_file('simple-manual.csv'))
        expected = [None] * 215
        expected.extend(['pd'] * 336)
        expected.extend(['c'] * 1396)
        expected.extend(['np'] * 1054)
        expected.extend(['c'] * 1013)
        expected.extend(['np'] * 625)
        self.assertSequenceEqual(expected, _EPGFingerprint(profile))

    def test_finding_closest_match(self):
        auto = EPGProfile.from_file(example_file('simple-auto.csv'))
        manual = EPGProfile.from_file(example_file('simple-manual.csv'))
        auto, manual = _EPGFingerprint(auto), _EPGFingerprint(manual)
        finder = auto.find_closest_match_to
        self.assertEqual((1941, 3000), finder(manual, 1947, 3000))
        self.assertEqual((235, 560), finder(manual, 215, 550))
        self.assertEqual((4016, 4638), finder(manual, 4014, 4638))

    def test_finding_closest_match_complex(self):
        auto = EPGProfile.from_file(example_file('Ap1-haba-auto.csv'))
        manual = EPGProfile.from_file(example_file('Ap1-haba-manual.csv'))
        auto, manual = _EPGFingerprint(auto), _EPGFingerprint(manual)
        finder = auto.find_closest_match_to
        self.assertEqual((0, 19313), finder(manual, 0, 19313))
        self.assertEqual(None, finder(manual, 19663, 66200))
        self.assertEqual((66201, 98635), finder(manual, 66201, 98635))
        self.assertEqual(None, finder(manual, 101423, 101845))
        self.assertEqual(None, finder(manual, 203058, 209251))
        self.assertEqual(None, finder(manual, 209667, 214063))
        self.assertEqual(None, finder(manual, 214507, 234165))
        self.assertEqual((249608, 251075), finder(manual, 249608, 251441))
        self.assertEqual(None, finder(manual, 251442, 359998))
        self.assertEqual(None, finder(manual, 371844, 382845))
        self.assertEqual(None, finder(manual, 382985, 389107))
        self.assertEqual((389108, 389470), finder(manual, 389108, 389470))
        self.assertEqual((424665, 425297), finder(manual, 424316, 425567))
        self.assertEqual((425298, 432213), finder(manual, 425568, 432213))


class EPGComparatorTests(unittest.TestCase):
    def test_creating_confusion_matrix(self):
        profile = EPGProfile.from_file(example_file('simple-manual.csv'))
        other = EPGProfile.from_file(example_file('simple-auto.csv'))
        stats = EPGComparator((profile, other)).compare()
        self.assertSequenceEqual(
            (1953, 2, 0, 6, 1677, 0, 450, 0, 316),
            tuple(stats.cfm))
        self.assertEqual(2409, sum(stats.cfm.column('c')))
        self.assertEqual(1679, sum(stats.cfm.column('np')))
        self.assertEqual(316, sum(stats.cfm.column('pd')))

    def test_calculate_offset(self):
        auto = EPGProfile.from_file(example_file('simple-auto.csv'))
        manual = EPGProfile.from_file(example_file('simple-manual.csv'))
        auto, manual = _EPGFingerprint(auto), _EPGFingerprint(manual)
        expected = dict(
            pd=[[20], [10]],
            c=[[10, 0], [-446, 2]],
            np=[[-6, 2], [0, 0]])
        offsets = EPGComparator._calculate_offsets((auto, manual),
                                                   fractional=False)
        self.assertDictEqual(expected, offsets)

    def test_calculate_matches(self):
        auto = EPGProfile.from_file(example_file('Ap1-haba-auto.csv'))
        manual = EPGProfile.from_file(example_file('Ap1-haba-manual.csv'))
        stats = EPGComparator((auto, manual)).compare()

        expected = dict(
            c=dict(n=33, full_matches=21, loffset=.03, roffset=.06),
            pd=dict(n=16, full_matches=8, loffset=0., roffset=0.),
            np=dict(n=26, full_matches=20, loffset=.13, roffset=.04))
        for waveform in expected:
            for name, val in expected[waveform].items():
                self.assertAlmostEqual(val, stats.get(waveform, mean)[name],
                                       2, name)

    def test_calculate_matches_complex(self):
        auto = EPGProfile.from_file(example_file('Ap10-haba-auto.csv'))
        manual = EPGProfile.from_file(example_file('Ap10-haba-manual.csv'))
        stats = EPGComparator((auto, manual)).compare()

        expected = dict(
            c=dict(n=71, full_matches=45, loffset=.01, roffset=.02),
            pd=dict(n=49, full_matches=37, loffset=0., roffset=.04),
            np=dict(n=22, full_matches=20, loffset=.02, roffset=0.))
        for waveform in expected:
            for name, val in expected[waveform].items():
                self.assertAlmostEqual(val, stats.get(waveform, mean)[name],
                                       2, name)

if __name__ == '__main__':
    unittest.main()
