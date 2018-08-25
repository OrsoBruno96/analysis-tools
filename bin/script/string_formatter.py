# -*- coding:utf-8 -*-
from uncertainties.core import UFloat


def tex_format(value):
    if isinstance(value, UFloat):
        return "$" + "{:.1uL}".format(value) + "$"
    else:
        return "$" + str(value) + "$"


def scrivi(datiDaScrivere, fileout):
    out = open(fileout, "w")
    for i in range(0, len(datiDaScrivere[0])):
        riga = [texFormat(col[i]) for col in datiDaScrivere]
        stringa = " & ".join(riga)
        stringa += " \\\\\n\\hline\n"
        out.write(stringa)
