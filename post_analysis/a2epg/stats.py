import csv
import io
from collections.abc import Iterable
from itertools import chain, product
from math import ceil, log
from statistics import mean


class ConfusionMatrix(Iterable):
    def __init__(self, categories, rows=None):
        self.categories = tuple(categories)
        self.size = len(self.categories)
        self.binary = self.size == 2
        self._rows = [[0 for __ in range(self.size)] for __ in range(self.size)]
        if rows:
            for i in range(self.size):
                for j in range(self.size):
                    self._rows[i][j] = rows[i][j]

    def __add__(self, other):
        if self.categories != other.categories:
            raise ValueError('confusion matrices do not have same categories')
        cfm = ConfusionMatrix(self.categories)
        for i, j in product(range(self.size), range(self.size)):
            cfm[i, j] = self[i, j] + other[i, j]
        return cfm

    def __getitem__(self, item):
        if isinstance(item, tuple):
            if len(item) != 2:
                raise IndexError('list indices must be either integers or a '
                                 '2-sized integer tuple, got {}' + str(item))
            i, j = tuple(self._convert_index(idx) for idx in item)
            return self._rows[i][j]
        return tuple(self._rows[self._convert_index(item)])

    def __iter__(self):
        return iter(chain(*self._rows))

    def __setitem__(self, item, val):
        if not isinstance(item, tuple) or len(item) != 2:
            raise IndexError('list indices must be a 2-sized integer tuple, '
                             'got {}' + str(item))
        i, j = tuple(self._convert_index(idx) for idx in item)
        self._rows[i][j] = val

    def __str__(self):
        digits = ceil(log(max(self), 10))
        template = ('{:<%s}' % (digits + 2)) * (self.size + 1) + '\n'
        content = template.format('', *self.categories)
        for category, row in zip(self.categories, self._rows):
            content += template.format(category, *row)
        return content[:-1]

    @property
    def diagonal(self):
        return tuple(self._rows[i][i] for i in range(self.size))
    total = property(lambda self: sum(chain(*self._rows)))

    true_positive = tp = property(lambda self: self.get_binary_cell(0, 0))
    false_positive = fp = property(lambda self: self.get_binary_cell(0, 1))
    false_negative = fn = property(lambda self: self.get_binary_cell(1, 0))
    true_negative = tn = property(lambda self: self.get_binary_cell(1, 1))

    accuracy = acc = property(lambda self: (self.tp + self.tn) / self.total)
    @property
    def precision(self):
        try:
            return self.tp / (self.tp + self.fp)
        except ZeroDivisionError:
            return -1
    ppv = precision
    @property
    def sensitivity(self):
        try:
            return self.tp / (self.tp + self.fn)
        except ZeroDivisionError:
            return -1
    tpr = sensitivity
    specificity = property(lambda self: self.tn / (self.fp + self.tn))

    def binarize(self, category):
        if self.size < 2:
            raise ValueError('cannot binarize matrix')

        cfm = ConfusionMatrix((category, 'not_{}'.format(category)))
        idx = self.categories.index(category)
        cfm[0, 0] = self[idx, idx]
        cfm[0, 1] = sum(self.row(idx)) - cfm[0][0]
        cfm[1, 0] = sum(self.column(idx)) - cfm[0][0]
        cfm[1, 1] = self.total - sum(cfm)
        return cfm

    def column(self, j):
        j = self._convert_index(j)
        return tuple(row[j] for row in self._rows)

    def compact(self):
        categories = [cat for cat in self.categories
                      if sum(self.column(cat)) > 0 or sum(self.row(cat)) > 0]
        cfm = ConfusionMatrix(categories)
        for i, j in product(categories, categories):
            cfm[i, j] = self[i, j]
        return cfm

    def copy(self):
        return ConfusionMatrix(self.categories, self._rows)

    def get_binary_cell(self, i, j):
        if not self.binary:
            raise ValueError('matrix is not binary')
        return self[i, j]

    def kappa(self, corrected=False):
        p_a = sum(self.diagonal) / self.total  # observed agreement
        p_e = sum(sum(self.column(i)) * sum(self.row(i))  # expected agreement
                  for i in range(self.size)) / self.total ** 2
        try:
            return (p_a - p_e) / ((self._p_max if corrected else 1) - p_e)
        except ZeroDivisionError:
            return -1

    def row(self, i):
        return self[i]

    @property
    def _p_max(self):
        return sum(min(sum(self.row(i)), sum(self.column(i))) / self.total
                   for i in range(self.size))

    def _convert_index(self, idx):
        try:
            return self.categories.index(idx)
        except ValueError:
            return idx


class EPGComparisonStats:
    HEADERS = ('category', 'n', 'length', 'matches', 'full_matches', 'loffset',
               'roffset', 'sensitivity', 'specificity', 'precision', 'accuracy',
               'kappa', 'ckappa')

    def __init__(self, group, cfm, offsets):
        self.group = group
        self.cfm, self.offsets = cfm, offsets
        self._stats = dict()

    def __contains__(self, item):
        return item in self._stats

    def __getitem__(self, item):
        return self.get(item)

    def __str__(self):
        with io.StringIO() as buffer:
            writer = csv.DictWriter(buffer, self.HEADERS, delimiter='\t')
            writer.writeheader()
            for category in self.cfm.categories:
                writer.writerow(self[category])
            content = buffer.getvalue()
        return content

    def get(self, category, accum=mean):
        if category not in self.cfm.categories:
            raise ValueError('unknown category, got {}'.format(category))
        bcfm = self.cfm.binarize(category)
        offsets = EPGComparisonStats._calculate_avg_offsets(
            self.offsets.get(category, [[], []]), accum)
        return dict(
            category=category,
            n=self.group.manual.count(category),
            length=sum(bcfm.column(0)),
            matches=bcfm.tp,
            full_matches=offsets[0]['count'],
            loffset=offsets[0]['accum' if accum else 'values'],
            roffset=offsets[1]['accum' if accum else 'values'],
            sensitivity=bcfm.sensitivity,
            specificity=bcfm.specificity,
            precision=bcfm.precision,
            accuracy=bcfm.accuracy,
            kappa=bcfm.kappa(corrected=False),
            ckappa=bcfm.kappa(corrected=True))

    @staticmethod
    def _calculate_avg_offsets(offsets, accum=None):
        if not offsets[0]:
            return [dict(count=0, accum=-1, values=[]),
                    dict(count=0, accum=-1, values=[])]
        avg_offsets = []
        for values in offsets:
            values = tuple(map(abs, values))
            data = dict(count=len(values), values=values)
            if callable(accum):
                data['accum'] = accum(values)
            avg_offsets.append(data)
        return avg_offsets
