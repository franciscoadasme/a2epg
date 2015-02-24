import unittest
from autoepg.stats import ConfusionMatrix


class ConfusionMatrixTests(unittest.TestCase):
    def test_construction(self):
        categories = ('a', 'b', 'c')
        cfm = ConfusionMatrix(categories)
        self.assertSequenceEqual(categories, cfm.categories)
        self.assertEqual(3, cfm.size)
        self.assertFalse(cfm.binary)
        self.assertSequenceEqual([0] * 9, tuple(cfm))
        self.assertRaisesRegex(ValueError, 'binary', cfm.get_binary_cell, 0, 0)

    def test_construction_with_rows(self):
        cfm = ConfusionMatrix(('a', 'b'), ((26, 3), (2, 72)))
        self.assertSequenceEqual((26, 3, 2, 72), tuple(cfm))
        self.assertEqual(26, cfm[0, 0])
        self.assertEqual(2, cfm[1, 0])

    def test_manipulation(self):
        cfm = ConfusionMatrix(('a', 'b', 'c'))
        cfm['a', 'a'] += 10
        cfm[0, 0] += 1
        cfm['b', 'a'] = 2
        cfm['b', 'b'] = 3
        cfm[2, 1] = 6
        cfm['c', 'c'] = 24
        self.assertSequenceEqual(
            (11, 0, 0, 2, 3, 0, 0, 6, 24),
            tuple(cfm))
        self.assertSequenceEqual((11, 3, 24), cfm.diagonal)
        self.assertEqual(46, cfm.total)
        self.assertSequenceEqual((2, 3, 0), cfm[1])
        self.assertSequenceEqual((2, 3, 0), cfm.row(1))
        self.assertSequenceEqual((0, 3, 6), cfm.column(1))

    def test_binarization(self):
        cfm = ConfusionMatrix(('a', 'b', 'c'))
        cfm[0, 0] = 14
        cfm[0, 1] = 3
        cfm[1, 0] = 2
        cfm[1, 1] = 36
        cfm[1, 2] = 3
        cfm[2, 0] = 1
        cfm[2, 2] = 13
        self.assertFalse(cfm.binary)

        data = (('a', (14, 3, 3, 52)),
                ('b', (36, 5, 3, 28)),
                ('c', (13, 1, 3, 55)))
        for category, expected in data:
            bcfm = cfm.binarize(category)
            self.assertTrue(bcfm.binary)
            self.assertEqual(expected, tuple(bcfm))

    def test_classification_stats(self):
        cfm = ConfusionMatrix(('a', 'not a'), ((20, 180), (10, 1820)))
        self.assertEqual(20, cfm.tp)
        self.assertEqual(180, cfm.fp)
        self.assertEqual(10, cfm.fn)
        self.assertEqual(1820, cfm.tn)
        self.assertAlmostEqual(.67, cfm.sensitivity, 2)
        self.assertAlmostEqual(.67, cfm.tpr, 2)
        self.assertAlmostEqual(.91, cfm.specificity, 2)
        self.assertAlmostEqual(.1, cfm.precision, 2)
        self.assertAlmostEqual(.1, cfm.ppv, 2)
        self.assertAlmostEqual(.91, cfm.accuracy, 2)
        self.assertAlmostEqual(.91, cfm.acc, 2)

        cfm = ConfusionMatrix(('a', 'b'), ((20, 5), (10, 15)))
        self.assertAlmostEqual(.4, cfm.kappa(corrected=False), 2)
        cfm = ConfusionMatrix(('a', 'b'), ((45, 15), (25, 15)))
        self.assertAlmostEqual(.1304, cfm.kappa(corrected=False), 4)
        cfm = ConfusionMatrix(('a', 'b'), ((25, 35), (5, 35)))
        self.assertAlmostEqual(.2593, cfm.kappa(corrected=False), 4)

        cfm = ConfusionMatrix(('a', 'b', 'c'),
                              ((88, 10, 2), (14, 40, 6), (18, 10, 12)))
        self.assertAlmostEqual(0.492, cfm.kappa(corrected=False), 3)
        self.assertAlmostEqual(0.592, cfm.kappa(corrected=True), 3)

if __name__ == '__main__':
    unittest.main()
