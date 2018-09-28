import matplotlib.pyplot as plt

wall = 1
user = 2
sys  = 3

def time_to_sec(l):
    res = []
    for e in l:
        tmp = e.strip('s').split('m')
        sec = int(tmp[0]) * 60 + float(tmp[1])
        res.append(sec)
    
    return res

def plot(title, x, y1, y2, l1, l2):
    fig = plt.figure()
    ax = plt.axes()
    ax.plot(x, y1, label=l1)
    ax.plot(x, y2, label=l2)
    ax.legend(frameon=False)
    plt.xlabel('# Fibers')
    plt.ylabel('time (s)')
    plt.title(title)
    plt.show()



f = open("Module.times")
mod = [ e.strip() for e in f.readlines() if e != '\n' ]
f.close()

f = open("Sigaltstack.times")
sig = [ e.strip() for e in f.readlines() if e != '\n' ]
f.close()

x = [ int(e) for e in mod[::4] ]


sig_s = time_to_sec(sig[sys::4])
mod_s = time_to_sec(mod[sys::4])

sig_w = time_to_sec(sig[wall::4])
mod_w = time_to_sec(mod[wall::4])

sig_u = time_to_sec(sig[user::4])
mod_u = time_to_sec(mod[user::4])

plot('User space time', x, sig_u, mod_u, "Sigaltstack", "Module")
plot('Kernel space time', x, sig_s, mod_s, "Sigaltstack", "Module")
