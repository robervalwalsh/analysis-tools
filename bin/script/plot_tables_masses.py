import pylab
import numpy

lep = ["chr", "lep"]
trig = ["false", "true"]

x_axis_names = ["Nothing", "Smearing", "B-Tag sf", "Reg", "FSR", "Trigger"]


counter = 0
for l in lep:
    counter += 1
    filename350 = "350/" + "_".join([l, "350"]) + ".txt"
    filename1200 = "1200/" + "_".join([l, "1200"]) + ".txt"
    m1, m2, m3 = pylab.loadtxt(filename350, unpack=True)
    n1, n2, n3 = pylab.loadtxt(filename1200, unpack=True)
    totale350 = m1 + m2 + m3
    totale1200 = n1 + n2 + n3
    x = range(0, len(m1))
    pylab.figure(counter)
    # e1 = numpy.sqrt(m1*(1 - m1/totale))/totale
    # e2 = numpy.sqrt(m2*(1 - m2/totale))/totale
    e3 = numpy.sqrt(m3*(1 - m3/totale350))/totale350
    ee3 = numpy.sqrt(n3*(1 - n3/totale1200))/totale1200
    # p1 = pylab.errorbar(x, m1/totale*100, e1*100, linestyle='',
    #                color='red', marker='o', label="0 match")
    # p2 = pylab.errorbar(x, m2/totale*100, e2*100, linestyle='',
    #                color='blue', marker='o', label="1 match")
    print(m3/totale350*100)
    print(n3/totale1200*100)
    p1 = pylab.errorbar(x, m3/totale350*100, e3*100, linestyle='',
                   color='green', marker='o', label="350")
    p2 = pylab.errorbar(x, n3/totale1200*100, ee3*100, linestyle='',
                   color='red', marker='o', label="1200")
    
    pylab.legend(handles=[p1, p2])
    for nome, centro in zip(x_axis_names, range(0, len(m1))):
        pylab.subplot().annotate(nome, xy=(centro, 0), xycoords=('data', 'axes fraction'),
                    xytext=(0, -32), textcoords='offset points',
                    va='top', ha='center')
    pylab.rc('font', size=16)
    pylab.grid(color='gray')
    pylab.minorticks_on()
    # pylab.xlabel("Correction level", fontsize=14)
    pylab.ylabel("% of events", fontsize=14)
    pylab.xlim(-0.3, len(m1) + 0.3 - 1)
    # pylab.ylim(0, 120)
    titolo = ""
    if l == "lep":
        titolo = "Leptonic"
    else:
        titolo = "Non leptonic"
    pylab.title(titolo)
        
pylab.show()
